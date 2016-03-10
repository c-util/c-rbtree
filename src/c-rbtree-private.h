#pragma once

/***
  This file is part of crbtree. See COPYING for details.

  crbtree is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  crbtree is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with crbtree; If not, see <http://www.gnu.org/licenses/>.
***/

/*
 * Private definitions
 * This file contains private definitions for the RB-Tree implementation, but
 * which are used by our test-suite.
 */

#include <stddef.h>
#include "org.bus1/c-rbtree.h"

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
        return (unsigned long)n->__parent_and_color & 1UL;
}

static inline _Bool c_rbnode_is_red(CRBNode *n) {
        return c_rbnode_color(n) == C_RBNODE_RED;
}

static inline _Bool c_rbnode_is_black(CRBNode *n) {
        return c_rbnode_color(n) == C_RBNODE_BLACK;
}
