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
 * Tests Lockless Tree Lookups
 * The RB-Tree implementation supports lockless tree lookups on shared
 * data-structures. While it does not guarantee correct results (you might skip
 * entire sub-trees), it does guarantee valid behavior (the traversal is
 * guaranteed to end and produce some valid result).
 * This test uses ptrace to run tree operations step-by-step in a separate
 * process, and after each instruction verify the pseudo-validity of the tree.
 * This means, a tree must only have valid left/right pointers (or NULL), and
 * must not contain any loops in those pointers.
 *
 * This test runs two processes with a shared context and tree. It runs them in
 * this order:
 *
 *         | PARENT             | CHILD     |
 *         +--------------------+-----------+
 *         ~                    ~           ~
 *          test_parent_start
 *                               test_child1
 *          test_parent_middle
 *                               test_child2
 *          test_parent_end
 *         ~                    ~           ~
 *         +--------------------+-----------+
 *
 * Additionally, on each TRAP of CHILD, the parent runs test_parent_step(). The
 * ptrace infrastructure generates a TRAP after each instruction, so this test
 * is very CPU aggressive in the parent.
 */

#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "c-rbtree.h"
#include "c-rbtree-private.h"

typedef struct {
        size_t mapsize;
        char *map;
        CRBTree *tree;
        CRBNode *node_mem;
        CRBNode **nodes;
        CRBNode **cache;
        size_t n_nodes;
} TestContext;

/* avoid ptrace-sigstop by using SIGKILL errors in traced children */
#define child_assert(_expr) ((void)(!!(_expr) ? 1 : (raise(SIGKILL), 0)))

static int compare(CRBTree *t, void *k, CRBNode *n) {
        return (char *)n - (char *)k;
}

static void shuffle(CRBNode **nodes, size_t n_memb) {
        unsigned int i, j;
        CRBNode *t;

        for (i = 0; i < n_memb; ++i) {
                j = rand() % n_memb;
                t = nodes[j];
                nodes[j] = nodes[i];
                nodes[i] = t;
        }
}

static void toggle(CRBNode *n, bool set) {
        /*
         * Bit 0x2 is unused in __parent_and_color so we use it as marker for
         * cycle detection.
         */
        if (set)
                n->__parent_and_color = (void *)((unsigned long)n->__parent_and_color | 0x2UL);
        else
                n->__parent_and_color = (void *)((unsigned long)n->__parent_and_color & ~0x2UL);
}

static void test_child1(TestContext *ctx) {
        CRBNode *p, **slot;
        size_t i;

        for (i = 0; i < ctx->n_nodes; ++i) {
                child_assert(!c_rbnode_is_linked(ctx->nodes[i]));
                slot = c_rbtree_find_slot(ctx->tree, compare, ctx->nodes[i], &p);
                c_rbtree_add(ctx->tree, p, slot, ctx->nodes[i]);
        }
}

static void test_child2(TestContext *ctx) {
        size_t i;

        for (i = 0; i < ctx->n_nodes; ++i) {
                child_assert(c_rbnode_is_linked(ctx->nodes[i]));
                c_rbtree_remove_init(ctx->tree, ctx->nodes[i]);
        }
}

