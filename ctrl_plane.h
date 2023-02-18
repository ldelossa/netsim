
#include "node.h"
#include "switch.h"
#include "interface.h"
#include "klist/klist.h"

// ctrl_plane_t organizes active nodes and switches during netsim's runtime. 
typedef struct ctrl_plane_ {
    // the active nodes known to the control plane.
    klist_node nodes;
    // the active switches known to the control plane.
    klist_node switches;
} ctrl_plane_t;


// returns the address of a newly created ctrl_plane_t
ctrl_plane_t *ctrl_plane_create();

// Will register the node with the control plane.
int ctrl_plane_add_node(ctrl_plane_t *cp, node_t *node);

// Will register the switch with the control plane.
int ctrl_plane_add_switch(ctrl_plane_t *cp, switch_t *sw);

// Will unregister the node with the control plane.
int ctrl_plane_remove_node(ctrl_plane_t *cp, node_t *node);

// Will unregister the switch with the control plane.
int ctrl_plane_remove_switch(ctrl_plane_t *cp, switch_t *sw);