/*
 * RB-Tree Implementation
 * This implements the insertion/removal of elements in RB-Trees. You're highly
 * recommended to have an RB-Tree documentation at hand when reading this. Both
 * insertion and removal can be split into a handful of situations that can
 * occur. Those situations are enumerated as "Case 1" to "Case n" here, and
 * follow closely the cases described in most RB-Tree documentations. This file
 * does not explain why it is enough to handle just those cases, nor does it
 * provide a proof of correctness. Dig out your algorithm 101 handbook if
 * you're interested.
 *
 * This implementation is *not* straightforward. Usually, a handful of
 * rotation, reparent, swap and link helpers can be used to implement the
 * rebalance operations. However, those often perform unnecessary writes.
 * Therefore, this implementation hard-codes all the operations. You're highly
 * recommended to look at the two basic helpers before reading the code:
 *     c_rbtree_swap_child()
 *     c_rbtree_set_parent_and_color()
 * Those are the only helpers used, hence, you should really know what they do
 * before digging into the code.
 *
 * For a highlevel documentation of the API, see the header file and docbook
 * comments.
 */

#include <assert.h>
#include <stddef.h>

#include "c-rbtree-private.h"
#include "c-rbtree.h"

/**
 * c_rbnode_leftmost() - return leftmost child
 * @n:          current node, or NULL
 *
 * This returns the leftmost child of @n. If @n is NULL, this will return NULL.
 * In all other cases, this function returns a valid pointer. That is, if @n
 * does not have any left children, this returns @n.
 *
 * Worst case runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to leftmost child, or NULL.
 */
_public_ CRBNode *c_rbnode_leftmost(CRBNode *n) {
        if (n)
                while (n->left)
                        n = n->left;
        return n;
}

/**
 * c_rbnode_rightmost() - return rightmost child
 * @n:          current node, or NULL
 *
 * This returns the rightmost child of @n. If @n is NULL, this will return
 * NULL. In all other cases, this function returns a valid pointer. That is, if
 * @n does not have any right children, this returns @n.
 *
 * Worst case runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to rightmost child, or NULL.
 */
_public_ CRBNode *c_rbnode_rightmost(CRBNode *n) {
        if (n)
                while (n->right)
                        n = n->right;
        return n;
}

/**
 * c_rbnode_leftdeepest() - return left-deepest child
 * @n:          current node, or NULL
 *
 * This returns the left-deepest child of @n. If @n is NULL, this will return
 * NULL. In all other cases, this function returns a valid pointer. That is, if
 * @n does not have any children, this returns @n.
 *
 * The left-deepest child is defined as the deepest child without any left
 * (grand-...)siblings.
 *
 * Worst case runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to left-deepest child, or NULL.
 */
_public_ CRBNode *c_rbnode_leftdeepest(CRBNode *n) {
        if (n) {
                for (;;) {
                        if (n->left)
                                n = n->left;
                        else if (n->right)
                                n = n->right;
                        else
                                break;
                }
        }
        return n;
}

/**
 * c_rbnode_rightdeepest() - return right-deepest child
 * @n:          current node, or NULL
 *
 * This returns the right-deepest child of @n. If @n is NULL, this will return
 * NULL. In all other cases, this function returns a valid pointer. That is, if
 * @n does not have any children, this returns @n.
 *
 * The right-deepest child is defined as the deepest child without any right
 * (grand-...)siblings.
 *
 * Worst case runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to right-deepest child, or NULL.
 */
_public_ CRBNode *c_rbnode_rightdeepest(CRBNode *n) {
        if (n) {
                for (;;) {
                        if (n->right)
                                n = n->right;
                        else if (n->left)
                                n = n->left;
                        else
                                break;
                }
        }
        return n;
}

/**
 * c_rbnode_next() - return next node
 * @n:          current node, or NULL
 *
 * An RB-Tree always defines a linear order of its elements. This function
 * returns the logically next node to @n. If @n is NULL, the last node or
 * unlinked, this returns NULL.
 *
 * Worst case runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to next node, or NULL.
 */
