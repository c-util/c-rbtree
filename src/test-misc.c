/*
 * Tests for Miscellaneous Tree Operations
 * This test contains all of the minor tests that did not fit anywhere else.
 */

#undef NDEBUG
#include <assert.h>
#include <c-stdaux.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-rbtree.h"
#include "c-rbtree-private.h"

static void insert(CRBTree *t, CRBNode *n) {
        CRBNode **i, *p;

        c_assert(t);
        c_assert(n);
        c_assert(!c_rbnode_is_linked(n));

        i = &t->root;
        p = NULL;
        while (*i) {
                p = *i;
                if (n < *i) {
                        i = &(*i)->left;
                } else {
                        c_assert(n > *i);
                        i = &(*i)->right;
                }
        }

        c_rbtree_add(t, p, i, n);
}

static void test_move(void) {
        CRBTree t1 = C_RBTREE_INIT, t2 = C_RBTREE_INIT;
        CRBNode n[128];
        unsigned int i;

        for (i = 0; i < sizeof(n) / sizeof(*n); ++i) {
                n[i] = (CRBNode)C_RBNODE_INIT(n[i]);
                insert(&t1, &n[i]);
        }

        c_assert(!c_rbtree_is_empty(&t1));
        c_assert(c_rbtree_is_empty(&t2));

        c_rbtree_move(&t2, &t1);

        c_assert(c_rbtree_is_empty(&t1));
        c_assert(!c_rbtree_is_empty(&t2));

        while (t2.root)
                c_rbnode_unlink(t2.root);

        c_assert(c_rbtree_is_empty(&t1));
        c_assert(c_rbtree_is_empty(&t2));
}

static void test_parent_encoding(void) {
        /*
         * The parent pointer and the node flags share @__parent_and_flags.
         * Use a parent address with all the upper bits set (never
         * dereferenced), so any mask narrower than a pointer would drop them.
         * This is the case of `unsigned long` on LLP64 targets (Windows),
         * where it is only 32 bits wide.
         */
        uintptr_t parent = ~(uintptr_t)0 & ~(uintptr_t)0xf;
        CRBNode n;

        c_assert(sizeof(n.__parent_and_flags) >= sizeof(void *));
        c_assert((~C_RBNODE_FLAG_MASK) == (UINTPTR_MAX ^ C_RBNODE_FLAG_MASK));

        n = (CRBNode)C_RBNODE_INIT(n);
        n.__parent_and_flags = parent | C_RBNODE_RED;

        c_assert(c_rbnode_parent(&n) == (CRBNode *)parent);
        c_assert(c_rbnode_raw(&n) == (void *)parent);
        c_assert(c_rbnode_is_red(&n));
        c_assert(!c_rbnode_is_root(&n));

        n.__parent_and_flags = parent | C_RBNODE_ROOT;

        c_assert(!c_rbnode_parent(&n));
        c_assert(c_rbnode_raw(&n) == (void *)parent);
        c_assert(c_rbnode_is_black(&n));
        c_assert(c_rbnode_is_root(&n));
}

int main(int argc, char **argv) {
        test_move();
        test_parent_encoding();

        return 0;
}
