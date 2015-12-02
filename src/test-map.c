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
 * Tests Map API
 * This contains tests for the CRBMap API, which wraps CRBTree but provides
 * insertion and lookup helpers. Tests for internal tree consistency are
 * skipped here, as other tests already do that. This just tests for the
 * extensions that CRBMap provides over CRBTree.
 */

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "crbtree.h"
#include "crbtree-private.h"

typedef struct {
        unsigned long key;
        CRBNode rb;
} Node;

#define node_from_rb(_rb) ((Node *)((char *)(_rb) - offsetof(Node, rb)))

static int test_compare(CRBTree *t, void *k, CRBNode *n) {
        unsigned long key = (unsigned long)k;
        Node *node = node_from_rb(n);

        return (key < node->key) ? -1 : (key > node->key) ? 1 : 0;
}

static void shuffle(Node **nodes, size_t n_memb) {
        unsigned int i, j;
        Node *t;

        for (i = 0; i < n_memb; ++i) {
                j = rand() % n_memb;
                t = nodes[j];
                nodes[j] = nodes[i];
                nodes[i] = t;
        }
}

static void test_map(void) {
        CRBNode **slot, *p;
        CRBTree t = {};
        Node *nodes[2048];
        unsigned long i;

        /* allocate and initialize all nodes */
        for (i = 0; i < sizeof(nodes) / sizeof(*nodes); ++i) {
                nodes[i] = malloc(sizeof(*nodes[i]));
                assert(nodes[i]);
                nodes[i]->key = i;
                c_rbnode_init(&nodes[i]->rb);
        }

        /* shuffle nodes */
        shuffle(nodes, sizeof(nodes) / sizeof(*nodes));

        /* add all nodes, and verify that each node is linked */
        for (i = 0; i < sizeof(nodes) / sizeof(*nodes); ++i) {
                assert(!c_rbnode_is_linked(&nodes[i]->rb));
                assert(!c_rbtree_find_entry(&t, test_compare, (void *)nodes[i]->key, Node, rb));

                slot = c_rbtree_find_slot(&t, test_compare, (void *)nodes[i]->key, &p);
                assert(slot);
                c_rbtree_add(&t, p, slot, &nodes[i]->rb);

                assert(c_rbnode_is_linked(&nodes[i]->rb));
                assert(nodes[i] == c_rbtree_find_entry(&t, test_compare, (void *)nodes[i]->key, Node, rb));
        }

        /* shuffle nodes again */
        shuffle(nodes, sizeof(nodes) / sizeof(*nodes));

        /* remove all nodes (in different order) */
        for (i = 0; i < sizeof(nodes) / sizeof(*nodes); ++i) {
                assert(c_rbnode_is_linked(&nodes[i]->rb));
                assert(nodes[i] == c_rbtree_find_entry(&t, test_compare, (void *)nodes[i]->key, Node, rb));

                c_rbtree_remove_init(&t, &nodes[i]->rb);

                assert(!c_rbnode_is_linked(&nodes[i]->rb));
                assert(!c_rbtree_find_entry(&t, test_compare, (void *)nodes[i]->key, Node, rb));
        }

        /* free nodes again */
        for (i = 0; i < sizeof(nodes) / sizeof(*nodes); ++i)
                free(nodes[i]);
}

int main(int argc, char **argv) {
        /* we want stable tests, so use fixed seed */
        srand(0xdeadbeef);

        test_map();
        return 0;
}
