#ifndef IR
#define IR

#include "hash.h"
#include "name.h"
#include "ast.h"


typedef struct ast_block_t {
	struct ast_t data[64];
	ast_block_t* next;
} lambda_expr_block_t ;

typedef struct ast_manager_t {
	ast_block_t * expressions[TOTAL_KINDS];

} ast_manager_t;


#endif
