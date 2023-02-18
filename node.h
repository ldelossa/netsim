#pragma once

#include "interface.h"

#define NODE_MAX_INTERFACES 10

typedef struct node_ {
    char name[256];
    interface_t *interfaces[NODE_MAX_INTERFACES];
} node_t;

