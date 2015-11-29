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
 * Tests for Public API
 * This test, unlikely the others, is linked against the real, distributed,
 * shared library. Its sole purpose is to test for symbol availability.
 */

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crbtree.h"

static void test_api(void) {
        CRBTree t = {};
        CRBNode n = C_RBNODE_INIT(n);

        assert(!c_rbnode_is_linked(&n));

        /* init, is_linked, add, remove, remove_init */

        c_rbtree_add(&t, NULL, &t.root, &n);
        assert(c_rbnode_is_linked(&n));

        c_rbtree_remove_init(&t, &n);
        assert(!c_rbnode_is_linked(&n));

        c_rbtree_add(&t, NULL, &t.root, &n);
        assert(c_rbnode_is_linked(&n));

        c_rbtree_remove(&t, &n);
        assert(c_rbnode_is_linked(&n)); /* @n wasn't touched */

        c_rbnode_init(&n);
        assert(!c_rbnode_is_linked(&n));

        /* first, last, leftmost, rightmost, next, prev */

        assert(!c_rbtree_first(&t));
        assert(!c_rbtree_last(&t));
        assert(&n == c_rbnode_leftmost(&n));
        assert(&n == c_rbnode_rightmost(&n));
        assert(!c_rbnode_next(&n));
        assert(!c_rbnode_prev(&n));
}

int main(int argc, char **argv) {
        test_api();
        return 0;
}
