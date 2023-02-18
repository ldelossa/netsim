#include "switch.h"
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/ethernet.h>

switch_t *switch_create(const char *name) {
  if (strlen(name) > SWITCH_MAX_NAME)
    return NULL;

  switch_t *sw = calloc(1, sizeof(switch_t));

  // no io_thread running yet.
  sw->io_thread = -1;

  strcpy(sw->name, name);

  if (pthread_mutex_init(&sw->mu, NULL) < 0)
    return NULL;

  // init all ports
  for (int i = 0; i < SWITCH_MAX_PORTS; i++) {
    char buf[SWITCH_MAX_NAME];
    snprintf(buf, sizeof(buf), "eth0/%d", i);
    interface_init(&sw->ports[i], buf);
  }

  // init admin socket pair.
  int pair[2];
  if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pair) < 0)
    return NULL;
  sw->admin_sock = pair[0];
  sw->admin_sock_poll = pair[1];

  return sw;
}

switch_opcode unmarshal_opcode(const char buf[8]) {
  struct {
    uint16_t t;
    uint16_t l;
    uint32_t v;
  } *tlv = (void *)buf;

  if (tlv->t != SWITCH_OPCODE_TYPE) {
    return SWITCH_OPCODE_INVALID;
  }
  if (tlv->l != 1) {
    return SWITCH_OPCODE_INVALID;
  }

  switch (tlv->v) {
  case SWITCH_OPCODE_CONNECT:
    return SWITCH_OPCODE_CONNECT;
  default:
    return SWITCH_OPCODE_INVALID;
  }
}

int frame_rx(switch_t *sw, uint8_t rx_port, char buf[ETHER_MAX_LEN], ssize_t size) {
  // sanity checks.
  if (size < ETHER_MIN_LEN) {
    return -1;
  }

  struct ether_header *hdr = (struct ether_header*)buf;

  // learn the MAC that came into this port.

  // search to see if we have a mapping for dest
    // if yes, forward to port.
    // if no, flood to all ports.
};

// the main io loop of a switch designed to be ran as a thread.
void *switch_io_thread(void *args) {
  // initialize poll
  switch_t *sw = (switch_t *)args;

  struct pollfd fds[SWITCH_MAX_PORTS + 1];

  // admin socket is always first fd.
  fds[0].fd = sw->admin_sock_poll;
  fds[0].events = POLLIN | POLLERR | POLLHUP;

  for (;;) {
  init:
    // (re)initialize the pollfd array with active ports.
    pthread_mutex_lock(&sw->mu);
    for (int i = 1; i <= SWITCH_MAX_PORTS; i++) {
      int port = i - 1;
      fds[i].fd = (sw->ports[port].foreign_intf) ? sw->ports[port].wire : -1;
      fds[i].events = POLLIN | POLLERR | POLLHUP;
    }
    pthread_mutex_unlock(&sw->mu);

    if (poll(fds, SWITCH_MAX_PORTS + 1, 0) < 0) {
      perror("switch_io_thread:poll: encountered error polling fds");
      return 0;
    }

    // admin socket events
    if (fds[0].revents & (POLLIN | POLLERR)) {
      // we'll only support a single opcode per message.
      char *buf = (char[8]){0};
      ssize_t n;
      while ((n = read(fds[0].fd, buf, 8)) == -1 && errno == EINTR) {
      };
      if (n == -1 || n == 0) {
        // admin socket hungup or gave error, this is a fatal case.
      }
      if (n < 8) {
        perror("switch_io_thread: admin socket read too small");
      } else {
        switch_opcode code = unmarshal_opcode(buf);
        switch (code) {
        case SWITCH_OPCODE_CONNECT:
          goto init;
        case SWITCH_OPCODE_INVALID:
          break;
        }
      }
    }

    // layer 2 frame forwarding.
    for (int i = 1; i <= SWITCH_MAX_PORTS; i++) {
      if (fds[i].revents & (POLLIN | POLLERR)) {
        uint8_t port = i - 1;

        // attempt to read max packet size
        // test code for now.
        int n = 0;
        char buf[ETHER_MAX_LEN] = {0};
        n = read(fds[i].fd, buf, sizeof(buf) - 1);

        // the other side closed the socket, or we have an error, this indicates
        // to use that the wire was "unplugged"
        if (n == -1 || n == 0) {
          perror("switch_io_thread: client hungup or returned error on read");
          fds[i].fd = -1;
          sw->ports[port].foreign_intf = NULL;
          sw->ports[port].wire = -1;
          continue;
        }
        // handle layer 2 frame.
      }
    }
    // for loop
  }
}

int switch_start(switch_t *sw) {
  pthread_attr_t attr;
  int err = 0;

  pthread_mutex_lock(&sw->mu);

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  // is io_thread -1 this indicates the switch is not "running".
  if (sw->io_thread != -1) {
    err = SWITCH_ERR_STARTED;
    goto cleanup;
  }

  // create thread and set switch's io_thread to the created thread id.
  // on error, set it back to -1, and handle error.
  if (pthread_create(&sw->io_thread, &attr, switch_io_thread, sw) != 0) {
    sw->io_thread = -1;
    perror("failed to start switch thread.");
    err = SWITCH_ERR_START_FAIL;
    goto cleanup;
  }

cleanup:
  pthread_mutex_unlock(&sw->mu);
  pthread_attr_destroy(&attr);
  return err;
};

// send a connection event to the switch.
int switch_admin_send_connect(switch_t *sw) {
  union {
    struct {
      uint16_t t;
      uint16_t l;
      uint32_t v;
    };
    char buf[8];
  } tlv = {.t = SWITCH_OPCODE_TYPE, .l = 1, .v = SWITCH_OPCODE_CONNECT};
  return write(sw->admin_sock, tlv.buf, sizeof(tlv.buf));
}

int switch_stop(switch_t *sw) {

}

int switch_connect_interface(switch_t *sw, int8_t port, interface_t *intf) {
  if (port > SWITCH_MAX_PORTS)
    return SWITCH_ERR_NO_PORT;

  // incoming interface should not have a foreign interface
  if (intf->foreign_intf)
    return SWITCH_ERR_CONNECT_FOREIGN_INTF;

  pthread_mutex_lock(&sw->mu);

  // caller wants next available port
  if (port == -1) {
    for (int i = 0; i < SWITCH_MAX_PORTS; i++) {
      if (!sw->ports[i].foreign_intf) {
        port = i;
        break;
      }
    }
    if (port == -1) {
      pthread_mutex_unlock(&sw->mu);
      return SWITCH_ERR_NO_PORT;
    }
  }

  // requested port is in use
  if (sw->ports[port].foreign_intf) {
    pthread_mutex_unlock(&sw->mu);
    return SWITCH_ERR_USED_PORT;
  }

  // generate a unix domain socket pair.
  int pair[2] = {0};
  if ((socketpair(AF_LOCAL, SOCK_STREAM, 0, pair)) < 0) {
    pthread_mutex_unlock(&sw->mu);
    return SWITCH_ERR_SOCKET_CREATE;
  }

  // set backlinks
  intf->foreign_intf = &sw->ports[port];
  sw->ports[port].foreign_intf = intf;

  // set fds for simulating wire.
  sw->ports[port].wire = pair[0];
  intf->wire = pair[1];

  // if the switch is running, tell it that a new port was plugged in via
  // admin socket.
  if (sw->io_thread != -1) { // hold lock while we check this.
    pthread_mutex_unlock(&sw->mu);
    return switch_admin_send_connect(sw);
  } else {
    pthread_mutex_unlock(&sw->mu);
  }

  return 1;
};
