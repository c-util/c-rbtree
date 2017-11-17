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
        C_RBNODE_RED                    = 0x1,
        C_RBNODE_UNUSED1                = 0x2,
        C_RBNODE_UNUSED2                = 0x4,
};

static inline unsigned long c_rbnode_flags(CRBNode *n) {
        return (unsigned long)n->__parent_and_flags & C_RBNODE_FLAG_MASK;
}

static inline _Bool c_rbnode_is_red(CRBNode *n) {
        return c_rbnode_flags(n) & C_RBNODE_RED;
}

static inline _Bool c_rbnode_is_black(CRBNode *n) {
        return !(c_rbnode_flags(n) & C_RBNODE_RED);
}
