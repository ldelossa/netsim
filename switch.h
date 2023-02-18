#pragma once

#include "interface.h"
#include "switch_fwd_tbl.h"
#include <linux/if_bridge.h>

// Max number of ports on switch.
#define SWITCH_MAX_PORTS 10

#define SWITCH_MAX_NAME 256

// Errors returned from switch API.
typedef enum switch_err {
  SWITCH_ERR_CONNECT_FOREIGN_INTF,
  SWITCH_ERR_NO_PORT,
  SWITCH_ERR_USED_PORT,
  SWITCH_ERR_SOCKET_CREATE,
  SWITCH_ERR_STARTED,
  SWITCH_ERR_START_FAIL = -5
} switch_err;

// Specifies the TLV type code for signaling a administrative operation to
// the switch.
//
// Switch admin socket TLVs have the following format.
// +---------+---------+---------+
// | Type(2) |Length(2)| Value(4)|
// +---------+---------+---------+
// |   01    |    01   |  OPCODE |
// +---------+---------+---------+
#define SWITCH_OPCODE_TYPE 1

// Specifies the 
typedef enum switch_opcode : uint32_t {
  // Informs the switch that an OPCODE was invalid.
  SWITCH_OPCODE_INVALID,
  // Informs the switch that a port has a new connection.
  SWITCH_OPCODE_CONNECT
} switch_opcode;

// switch_t represents a layer 2 switch which bridges together interface_t(s)
// such that they can communicate with each other.
//
// the switch_t is a simulation of a real layer 2 switch and its API implements
// which will learn layer 2 address on its ports.
//
// each "port" is an interface_t structure.
typedef struct switch_ {
  pthread_mutex_t mu;
  char name[256];
  switch_fwd_tbl_t *tbl; 
  interface_t ports[SWITCH_MAX_PORTS];
  // a connected unix_socket pair, admin_sock is for clients to write to
  // admin_sock_poll if for the switch to read from.
  int admin_sock;
  int admin_sock_poll;
  // if the switch is running, this is the thread_id for the forwarding thread.
  pthread_t io_thread;
  uint8_t flags;
} switch_t;

// creates a new switch_t.
switch_t *switch_create(const char *name);

// start the provided switch.
//
// this will start the switch's i/o loop, such that it will begin learning
// and forwarding  layer 2 frames written to any of it's interface's `wire`
// fields.
int switch_start(switch_t *sw);

// stops the provided switch.
//
// this will destroy the switch's i/o loop, after this succeeeds any foreign
// interfaces attached to a port will read EOF (0 bytes), and should detach
// themselves from the switch.
int switch_stop(switch_t *sw);

// "plug" the provided interface into the switch's port.
//
// this will generate a unix domain socket pair to simulate a "wire" and both
// the switch's interface, at port number `port`, and the provided interface
// will have their 'wire' fields set with an fd that facilities communication
// between the interfaces.
//
// the caller is expected to limit concurrent access to `intf` and should ensure
// no other thread can modify it during the execution of this function.
//
// returns one of switch_err on error, or the port number `intf` is connected to
// on success.
int switch_connect_interface(switch_t *sw, int8_t port, interface_t *intf);