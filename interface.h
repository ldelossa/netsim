#pragma once

#include <netinet/in.h>
#include <net/ethernet.h>

#define INTERFACE_MAX_NAME 256

// forward declare node, keep node opaque to interface's API to avoid
// cyclic imports.
typedef struct node_ node_t;

// forward declare switch, keep node opaque to interface's API to avoid
// cyclic imports.
typedef struct switch_ switch_t;

// interface_t is an abstraction for a network interface.
//
// an interface must be assigned a hw address and can optionally be assigned
// an ipv4 and ipv6 address.
//
// the "wire" is simulated with a unix socket fd, which when read, indicates
// data is available.
typedef struct interface_ {
  char name[INTERFACE_MAX_NAME];
  union {
    // back pointer to the node which owns this interface
    node_t *node;
    // back pointer to switch which owns this interface
    switch_t *sw;
  } owner;
  struct ether_addr mac;
  struct in6_addr ipv6;
  struct in6_addr ipv6_mask;
  struct in_addr ipv4;
  struct in_addr ipv4_mask;
  // if this interface is plugged into another, the foreign_intf will be set.
  struct interface_ *foreign_intf;
  // this will be abstracted by a streaming unix domain socket, which the
  // may read and write to, simulating wire tx/rx.
  int wire;
} interface_t;

// initialize a pointer to an interface with the name and zero values.
int interface_init(interface_t *intf, char *name);

// create an interface with the specified name and return a pointer to it.
interface_t *interface_create(char *name);
