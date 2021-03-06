/**
 * @file analyzer/walker.cc
 * @brief Semantic analyzer module, walker pattern implementation.
 *
 * Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 **/

#include <common/common.hh>

#include <expr/expr.hh>
#include <type/type.hh>

#include <symb/classes.hh>
#include <symb/proxy.hh>

#include <sat/sat.hh>
#include <model/analyzer/analyzer.hh>
#include <model/compiler/compiler.hh>

#include <utils/misc.hh>

bool Analyzer::walk_F_preorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_F_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_G_preorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_G_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_X_preorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_X_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_U_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_U_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_U_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_R_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_R_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_R_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_at_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_at_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_at_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_next_preorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_next_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_neg_preorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_neg_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_not_preorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_not_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_bw_not_preorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_bw_not_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_add_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_add_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_add_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_sub_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_sub_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_sub_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_div_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_div_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_div_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_mul_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_mul_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_mul_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_mod_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_mod_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_mod_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_and_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_and_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_and_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_bw_and_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_bw_and_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_bw_and_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_or_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_or_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_or_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_bw_or_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_bw_or_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_bw_or_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_bw_xor_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_bw_xor_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_bw_xor_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_bw_xnor_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_bw_xnor_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_bw_xnor_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_guard_preorder(const Expr_ptr expr)
{
    ExprMgr& em
        (owner().em());

    if (f_section == ANALYZE_INIT)
        throw SemanticError("Guards not allowed in INITs");

    if (f_section == ANALYZE_INVAR)
        throw SemanticError("Guards not allowed in INVARs");

    if (f_section == ANALYZE_DEFINE)
        throw SemanticError("Guards not allowed in DEFINEs");

    /* now we know it's a TRANS */
    if (1 != f_expr_stack.size())
        throw SemanticError("Guards are only allowed toplevel in TRANSes");

    Expr_ptr guard
        (expr->lhs());

    Expr_ptr action
        (expr->rhs());

    if (! em.is_assignment(action)) {
        throw SemanticError("Guarded actions must be assignments");
    }

    Expr_ptr lhs
        (action->lhs());

    INFO
        << "Tracking dependency: "
        << lhs
        << " -> "
        << guard
        << std::endl;

    f_dependency_tracking_map.insert(std::pair<Expr_ptr, Expr_ptr> (guard, lhs));

    return true;
}

bool Analyzer::walk_guard_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_guard_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_implies_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_implies_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_implies_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_cast_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_cast_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_cast_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_type_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_type_inorder(const Expr_ptr expr)
{ assert(false); /* unreachable */ return false; }
void Analyzer::walk_type_postorder(const Expr_ptr expr)
{ assert(false); /* unreachable */ }

bool Analyzer::walk_lshift_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_lshift_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_lshift_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_rshift_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_rshift_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_rshift_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_assignment_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_assignment_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_assignment_postorder(const Expr_ptr expr)
{
    if (f_section == ANALYZE_INIT)
        throw SemanticError("Assignments not allowed in INITs");

    if (f_section == ANALYZE_INVAR)
        throw SemanticError("Assignments not allowed in INVARs");

    if (f_section == ANALYZE_DEFINE)
        throw SemanticError("Assignments not allowed in DEFINEs");

    ExprMgr& em
        (owner().em());

    Expr_ptr lhs
        (expr->lhs());

    if (! em.is_lvalue(lhs))
        throw SemanticError("Assignments require an lvalue for lhs");

    /* strip [] */
    if (em.is_subscript(lhs))
        lhs = lhs->lhs();

    /* assignment lhs *must* be an ordinary state variable */
    Expr_ptr ctx
        (f_ctx_stack.back());
    Expr_ptr full
        (em.make_dot(ctx, lhs));

    ResolverProxy resolver;

    Symbol_ptr symb
        (resolver.symbol(full));

    if (symb->is_variable()) {

        const Variable& var
            (symb->as_variable());

        /* INPUT vars are in fact bodyless, typed DEFINEs */
        if (var.is_input())
            throw SemanticError("Assignments not allowed on input vars.");

        /* FROZEN vars can not be assigned */
        if (var.is_frozen())
            throw SemanticError("Assignments can not be used on frozen vars.");

        /* assignments can only be used on INERTIAL vars */
        if (! var.is_inertial())
            throw SemanticError("Assignments can only be used on inertial vars.");
    }

    else if (symb->is_define()) {

      Define& define
        (symb->as_define());

      Expr_ptr body
        (define.body());

      /* recur in body */
      (*this)(body);
    }
}

bool Analyzer::walk_eq_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_eq_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_eq_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_ne_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_ne_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_ne_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_gt_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_gt_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_gt_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_ge_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_ge_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_ge_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_lt_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_lt_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_lt_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_le_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_le_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_le_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_ite_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_ite_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_ite_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_cond_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_cond_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_cond_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_dot_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_dot_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_dot_postorder(const Expr_ptr expr)
{}

/* on-demand preprocessing to expand defines delegated to Preprocessor */
bool Analyzer::walk_params_preorder(const Expr_ptr expr)
{
    Expr_ptr ctx
        (f_ctx_stack.back());

    (*this)(f_owner.preprocess(expr, ctx));

    return false;
}
bool Analyzer::walk_params_inorder(const Expr_ptr expr)
{ assert( false ); return false; /* unreachable */ }
void Analyzer::walk_params_postorder(const Expr_ptr expr)
{ assert( false ); return ; /* unreachable */ }

bool Analyzer::walk_params_comma_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_params_comma_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_params_comma_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_subscript_preorder(const Expr_ptr expr)
{ return true; }
bool Analyzer::walk_subscript_inorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_subscript_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_array_preorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_array_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_array_comma_preorder(Expr_ptr expr)
{ return true; }

bool Analyzer::walk_array_comma_inorder(Expr_ptr expr)
{ return true; }

void Analyzer::walk_array_comma_postorder(Expr_ptr expr)
{}

bool Analyzer::walk_set_preorder(const Expr_ptr expr)
{ return true; }
void Analyzer::walk_set_postorder(const Expr_ptr expr)
{}

bool Analyzer::walk_set_comma_preorder(Expr_ptr expr)
{ return true; }

bool Analyzer::walk_set_comma_inorder(Expr_ptr expr)
{ return true; }

void Analyzer::walk_set_comma_postorder(Expr_ptr expr)
{}

void Analyzer::walk_leaf(const Expr_ptr expr)
{
    ExprMgr& em
        (owner().em());

    if (em.is_identifier(expr)) {

        Expr_ptr ctx
            (f_ctx_stack.back());
        Expr_ptr full
            (em.make_dot(ctx, expr));

        ResolverProxy resolver;

        Symbol_ptr symb
            (resolver.symbol(full));

        if (symb->is_variable()) {
            const Variable& var
                (symb->as_variable());

            /* Sanity checks on var modifiers */
            if (var.is_input() && var.is_inertial())
                throw SemanticError("@input and @inertial not simultaneously allowed on the same var.");

            if (var.is_input() && var.is_frozen())
                throw SemanticError("@input and @frozen not simultaneously allowed on the same var.");

            if (var.is_inertial() && var.is_frozen())
                throw SemanticError("@inertial and @frozen not simultaneously allowed on the same var.");
        }
        else if (symb->is_define()) {

            Define& define
                (symb->as_define());

            Expr_ptr body
                (define.body());

            /* recur in body */
            (*this)(body);
        }
    }
}

