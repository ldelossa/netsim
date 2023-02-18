#include "interface.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

int interface_init(interface_t *intf, char *name) {
  if (strlen(name) > INTERFACE_MAX_NAME)
    return -1;

  strcpy(intf->name, name);

  intf->owner.node = NULL;
  intf->foreign_intf = NULL;
  intf->wire = -1;
  intf->ipv4.s_addr = 0;
  intf->ipv4_mask.s_addr = 0;
  bzero(&intf->ipv6, sizeof(intf->ipv6));
  bzero(&intf->ipv6_mask, sizeof(intf->ipv6));
  return 0;
}

interface_t *interface_create(char *name) {
  if (strlen(name) > INTERFACE_MAX_NAME) {
    return NULL;
  }
  interface_t *intf = calloc(1, sizeof(interface_t));
  interface_init(intf, name);
  return intf;
};
