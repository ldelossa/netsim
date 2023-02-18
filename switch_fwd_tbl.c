#include "klist/klist.h"
#include <net/ethernet.h>
#include <stdlib.h>
#include <string.h>

// wants two uint8_t[6] arrays representing MAC addresses.
// cast pointer to array to 64 bits and right shift off extra bits.
//
// this is little endian specific, for fun, but make this a memcmp in the future.
#define MAC_IS_EQUAL(mac_a, mac_b)                                             \
  (*(uint64_t *)mac_a) << 16 == (*(uint64_t *)mac_b) << 16

typedef struct switch_fwd_tbl_entry {
  klist_node entries;
  struct ether_addr mac;
  uint8_t port;
} switch_fwd_tbl_entry_t;

typedef struct switch_fwd_tbl_ {
  klist_node entries;
} switch_fwd_tbl_t;

switch_fwd_tbl_t *switch_fwd_tbl_create() {
  switch_fwd_tbl_t *tbl = calloc(1, sizeof(switch_fwd_tbl_t));
  return tbl;
}

int8_t switch_fwd_tbl_lookup(switch_fwd_tbl_t *tlb, struct ether_header *hdr,
                             uint8_t port) {
  switch_fwd_tbl_entry_t *found = NULL;
  switch_fwd_tbl_entry_t *fwd = NULL;
  int8_t fwd_port = -1;

  KLIST_ITERATE(tlb->entries.next, switch_fwd_tbl_entry_t, entries) {
    // have we learned this source mac before?
    if (MAC_IS_EQUAL(data->mac.ether_addr_octet, hdr->ether_shost))
      found = data;

    // do we have a forwarding entry for destination mac?
    if (MAC_IS_EQUAL(data->mac.ether_addr_octet, hdr->ether_dhost)) {
      fwd_port = data->port;
    }
  }

  if (!found) {
    found = calloc(1, sizeof(switch_fwd_tbl_entry_t));
    memcpy(&found->mac, hdr->ether_shost, sizeof(hdr->ether_shost));
    found->port = port;
    klist_append(&tlb->entries, &found->entries);
  } else {
    // update port if the source is coming from a new port.
    if (found->port != port)
      found->port = port;
  }

  return fwd_port;
}