_public_ CRBNode *c_rbnode_next(CRBNode *n) {
        CRBNode *p;

        if (!c_rbnode_is_linked(n))
                return NULL;
        if (n->right)
                return c_rbnode_leftmost(n->right);

        while ((p = c_rbnode_parent(n)) && n == p->right)
                n = p;

        return p;
}

/**
 * c_rbnode_prev() - return previous node
 * @n:          current node, or NULL
 *
 * An RB-Tree always defines a linear order of its elements. This function
 * returns the logically previous node to @n. If @n is NULL, the first node or
 * unlinked, this returns NULL.
 *
 * Worst case runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to previous node, or NULL.
 */
_public_ CRBNode *c_rbnode_prev(CRBNode *n) {
        CRBNode *p;

        if (!c_rbnode_is_linked(n))
                return NULL;
        if (n->left)
                return c_rbnode_rightmost(n->left);

        while ((p = c_rbnode_parent(n)) && n == p->left)
                n = p;

        return p;
}

/**
 * c_rbnode_next_postorder() - return next node in post-order
 * @n:          current node, or NULL
 *
 * This returns the next node to @n, based on a left-to-right post-order
 * traversal. If @n is NULL, the root node, or unlinked, this returns NULL.
 *
 * This implements a left-to-right post-order traversal: First visit the left
 * child of a node, then the right, and lastly the node itself. Children are
 * traversed recursively.
 *
 * This function can be used to implement a left-to-right post-order traversal:
 *
 *     for (n = c_rbtree_first_postorder(t); n; n = c_rbnode_next_postorder(n))
 *             visit(n);
 *
 * Worst case runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to next node, or NULL.
 */
_public_ CRBNode *c_rbnode_next_postorder(CRBNode *n) {
        CRBNode *p;

        if (!c_rbnode_is_linked(n))
                return NULL;

        p = c_rbnode_parent(n);
        if (p && n == p->left && p->right)
                return c_rbnode_leftdeepest(p->right);

        return p;
}

/**
 * c_rbnode_prev_postorder() - return previous node in post-order
 * @n:          current node, or NULL
 *
 * This returns the previous node to @n, based on a left-to-right post-order
 * traversal. That is, it is the inverse operation to c_rbnode_next_postorder().
 * If @n is NULL, the left-deepest node, or unlinked, this returns NULL.
 *
 * This function returns the logical previous node in a directed post-order
 * traversal. That is, it effectively does a pre-order traversal (since a
 * reverse post-order traversal is a pre-order traversal). This function does
 * NOT do a right-to-left post-order traversal! In other words, the following
 * invariant is guaranteed, if c_rbnode_next_postorder(n) is non-NULL:
 *
 *     n == c_rbnode_prev_postorder(c_rbnode_next_postorder(n))
 *
 * This function can be used to implement a right-to-left pre-order traversal,
 * using the fact that a reverse post-order traversal is also a valid pre-order
 * traversal:
 *
 *     for (n = c_rbtree_last_postorder(t); n; n = c_rbnode_prev_postorder(n))
 *             visit(n);
 *
 * This would effectively perform a right-to-left pre-order traversal: first
 * visit a parent, then its right child, then its left child. Both children are
 * traversed recursively.
 *
 * Worst case runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to previous node in post-order, or NULL.
 */
_public_ CRBNode *c_rbnode_prev_postorder(CRBNode *n) {
        CRBNode *p;

        if (!c_rbnode_is_linked(n))
                return NULL;
        if (n->right)
                return n->right;
        if (n->left)
                return n->left;

        while ((p = c_rbnode_parent(n))) {
                if (p->left && n != p->left)
                        return p->left;
                n = p;
        }

        return NULL;
}

/**
 * c_rbtree_first() - return first node
 * @t:          tree to operate on
 *
 * An RB-Tree always defines a linear order of its elements. This function
 * returns the logically first node in @t. If @t is empty, NULL is returned.
 *
 * Fixed runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to first node, or NULL.
 */
