#include "interface.h"
#include "switch.h"
#include "switch_fwd_tbl.h"
#include <error.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <unistd.h>

#define RUNTEST(func, name)                                                    \
  res = func();                                                                \
  if (res != 0) {                                                              \
    printf("test: %s return: %d", name, res);                                  \
    return res;                                                                \
  }

#define ASSERT(exp, err_code, fmt)                                             \
  if (!(exp)) {                                                                \
    printf(fmt);                                                               \
    return err_code;                                                           \
  }

int test_switch_fwd_tbl() {
  switch_fwd_tbl_t *tbl = switch_fwd_tbl_create();

  struct ether_header hdr = {
      .ether_dhost = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD},
      .ether_shost = {0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF},
  };
  int8_t in_port = 0;
  int8_t out_port = -10;

  out_port = switch_fwd_tbl_lookup(tbl, &hdr, 1);

  ASSERT((out_port == -1), -1, "expected -1 for flood.");

  // flip the lookup, since we should have just learned that
  // BEEFDEADBEEF exists off port 1;
  struct ether_header hdr2 = {
      .ether_shost = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD},
      .ether_dhost = {0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF},
  };

  out_port = switch_fwd_tbl_lookup(tbl, &hdr2, 2);

  ASSERT((out_port == 1), -2, "expected lookup to return port 1");

  return 0;
}

int test_switch() {
  switch_t *sw = switch_create("test-switch-1\n");

  // test that connecting an interface to a switch port works
  interface_t *test_intf = interface_create("test_intf-1\n");

  switch_connect_interface(sw, -1, test_intf);

  ASSERT((test_intf->wire != -1), -1, "test interface wire not set\n");
  ASSERT((sw->ports[0].wire != -1), -2, "switch port wire not set\n");

  ASSERT((test_intf->foreign_intf != 0), -3,
         "test interface foreign intf not set\n");
  ASSERT((sw->ports[0].foreign_intf != 0), -4,
         "switch port foreign intf not set\n");

  ASSERT((test_intf->foreign_intf == &sw->ports[0]), -5,
         "test interface foriegn intf not switch port\n");
  ASSERT((sw->ports[0].foreign_intf == test_intf), -6,
         "test interface foriegn intf not switch port\n");

  // test the starting of the switch
  switch_start(sw);

  write(test_intf->wire, "hello", sizeof("hello"));

  // ensure printf flushes
  sleep(1);

  // test that connecting another interface to a switch port works, while the
  // switch is running.
  interface_t *test_intf2 = interface_create("test_intf-1\n");

  switch_connect_interface(sw, -1, test_intf2);

  ASSERT((test_intf2->wire != -1), -1, "test interface2 wire not set\n");
  ASSERT((sw->ports[1].wire != -1), -2, "switch port wire not set\n");

  ASSERT((test_intf2->foreign_intf != 0), -3,
         "test interface2 foreign intf not set\n");
  ASSERT((sw->ports[1].foreign_intf != 0), -4,
         "switch port foreign intf not set\n");

  ASSERT((test_intf2->foreign_intf == &sw->ports[1]), -5,
         "test interface2 foriegn intf not switch port\n");
  ASSERT((sw->ports[1].foreign_intf == test_intf2), -6,
         "test interface2 foriegn intf not switch port\n");

  write(test_intf2->wire, "hello2", sizeof("hello2"));

  // give it a bit for switch to get connected event (occurs in
  // switch_connect_interface)
  sleep(1);

  // test the disconnection of the first interface.
  // a disconnect occurs by closing the client side.
  ASSERT((close(test_intf->wire) == 0), -7, "failed to close test_intf.wire\n");

  // give it a bit for switch to get closed socket read.
  sleep(1);

  ASSERT((sw->ports[0].wire == -1), -8,
         "switch port wire set after disconnect\n");
  ASSERT((sw->ports[0].foreign_intf == 0), -9,
         "switch port foreign intf set after disconnect\n");

  return 0;
}

int main(int argc, char *argv[]) {
  int res = 0;
  RUNTEST(test_switch, "test_switch")
  RUNTEST(test_switch_fwd_tbl, "test_switch_fwd_table")
}
