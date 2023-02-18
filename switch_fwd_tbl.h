#include <net/ethernet.h>

// switch_fwd_tbl is responsible for learning which ports source MACs ingress
// and forwarding subsequent destinations directly to these port.
//
// switch_fwd_tbl can stay opaque to client code, and the API's function set
// can be used.
//
// this allows the implementation of the tbl to shift while client code
// remains the same.
typedef struct switch_fwd_tbl_ switch_fwd_tbl_t;

// Creates a new forwarding table on the heap and returns a pointer to it.
switch_fwd_tbl_t *switch_fwd_tbl_create();

// Request a lookup for the hdr into the fwd table, and return the port
// to forward to.
//
// -1 is returned if the frame should be flooded to all ports.
//
// *hdr must be a valid layer 2 frame, if it is not, behavior is undefined.
int8_t switch_fwd_tbl_lookup(switch_fwd_tbl_t *tlb, struct ether_header *hdr, uint8_t port);