_public_ CRBNode *c_rbtree_first(CRBTree *t) {
        assert(t);
        return c_rbnode_leftmost(t->root);
}

/**
 * c_rbtree_last() - return last node
 * @t:          tree to operate on
 *
 * An RB-Tree always defines a linear order of its elements. This function
 * returns the logically last node in @t. If @t is empty, NULL is returned.
 *
 * Fixed runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to last node, or NULL.
 */
_public_ CRBNode *c_rbtree_last(CRBTree *t) {
        assert(t);
        return c_rbnode_rightmost(t->root);
}

/**
 * c_rbtree_first_postorder() - return first node in post-order
 * @t:          tree to operate on
 *
 * This returns the first node of a left-to-right post-order traversal. That
 * is, it returns the left-deepest leaf. If the tree is empty, this returns
 * NULL.
 *
 * This can also be interpreted as the last node of a right-to-left pre-order
 * traversal.
 *
 * Fixed runtime (n: number of elements in tree): O(log(n))
 *
 * Return: Pointer to first node in post-order, or NULL.
 */
_public_ CRBNode *c_rbtree_first_postorder(CRBTree *t) {
        assert(t);
        return c_rbnode_leftdeepest(t->root);
}

/**
 * c_rbtree_last_postorder() - return last node in post-order
 * @t:          tree to operate on
 *
 * This returns the last node of a left-to-right post-order traversal. That is,
 * it always returns the root node, or NULL if the tree is empty.
 *
 * This can also be interpreted as the first node of a right-to-left pre-order
 * traversal.
 *
 * Fixed runtime (n: number of elements in tree): O(1)
 *
 * Return: Pointer to last node in post-order, or NULL.
 */
_public_ CRBNode *c_rbtree_last_postorder(CRBTree *t) {
        assert(t);
        return t->root;
}

/*
 * Set the color and parent of a node. This should be treated as a simple
 * assignment of the 'color' and 'parent' fields of the node. No other magic is
 * applied. But since both fields share its backing memory, this helper
 * function is provided.
 */
static inline void c_rbnode_set_parent_and_color(CRBNode *n, CRBNode *p, unsigned long c) {
        assert(!((unsigned long)p & 1));
        assert(c < 2);
        n->__parent_and_color = (CRBNode*)((unsigned long)p | c);
}

/* same as c_rbnode_set_parent_and_color(), but keeps the current color */
static inline void c_rbnode_set_parent(CRBNode *n, CRBNode *p) {
        c_rbnode_set_parent_and_color(n, p, c_rbnode_color(n));
}

static inline void c_rbtree_store(volatile void *ptr, void *addr) {
        /*
         * We use volatile accesses whenever we STORE @left or @right members
         * of a node. This guarantees that any parallel, lockless lookup gets
         * to see those stores in the correct order, which itself guarantees
         * that there're no temporary loops during tree rotation.
         * Note that you still need to properly synchronize your accesses via
         * seqlocks, rcu, whatever. We just guarantee that you get *some*
         * result on a lockless traversal and never run into endless loops, or
         * undefined behavior.
         */
        *(volatile CRBNode **)ptr = addr;
}

/*
 * This function partially replaces an existing child pointer to a new one. The
 * existing child must be given as @old, the new child as @new. @p must be the
 * parent of @old (or NULL if it has no parent).
 * This function ensures that the parent of @old now points to @new. However,
 * it does *NOT* change the parent pointer of @new. The caller must ensure
 * this.
 * If @p is NULL, this function ensures that the root-pointer is adjusted
 * instead (given as @t).
 */
static inline void c_rbtree_swap_child(CRBTree *t, CRBNode *p, CRBNode *old, CRBNode *new) {
        if (p) {
                if (p->left == old)
                        c_rbtree_store(&p->left, new);
                else
                        c_rbtree_store(&p->right, new);
        } else {
                t->root = new;
        }
}

