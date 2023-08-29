#ifndef AST_HPP
#define AST_HPP

#include <cstdlib>
#include <cstring>
#include <assert.h>
#include <stdio.h>

#include "hash.h"
#include "name.h"
#include "name_name_map.h"

enum ast_kind_t {
	VAR = 0,
	APP,
	LAMBDA,
	STATEMENT,
	BIND,
	ASSIGNMENT,
	DECLARATION,	
	ARROW_TYPE,
	TOTAL_KINDS
};

typedef struct ast_t {
	enum ast_kind_t kind;

  struct name_t * name;
	
	struct ast_t* lhs;
	struct ast_t* rhs;
	struct ast_t* parent;

	struct hash_t tag;
	
	// variable name -> hashed position tree
	struct name_name_map_t * fv_to_ctx_map;
} ast_t;


struct ast_t * alloc_node(ast_kind_t kind) {
	struct ast_t* node = (struct ast_t *)malloc(sizeof(struct ast_t));
 
	node->kind = kind;
	node->parent = 0;
	node->lhs = 0;
	node->rhs = 0;

	node->name = 0;
	
	node->fv_to_ctx_map = name_name_map_allocate();
	
	return node;
} 

struct ast_t * var(const char * id) {
	struct ast_t * node = alloc_node(VAR);

	node->name = allocate_name(id);

	return node;
}

struct ast_t * lambda(struct ast_t * bind, struct ast_t * body) {
	struct ast_t * node = alloc_node(LAMBDA);

	assert(bind->kind == BIND);
	
	node->lhs = bind;
	node->rhs = body;

	bind->parent = node;
	body->parent = node;
	
	return node;
}

struct ast_t * app(struct ast_t * lhs, struct ast_t * rhs) {
	struct ast_t * node = alloc_node(APP);

	node->lhs = lhs;
	node->rhs = rhs;

	lhs->parent = node;
	rhs->parent = node;
	
	return node;
}

struct ast_t * bind(struct ast_t * var, struct ast_t * ty) {
	struct ast_t * node = alloc_node(BIND);

	assert(var->kind == VAR);

	node->lhs = var;
	node->rhs = ty;

	var->parent = node;
	ty->parent = node;
	
	return node;
}

struct ast_t * assign(struct ast_t * lhs, struct ast_t * rhs) {
	struct ast_t * node = alloc_node(ASSIGNMENT);

	assert(lhs->kind == BIND);
	
	node->lhs = lhs;
	node->rhs = rhs;

	lhs->parent = node;
	rhs->parent = node;

	return node;
}

struct ast_t * statement(struct ast_t * lhs, struct ast_t * rhs) {
	struct ast_t * node = alloc_node(STATEMENT);

	assert(lhs->kind == ASSIGNMENT || lhs->kind == DECLARATION);
	assert(rhs == 0 || rhs->kind == STATEMENT);
	
	node->lhs = lhs;
	node->rhs = rhs;

	lhs->parent = node;

	if(rhs) {
		rhs->parent = node;
	}

	return node;
}


struct ast_t * arrow(struct ast_t * lhs, struct ast_t * rhs) {
	struct ast_t * node = alloc_node(ARROW_TYPE);

	node->lhs = lhs;
	node->rhs = rhs;

	lhs->parent = node;
	rhs->parent = node;

	return node;
} 

struct ast_t * declaration(struct ast_t * lhs) {
	struct ast_t * node = alloc_node(DECLARATION);

	assert(lhs->kind == BIND);

	node->lhs = lhs;

	lhs->parent = node;

	return node;
} 

void ast_free_node(struct ast_t* ast) {
	if(ast == 0) return;
	
	name_free(ast->name);
	free(ast);
}

void ast_free(struct ast_t* ast) {
	if(ast == 0) return;

	ast_free(ast->lhs);
	ast_free(ast->rhs);

	ast_free_node(ast);

	if(ast->fv_to_ctx_map) {
		name_name_map_free(ast->fv_to_ctx_map);
	}
}

void ast_print(struct ast_t * expr) {
	if(expr == 0) return;
	
	if(expr->kind == STATEMENT) {
		printf("let ");
		ast_print(expr->lhs);
		if(expr->rhs) {
			printf(" in\n");
			ast_print(expr->rhs);
		} else {
			printf(";\n");
		}
	}
	
	if(expr->kind == LAMBDA) {
		printf("fn ");
		ast_print(expr->lhs);
		printf(" => ");
		ast_print(expr->rhs);
	}
	
	if(expr->kind == BIND) {
		ast_print(expr->lhs);
		printf(": ");
		ast_print(expr->rhs);
	}

	if(expr->kind == VAR) {
		printf("%s", name_get_str(expr->name));
	}

	if(expr->kind == ASSIGNMENT) {
		ast_print(expr->lhs);
		printf(" = ");
		ast_print(expr->rhs);
	}

	if(expr->kind == ARROW_TYPE) {
		ast_print(expr->lhs);
		printf(" -> ");
		ast_print(expr->rhs);
	}

	if(expr->kind == APP) {
		printf("(");
		ast_print(expr->lhs);
		printf(" ");
		ast_print(expr->rhs);
		printf(")");
	}

	if(expr->kind == DECLARATION) {
		ast_print(expr->lhs);
	}
}


#endif

