#ifndef REDUCTION_HPP
#define REDUCTION_HPP

#include "ast.hpp"

int can_insert_bound_variable(struct ast_t * expr) {
	return expr->kind == STATEMENT || expr->kind == LAMBDA ? 1 : 0;
}

void offset_de_bruijn_indices_of_free_variables(struct ast_t * expr, int increment, unsigned depth) {
	if(expr == 0) return;
	
	depth = depth + can_insert_bound_variable(expr);
	
	if(expr->de_bruijn_indice != -1 && expr->de_bruijn_indice > depth) {
		expr->de_bruijn_indice += increment;
	}
	
	offset_de_bruijn_indices_of_free_variables(expr->lhs, increment, depth);
	offset_de_bruijn_indices_of_free_variables(expr->rhs, increment, depth);
}

void offset_de_bruijn_indices_of_free_variables(struct ast_t * expr, int increment) {
	offset_de_bruijn_indices_of_free_variables(expr, increment, 0);
}

void replace_in_parent(ast_t * parent, ast_t * expr, ast_t * replacement) {
	if(parent == 0) return;
	
	if(parent->lhs == expr) {
		parent->lhs = replacement;
	}

	if(parent->rhs == expr) {
		parent->rhs = replacement;
	}
}

void replace_indice_at_depth(struct ast_t * expr, unsigned from_indice, ast_t * node, int depth) {
	if(expr == 0) return;
	
	if(from_indice == -1) return;
 
	if(expr->de_bruijn_indice == from_indice + depth) {
		expr->kind = node->kind;
		expr->de_bruijn_indice = node->de_bruijn_indice;
		expr->declaration_indice = node->declaration_indice;
		for(unsigned i = 0; i < 8; i++) {
			expr->identifier[i] = node->identifier[i];
		}
		ast_free(expr->lhs);
		ast_free(expr->rhs);
		
		expr->lhs = ast_copy(node->lhs);
		if(expr->lhs) expr->lhs->parent = expr;

		expr->rhs = ast_copy(node->rhs);
		if(expr->rhs) expr->rhs->parent = expr;

		return;
	}

	replace_indice_at_depth(expr->lhs, from_indice, node, depth);

	depth += can_insert_bound_variable(expr);
	
	replace_indice_at_depth(expr->rhs, from_indice, node, depth);
}

void replace_de_bruijn_indice_in_expression_by(struct ast_t * expr, unsigned indice, struct ast_t* to) {
	replace_indice_at_depth(expr, indice, to, 0);
}

struct ast_t * get_lambda_from_variable(struct ast_t * lam) {
	if(lam == 0) return 0;
	
	if (lam->kind == VAR) {
		ast_t* decl = find_declaration(lam);
		lam = decl->parent->rhs;
	}

	if(lam->kind != LAMBDA) return 0;
	
	return lam;
}

void beta_reduction(struct ast_t * expr) {
	if (expr->kind != APPL)	return;

	struct ast_t* lam =	get_lambda_from_variable(expr->lhs);

	if(lam == 0) return;

	struct ast_t* body = ast_copy(lam->rhs);
	struct ast_t* arg  = expr->rhs;

	unsigned offset = expr->declaration_indice - lam->declaration_indice;

	offset_de_bruijn_indices_of_free_variables(body, offset);
	replace_de_bruijn_indice_in_expression_by(body, 0, arg);
	replace_in_parent(expr->parent, expr, body);
	
	ast_free(expr);
}

#endif