static inline CRBNode *c_rbtree_paint_one(CRBTree *t, CRBNode *n) {
        CRBNode *p, *g, *gg, *u, *x;

        /*
         * Paint a single node according to RB-Tree rules. The node must
         * already be linked into the tree and painted red.
         * We repaint the node or rotate the tree, if required. In case a
         * recursive repaint is required, the next node to be re-painted
         * is returned.
         *      p: parent
         *      g: grandparent
         *      gg: grandgrandparent
         *      u: uncle
         *      x: temporary
         */

        /* node is red, so we can access the parent directly */
        p = n->__parent_and_color;

        if (!p) {
                /* Case 1:
                 * We reached the root. Mark it black and be done. As all
                 * leaf-paths share the root, the ratio of black nodes on each
                 * path stays the same. */
                c_rbnode_set_parent_and_color(n, p, C_RBNODE_BLACK);
                n = NULL;
        } else if (c_rbnode_is_black(p)) {
                /* Case 2:
                 * The parent is already black. As our node is red, we did not
                 * change the number of black nodes on any path, nor do we have
                 * multiple consecutive red nodes. */
                n = NULL;
        } else if (p == p->__parent_and_color->left) { /* parent is red, so grandparent exists */
                g = p->__parent_and_color;
                gg = c_rbnode_parent(g);
                u = g->right;

                if (u && c_rbnode_is_red(u)) {
                        /* Case 3:
                         * Parent and uncle are both red. We know the
                         * grandparent must be black then. Repaint parent and
                         * uncle black, the grandparent red and recurse into
                         * the grandparent. */
                        c_rbnode_set_parent_and_color(p, g, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(u, g, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(g, gg, C_RBNODE_RED);
                        n = g;
                } else {
                        /* parent is red, uncle is black */

                        if (n == p->right) {
                                /* Case 4:
                                 * We're the right child. Rotate on parent to
                                 * become left child, so we can handle it the
                                 * same as case 5. */
                                x = n->left;
                                c_rbtree_store(&p->right, n->left);
                                c_rbtree_store(&n->left, p);
                                if (x)
                                        c_rbnode_set_parent_and_color(x, p, C_RBNODE_BLACK);
                                c_rbnode_set_parent_and_color(p, n, C_RBNODE_RED);
                                p = n;
                        }

                        /* 'n' is invalid from here on! */
                        n = NULL;

                        /* Case 5:
                         * We're the red left child or a red parent, black
                         * grandparent and uncle. Rotate on grandparent and
                         * switch color with parent. Number of black nodes on
                         * each path stays the same, but we got rid of the
                         * double red path. As the grandparent is still black,
                         * we're done. */
                        x = p->right;
                        c_rbtree_store(&g->left, x);
                        c_rbtree_store(&p->right, g);
                        if (x)
                                c_rbnode_set_parent_and_color(x, g, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(p, gg, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(g, p, C_RBNODE_RED);
                        c_rbtree_swap_child(t, gg, g, p);
                }
        } else /* if (p == p->__parent_and_color->left) */ { /* same as above, but mirrored */
                g = p->__parent_and_color;
                gg = c_rbnode_parent(g);
                u = g->left;

                if (u && c_rbnode_is_red(u)) {
                        c_rbnode_set_parent_and_color(p, g, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(u, g, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(g, gg, C_RBNODE_RED);
                        n = g;
                } else {
                        if (n == p->left) {
                                x = n->right;
                                c_rbtree_store(&p->left, n->right);
                                c_rbtree_store(&n->right, p);
                                if (x)
                                        c_rbnode_set_parent_and_color(x, p, C_RBNODE_BLACK);
                                c_rbnode_set_parent_and_color(p, n, C_RBNODE_RED);
                                p = n;
                        }

                        n = NULL;

                        x = p->left;
                        c_rbtree_store(&g->right, x);
                        c_rbtree_store(&p->left, g);
                        if (x)
                                c_rbnode_set_parent_and_color(x, g, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(p, gg, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(g, p, C_RBNODE_RED);
                        c_rbtree_swap_child(t, gg, g, p);
                }
        }

        return n;
}

static inline void c_rbtree_paint(CRBTree *t, CRBNode *n) {
        assert(t);
        assert(n);

        while (n)
                n = c_rbtree_paint_one(t, n);
}

/**
 * c_rbtree_add() - add node to tree
 * @t:          tree to operate one
 * @p:          parent node to link under, or NULL
 * @l:          left/right slot of @p (or root) to link at
 * @n:          node to add
 *
 * This links @n into the tree given as @t. The caller must provide the exact
 * spot where to link the node. That is, the caller must traverse the tree
 * based on their search order. Once they hit a leaf where to insert the node,
 * call this function to link it and rebalance the tree.
 *
 * A typical insertion would look like this (@t is your tree, @n is your node):
 *
 *        CRBNode **i, *p;
 *
 *        i = &t->root;
 *        p = NULL;
 *        while (*i) {
 *                p = *i;
 *                if (compare(n, *i) < 0)
 *                        i = &(*i)->left;
 *                else
 *                        i = &(*i)->right;
 *        }
 *
 *        c_rbtree_add(t, p, i, n);
 *
 * Once the node is linked into the tree, a simple lookup on the same tree can
 * be coded like this:
 *
 *        CRBNode *i;
 *
 *        i = t->root;
 *        while (i) {
 *                int v = compare(n, i);
 *                if (v < 0)
 *                        i = (*i)->left;
 *                else if (v > 0)
 *                        i = (*i)->right;
 *                else
 *                        break;
 *        }
 *
 * When you add nodes to a tree, the memory contents of the node do not matter.
 * That is, there is no need to initialize the node via c_rbnode_init().
 * However, if you relink nodes multiple times during their lifetime, it is
 * usually very convenient to use c_rbnode_init() and c_rbtree_remove_init().
 * In those cases, you should validate that a node is unlinked before you call
 * c_rbtree_add().
 */
_public_ void c_rbtree_add(CRBTree *t, CRBNode *p, CRBNode **l, CRBNode *n) {
        assert(t);
        assert(l);
        assert(n);
        assert(!p || l == &p->left || l == &p->right);
        assert(p || l == &t->root);

        c_rbnode_set_parent_and_color(n, p, C_RBNODE_RED);
        c_rbtree_store(&n->left, NULL);
        c_rbtree_store(&n->right, NULL);
        c_rbtree_store(l, n);

        c_rbtree_paint(t, n);
}

static inline CRBNode *c_rbtree_rebalance_one(CRBTree *t, CRBNode *p, CRBNode *n) {
        CRBNode *s, *x, *y, *g;

        /*
         * Rebalance tree after a node was removed. This happens only if you
         * remove a black node and one path is now left with an unbalanced
         * number or black nodes.
         * This function assumes all paths through p and n have one black node
         * less than all other paths. If recursive fixup is required, the
         * current node is returned.
         */

        if (n == p->left) {
                s = p->right;
                if (c_rbnode_is_red(s)) {
                        /* Case 3:
                         * We have a red node as sibling. Rotate it onto our
                         * side so we can later on turn it black. This way, we
                         * gain the additional black node in our path. */
                        g = c_rbnode_parent(p);
                        x = s->left;
                        c_rbtree_store(&p->right, x);
                        c_rbtree_store(&s->left, p);
                        c_rbnode_set_parent_and_color(x, p, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(s, g, c_rbnode_color(p));
                        c_rbnode_set_parent_and_color(p, s, C_RBNODE_RED);
                        c_rbtree_swap_child(t, g, p, s);
                        s = x;
                }

                x = s->right;
                if (!x || c_rbnode_is_black(x)) {
                        y = s->left;
                        if (!y || c_rbnode_is_black(y)) {
                                /* Case 4:
                                 * Our sibling is black and has only black
                                 * children. Flip it red and turn parent black.
                                 * This way we gained a black node in our path,
                                 * or we fix it recursively one layer up, which
                                 * will rotate the red sibling as parent. */
                                c_rbnode_set_parent_and_color(s, p, C_RBNODE_RED);
                                if (c_rbnode_is_black(p))
                                        return p;

                                c_rbnode_set_parent_and_color(p, c_rbnode_parent(p), C_RBNODE_BLACK);
                                return NULL;
                        }

                        /* Case 5:
                         * Left child of our sibling is red, right one is black.
                         * Rotate on parent so the right child of our sibling is
                         * now red, and we can fall through to case 6. */
                        x = y->right;
                        c_rbtree_store(&s->left, y->right);
                        c_rbtree_store(&y->right, s);
                        c_rbtree_store(&p->right, y);
                        if (x)
                                c_rbnode_set_parent_and_color(x, s, C_RBNODE_BLACK);
                        x = s;
                        s = y;
                }

                /* Case 6:
                 * The right child of our sibling is red. Rotate left and flip
                 * colors, which gains us an additional black node in our path,
                 * that was previously on our sibling. */
                g = c_rbnode_parent(p);
                y = s->left;
                c_rbtree_store(&p->right, y);
                c_rbtree_store(&s->left, p);
                c_rbnode_set_parent_and_color(x, s, C_RBNODE_BLACK);
                if (y)
                        c_rbnode_set_parent_and_color(y, p, c_rbnode_color(y));
                c_rbnode_set_parent_and_color(s, g, c_rbnode_color(p));
                c_rbnode_set_parent_and_color(p, s, C_RBNODE_BLACK);
                c_rbtree_swap_child(t, g, p, s);
        } else /* if (!n || n == p->right) */ { /* same as above, but mirrored */
                s = p->left;
                if (c_rbnode_is_red(s)) {
                        g = c_rbnode_parent(p);
                        x = s->right;
                        c_rbtree_store(&p->left, x);
                        c_rbtree_store(&s->right, p);
                        c_rbnode_set_parent_and_color(x, p, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(s, g, C_RBNODE_BLACK);
                        c_rbnode_set_parent_and_color(p, s, C_RBNODE_RED);
                        c_rbtree_swap_child(t, g, p, s);
                        s = x;
                }

                x = s->left;
                if (!x || c_rbnode_is_black(x)) {
                        y = s->right;
                        if (!y || c_rbnode_is_black(y)) {
                                c_rbnode_set_parent_and_color(s, p, C_RBNODE_RED);
                                if (c_rbnode_is_black(p))
                                        return p;

                                c_rbnode_set_parent_and_color(p, c_rbnode_parent(p), C_RBNODE_BLACK);
                                return NULL;
                        }

                        x = y->left;
                        c_rbtree_store(&s->right, y->left);
                        c_rbtree_store(&y->left, s);
                        c_rbtree_store(&p->left, y);
                        if (x)
                                c_rbnode_set_parent_and_color(x, s, C_RBNODE_BLACK);
                        x = s;
                        s = y;
                }

                g = c_rbnode_parent(p);
                y = s->right;
                c_rbtree_store(&p->left, y);
                c_rbtree_store(&s->right, p);
                c_rbnode_set_parent_and_color(x, s, C_RBNODE_BLACK);
                if (y)
                        c_rbnode_set_parent_and_color(y, p, c_rbnode_color(y));
                c_rbnode_set_parent_and_color(s, g, c_rbnode_color(p));
                c_rbnode_set_parent_and_color(p, s, C_RBNODE_BLACK);
                c_rbtree_swap_child(t, g, p, s);
        }

        return NULL;
}

static inline void c_rbtree_rebalance(CRBTree *t, CRBNode *p) {
        CRBNode *n = NULL;

        assert(t);
        assert(p);

        do {
                n = c_rbtree_rebalance_one(t, p, n);
                p = n ? c_rbnode_parent(n) : NULL;
        } while (p);
}

/**
 * c_rbtree_remove() - remove node from tree
 * @t:          tree to operate one
 * @n:          node to remove
 *
 * This removes the given node from its tree. Once unlinked, the tree is
 * rebalanced.
 * The caller *must* ensure that the given tree is actually the tree it is
 * linked on. Otherwise, behavior is undefined.
 *
 * This does *NOT* reset @n to being unlinked (for performance reason, this
 * function *never* modifies @n at all). If you need this, use
 * c_rbtree_remove_init().
 */
_public_ void c_rbtree_remove(CRBTree *t, CRBNode *n) {
        CRBNode *p, *s, *gc, *x, *next = NULL;
        unsigned long c;

        assert(t);
        assert(n);
        assert(c_rbnode_is_linked(n));

        /*
         * There are three distinct cases during node removal of a tree:
         *  * The node has no children, in which case it can simply be removed.
         *  * The node has exactly one child, in which case the child displaces
         *    its parent.
         *  * The node has two children, in which case there is guaranteed to
         *    be a successor to the node (successor being the node ordered
         *    directly after it). This successor cannot have two children by
         *    itself (two interior nodes can never be successive). Therefore,
         *    we can simply swap the node with its successor (including color)
         *    and have reduced this case to either of the first two.
         *
         * Whenever the node we removed was black, we have to rebalance the
         * tree. Note that this affects the actual node we _remove_, not @n (in
         * case we swap it).
         *
         *      p: parent
         *      s: successor
         *      gc: grand-...-child
         *      x: temporary
         *      next: next node to rebalance on
         */

        if (!n->left) {
                /*
                 * Case 1:
                 * The node has no left child. If it neither has a right child,
                 * it is a leaf-node and we can simply unlink it. If it also
                 * was black, we have to rebalance, as always if we remove a
                 * black node.
                 * But if the node has a right child, the child *must* be red
                 * (otherwise, the right path has more black nodes as the
                 * non-existing left path), and the node to be removed must
                 * hence be black. We simply replace the node with its child,
                 * turning the red child black, and thus no rebalancing is
                 * required.
                 */
                p = c_rbnode_parent(n);
                c = c_rbnode_color(n);
                c_rbtree_swap_child(t, p, n, n->right);
                if (n->right)
                        c_rbnode_set_parent_and_color(n->right, p, c);
                else
                        next = (c == C_RBNODE_BLACK) ? p : NULL;
        } else if (!n->right) {
                /*
                 * Case 1.1:
                 * The node has exactly one child, and it is on the left. Treat
                 * it as mirrored case of Case 1 (i.e., replace the node by its
                 * child).
                 */
                p = c_rbnode_parent(n);
                c = c_rbnode_color(n);
                c_rbtree_swap_child(t, p, n, n->left);
                c_rbnode_set_parent_and_color(n->left, p, c);
        } else {
                /*
                 * Case 2:
                 * We are dealing with a full interior node with a child not on
                 * both sides. Find its successor and swap it. Then remove the
                 * node similar to Case 1. For performance reasons we don't
                 * perform the full swap, but skip links that are about to be
                 * removed, anyway.
                 */
                s = n->right;
                if (!s->left) {
                        /* right child is next, no need to touch grandchild */
                        p = s;
                        gc = s->right;
                } else {
                        /* find successor and swap partially */
                        s = c_rbnode_leftmost(s);
                        p = c_rbnode_parent(s);

                        gc = s->right;
                        c_rbtree_store(&p->left, s->right);
                        c_rbtree_store(&s->right, n->right);
                        c_rbnode_set_parent(n->right, s);
                }

                /* node is partially swapped, now remove as in Case 1 */
                c_rbtree_store(&s->left, n->left);
                c_rbnode_set_parent(n->left, s);

                x = c_rbnode_parent(n);
                c = c_rbnode_color(n);
                c_rbtree_swap_child(t, x, n, s);
                if (gc)
                        c_rbnode_set_parent_and_color(gc, p, C_RBNODE_BLACK);
                else
                        next = c_rbnode_is_black(s) ? p : NULL;
                c_rbnode_set_parent_and_color(s, x, c);
        }

        if (next)
                c_rbtree_rebalance(t, next);
}
