#pragma once

/*
 * Private definitions
 * This file contains private definitions for the RB-Tree implementation, but
 * which are used by our test-suite.
 */

#include <stddef.h>
#include "c-rbtree.h"

/*
 * Macros
 */

#define _public_ __attribute__((__visibility__("default")))

/*
 * Nodes
 */

enum {
        C_RBNODE_RED   = 0,
        C_RBNODE_BLACK = 1,
};

static inline unsigned long c_rbnode_color(CRBNode *n) {
        return (unsigned long)n->__parent_and_color & C_RBNODE_FLAG_MASK;
}

static inline _Bool c_rbnode_is_red(CRBNode *n) {
        return c_rbnode_color(n) == C_RBNODE_RED;
}

static inline _Bool c_rbnode_is_black(CRBNode *n) {
        return c_rbnode_color(n) == C_RBNODE_BLACK;
}
