/**
 * @file dd_walker.cc
 * @brief DD leaves walker
 *
 * Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **/
#include <common.hh>

#include <dd_walker.hh>

DDWalker::DDWalker(CuddMgr& owner)
    : f_owner(owner)
{}

DDWalker::~DDWalker()
{}

DDLeafWalker::~DDLeafWalker()
{}

DDNodeWalker::~DDNodeWalker()
{}

DDLeafWalker::DDLeafWalker(CuddMgr& owner)
    : DDWalker(owner)
    , f_data(NULL)
{}

DDNodeWalker::DDNodeWalker(CuddMgr& owner)
    : DDWalker(owner)
{}

void DDLeafWalker::pre_hook()
{
    assert (NULL == f_data);
    size_t nelems = f_owner.dd().getManager()->size;
    f_data = (char *) malloc( nelems * sizeof(char));

    /* 0 is false, 1 is true, 2 is don't care */
    for (unsigned i = 0; i < nelems; ++ i) {
        *( f_data + i ) = 2;
    }
}

void DDNodeWalker::pre_hook()
{}

void DDLeafWalker::post_hook()
{
    /* release temp memory */
    free(f_data);
    f_data = NULL;
}

void DDNodeWalker::post_hook()
{}

DDWalker& DDWalker::operator() (ADD dd)
{
    dd_activation_record call(dd.getNode());

    // setup toplevel act. record and perform walk
    f_recursion_stack.push(call);

    pre_hook();

    walk();

    post_hook();

    return *this;
}

/* pre-order visit strategy */
void DDLeafWalker::walk ()
{
    DdNode *N, *Nv, *Nnv;

    /* for fast access to the DD variables array */
    char *cell;

    while (0 != f_recursion_stack.size()) {
    call:
        dd_activation_record curr = f_recursion_stack.top();
        N = Cudd_Regular(curr.node);

        /* if node is a constand and fulfills condition, perform
           action on it. Recur into its children
           afterwards. (pre-order). */
        if ( cuddIsConstant(N) ) {
            if (condition(curr.node)) {
                action(curr.node);
            }

            // continue
            goto entry_RETURN;
        }

        /* init common to all paths below here */
        cell = f_data + N->index;

        if (Cudd_IsComplement(curr.node)) {
            Nv  = Cudd_Not(cuddT(N));
            Nnv = Cudd_Not(cuddE(N));
        }
        else {
            Nv  = cuddT(N);
            Nnv = cuddE(N);
        }

        // restore caller location (simulate call return behavior)
        if (curr.pc != DD_DEFAULT) {
            switch(curr.pc) {
            case DD_ELSE: goto entry_ELSE;
            case DD_RETURN: goto entry_RETURN;
            default: assert( false ); // unexpected
            }
        } /* pc != DEFAULT */

        // entry_THEN: (fallthru)
        *cell = 0; /* false */
        f_recursion_stack.top().pc = DD_ELSE;
        f_recursion_stack.push(dd_activation_record(Nnv));
        goto call;

    entry_ELSE:
        *cell = 1; /* true */
        f_recursion_stack.top().pc = DD_RETURN;
        f_recursion_stack.push(dd_activation_record(Nv));
        goto call;

    entry_RETURN:
        *cell = 2; /* don't care */

        f_recursion_stack.pop();
    } // while
}

/* post-order visit strategy */
void DDNodeWalker::walk ()
{
    dd_activation_record curr = f_recursion_stack.top();

    while(0 != f_recursion_stack.size()) {
    call:
        dd_activation_record curr = f_recursion_stack.top();

        /* skip constants */
        if (cuddIsConstant(curr.node)) {
            f_recursion_stack.pop();
            continue;
        }

        // restore caller location (simulate call return behavior)
        if (curr.pc != DD_DEFAULT) {
            switch(curr.pc) {
            case DD_ELSE: goto entry_node_ELSE;
            case DD_RETURN: goto entry_node_ACTION;
            default: assert( false ); // unexpected
            }
        } /* pc != DEFAULT */

        /* if node is not a constant and fulfills condition, perform
           action on it. Recur into its children afterwards (pre-order). */
        if (condition(curr.node)) {
            /* recur in THEN */
            f_recursion_stack.top().pc = DD_ELSE;
            f_recursion_stack.push(dd_activation_record(cuddT(curr.node)));
            goto call;

        entry_node_ELSE:
            /* recur in ELSE */
            f_recursion_stack.top().pc = DD_RETURN;
            f_recursion_stack.push(dd_activation_record(cuddE(curr.node)));
            goto call;

        entry_node_ACTION:
            /* process node in post-order. */
            action(curr.node);
        }

        f_recursion_stack.pop();
    } // while
}