static void test_parent_start(TestContext *ctx) {
        size_t i;

        /*
         * Generate a tree with @n_nodes entries. We store the entries in
         * @ctx->node_mem, generate a randomized access-map in @ctx->nodes
         * (i.e., an array of pointers to entries in @ctx->node_mem, but in
         * random order), and a temporary cache for free use in the parent.
         *
         * All this is stored in a MAP_SHARED memory region so it is equivalent
         * in child and parent.
         */

        ctx->n_nodes = 32;
        ctx->mapsize = sizeof(CRBTree);
        ctx->mapsize += ctx->n_nodes * sizeof(CRBNode);
        ctx->mapsize += ctx->n_nodes * sizeof(CRBNode*);
        ctx->mapsize += ctx->n_nodes * sizeof(CRBNode*);

        ctx->map = mmap(NULL, ctx->mapsize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
        assert(ctx->map != MAP_FAILED);

        ctx->tree = (void *)ctx->map;
        ctx->node_mem = (void *)(ctx->tree + 1);
        ctx->nodes = (void *)(ctx->node_mem + ctx->n_nodes);
        ctx->cache = (void *)(ctx->nodes + ctx->n_nodes);

        for (i = 0; i < ctx->n_nodes; ++i) {
                ctx->nodes[i] = &ctx->node_mem[i];
                c_rbnode_init(ctx->nodes[i]);
        }

        shuffle(ctx->nodes, ctx->n_nodes);
}

static void test_parent_middle(TestContext *ctx) {
        size_t i;

        shuffle(ctx->nodes, ctx->n_nodes);

        for (i = 0; i < ctx->n_nodes; ++i)
                child_assert(c_rbnode_is_linked(ctx->nodes[i]));
}

static void test_parent_end(TestContext *ctx) {
        size_t i;
        int r;

        for (i = 0; i < ctx->n_nodes; ++i)
                assert(!c_rbnode_is_linked(ctx->nodes[i]));

        r = munmap(ctx->map, ctx->mapsize);
        assert(r >= 0);
}

static void test_parent_step(TestContext *ctx) {
        size_t i, i_level;
        CRBNode *n, *p;

        n = ctx->tree->root;
        i_level = 0;

        while (n) {
                /* verify that we haven't visited @n, yet */
                assert(!((unsigned long)n->__parent_and_color & 0x2UL));

                /* verify @n is a valid node */
                for (i = 0; i < ctx->n_nodes; ++i)
                        if (n == ctx->nodes[i])
                                break;
                assert(i < ctx->n_nodes);

                /* pre-order traversal and marker for cycle detection */
                if (n->left) {
                        toggle(n, true);
                        ctx->cache[i_level++] = n;
                        n = n->left;
                } else if (n->right) {
                        toggle(n, true);
                        ctx->cache[i_level++] = n;
                        n = n->right;
                } else {
                        while (i_level > 0) {
                                p = ctx->cache[i_level - 1];
                                if (p->right && n != p->right) {
                                        n = p->right;
                                        break;
                                }
                                --i_level;
                                n = p;
                                toggle(n, false);
                        }
                        if (i_level == 0)
                                break;
                }
        }
}

static int test_parallel_child(TestContext *ctx) {
        int r;

        /* make parent trace us and enter stopped state */
        r = ptrace(PTRACE_TRACEME, 0, 0, 0);
        child_assert(r >= 0);

        /* SIGUSR1 to signal readiness */
        r = raise(SIGUSR1);
        assert(r >= 0);

        /* run first part */
        test_child1(ctx);

        /* SIGURG to cause re-shuffle */
        r = raise(SIGURG);
        assert(r >= 0);

        /* run second part */
        test_child2(ctx);

        /* SIGUSR2 to signal end */
        r = raise(SIGUSR2);
        assert(r >= 0);

        /* return known exit code to parent */
        return 0xef;
}

static void test_parallel(void) {
        TestContext ctx = {};
        int r, pid, status;
        uint64_t n_instr, n_event;

        /* create shared area for tree verification */
        test_parent_start(&ctx);

        /* run child */
        pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
                r = test_parallel_child(&ctx);
                _exit(r);
        }

        /*
         * After setup, the child immediately enters TRACE-operation and raises
         * SIGUSR1. Once continued, the child performs the pre-configured tree
         * operations. When done, it raises SIGUSR2, and then exits.
         *
         * Here in the parent we catch all trace-stops of the child via waitpid
         * until we get no more such stop-events. Based on the stop-event we
         * get, we verify child-state, STEP it, or perform other state tracking.
         * We repeat this as long as we catch trace-stops from the child.
         */
        n_instr = 0;
        n_event = 0;
        for (r = waitpid(pid, &status, 0);
             r == pid && WIFSTOPPED(status);
             r = waitpid(pid, &status, 0)) {

                switch (WSTOPSIG(status)) {
                case SIGUSR1:
                        n_event |= 0x1;

                        /* step child */
                        r = ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
                        assert(r >= 0);
                        break;

                case SIGURG:
                        n_event |= 0x2;
                        test_parent_middle(&ctx);

                        /* step child */
                        r = ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
                        assert(r >= 0);
                        break;

                case SIGUSR2:
                        n_event |= 0x4;
                        test_parent_end(&ctx);

                        /* continue child */
                        r = ptrace(PTRACE_CONT, pid, 0, 0);
                        assert(r >= 0);
                        break;

                case SIGTRAP:
                        ++n_instr;
                        test_parent_step(&ctx);

                        /* step repeatedly as long as we get SIGTRAP */
                        r = ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
                        assert(r >= 0);
                        break;

                default:
                        assert(0);
                        break;
                }
        }

        /* verify expected exit-value of 0xef */
        assert(r == pid);
        assert(!!WIFEXITED(status));
        assert(WEXITSTATUS(status) == 0xef);

        /* verify we hit all child states */
        assert(n_event & 0x1);
        assert(n_event & 0x2);
        assert(n_event & 0x4);
        assert(n_instr > 0);
}

int main(int argc, char **argv) {
        unsigned int i;

        /* we want stable tests, so use fixed seed */
        srand(0xdeadbeef);

        /*
         * The tests are pseudo random; run them multiple times, each run will
         * have different orders and thus different results.
         */
        for (i = 0; i < 4; ++i)
                test_parallel();

        return 0;
}