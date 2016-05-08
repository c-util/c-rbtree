/***
  This file is part of c-rbtree. See COPYING for details.

  c-rbtree is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  c-rbtree is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with c-rbtree; If not, see <http://www.gnu.org/licenses/>.
***/

/*
 * RB-Tree based Map
 * This implements a basic Map between integer keys and objects. It uses the
 * lookup and insertion helpers, rather than open-coding it.
 */

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "c-rbtree.h"
#include "c-rbtree-private.h"

typedef struct {
        unsigned long key;
        unsigned int marker;
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
        unsigned long i, v;

        /* allocate and initialize all nodes */
        for (i = 0; i < sizeof(nodes) / sizeof(*nodes); ++i) {
                nodes[i] = malloc(sizeof(*nodes[i]));
                assert(nodes[i]);
                nodes[i]->key = i;
                nodes[i]->marker = 0;
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

        /* verify in-order traversal works */
        i = 0;
        v = 0;
        for (p = c_rbtree_first(&t); p; p = c_rbnode_next(p)) {
                ++i;
                assert(!node_from_rb(p)->marker);
                node_from_rb(p)->marker = 1;

                assert(v <= node_from_rb(p)->key);
                v = node_from_rb(p)->key;

                assert(!c_rbnode_next(p) || p == c_rbnode_prev(c_rbnode_next(p)));
        }
        assert(i == sizeof(nodes) / sizeof(*nodes));

        /* verify reverse in-order traversal works */
        i = 0;
        v = -1;
        for (p = c_rbtree_last(&t); p; p = c_rbnode_prev(p)) {
                ++i;
                assert(node_from_rb(p)->marker);
                node_from_rb(p)->marker = 0;

                assert(v >= node_from_rb(p)->key);
                v = node_from_rb(p)->key;
        }
        assert(i == sizeof(nodes) / sizeof(*nodes));

        /* verify post-order traversal works */
        i = 0;
        for (p = c_rbtree_first_postorder(&t); p; p = c_rbnode_next_postorder(p)) {
                ++i;
                assert(!node_from_rb(p)->marker);
                assert(!c_rbnode_parent(p) || !node_from_rb(c_rbnode_parent(p))->marker);
                assert(!p->left || node_from_rb(p->left)->marker);
                assert(!p->right || node_from_rb(p->right)->marker);
                node_from_rb(p)->marker = 1;

                assert(!c_rbnode_next_postorder(p) || p == c_rbnode_prev_postorder(c_rbnode_next_postorder(p)));
        }
        assert(i == sizeof(nodes) / sizeof(*nodes));

        /* verify pre-order (inverse post-order) traversal works */
        i = 0;
        for (p = c_rbtree_last_postorder(&t); p; p = c_rbnode_prev_postorder(p)) {
                ++i;
                assert(node_from_rb(p)->marker);
                assert(!c_rbnode_parent(p) || !node_from_rb(c_rbnode_parent(p))->marker);
                assert(!p->left || node_from_rb(p->left)->marker);
                assert(!p->right || node_from_rb(p->right)->marker);
                node_from_rb(p)->marker = 0;
        }
        assert(i == sizeof(nodes) / sizeof(*nodes));

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
        for (i = 0; i < sizeof(nodes) / sizeof(*nodes); ++i) {
                assert(!nodes[i]->marker);
                free(nodes[i]);
        }
}

int main(int argc, char **argv) {
        /* we want stable tests, so use fixed seed */
        srand(0xdeadbeef);

        test_map();
        return 0;
}