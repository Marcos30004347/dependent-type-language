#include "ast.h"
#include "hash.h"

#include <cstdlib>
#include <cstring>
#include <sys/cdefs.h>
#include <sys/signal.h>


enum positions_type_t {
	POSITION_HERE,
	POSITION_JOIN,
};

typedef struct position_tree_t {
	enum positions_type_t kind;

	struct position_tree_t* lhs;
	struct position_tree_t* rhs;

} position_tree_t;

unsigned position_tree_tokens_count(struct position_tree_t * tree) {
	if(tree == 0 || tree->kind == POSITION_HERE) return 1;
	
	unsigned length = 1;
	
	length += position_tree_tokens_count(tree->lhs);
	length += position_tree_tokens_count(tree->rhs);

	return length;
}

void position_tree_to_compressed_string(struct position_tree_t * tree, char * buffer, unsigned length, unsigned bit) {
	if(tree == 0) return;
	
	char n = 0;
	char h = 1;
	char j = 2;

	char c = tree == 0 ? n : tree->kind == POSITION_JOIN ? j : h; 
	
	*buffer |= (c << bit);

	bit += 2;
	buffer += bit / 8;
	bit = bit % 8;

	if(c == h) return;

	position_tree_to_compressed_string(tree->lhs, buffer, length, bit);
	position_tree_to_compressed_string(tree->rhs, buffer, length, bit);
}


struct position_tree_t * position_tree_here() {
	struct position_tree_t * pos = (position_tree_t*)malloc(sizeof(struct position_tree_t));

	pos->kind = POSITION_HERE;

	pos->lhs = 0;
	pos->rhs = 0;

	return pos;
}

struct position_tree_t * position_tree_join(struct position_tree_t * lhs, struct position_tree_t * rhs) {
	struct position_tree_t * pos = (position_tree_t*)malloc(sizeof(struct position_tree_t));

	pos->kind = POSITION_JOIN;

	pos->lhs = lhs;
	pos->rhs = rhs;

	return pos;
}

void position_tree_free(struct position_tree_t * p) {
	if(p == 0) return;
	
	position_tree_free(p->lhs);
	position_tree_free(p->rhs);

	free(p);
}

struct position_tree_t * position_tree_copy(struct position_tree_t * p) {
	if(p == 0) return 0;
	
	struct position_tree_t * pos = (position_tree_t*)malloc(sizeof(struct position_tree_t));
	
	pos->kind = p->kind;

	pos->lhs = position_tree_copy(p->lhs);
	pos->rhs = position_tree_copy(p->rhs);

	return pos;
}

typedef struct variable_map_t {
	unsigned size;
	unsigned capacity;
	
	name_t** names;
	struct position_tree_t ** trees;
} variable_map_t;


typedef struct summary_t {
	struct ast_t * structure;
	struct variable_map_t * variable_map;
	struct position_tree_t * position;

	unsigned tag;
	unsigned left_bigger;

	struct summary_t * lhs;
	struct summary_t * rhs;
} summary_t;

struct variable_map_t* variable_map_allocate() {
	struct variable_map_t * vm = (struct variable_map_t*)malloc(sizeof(struct variable_map_t));
	
	vm->capacity = 4;
	vm->size = 0;

	vm->names = (struct name_t**)malloc(sizeof(name_t*) * vm->capacity);
	vm->trees = (struct position_tree_t**)malloc(sizeof(position_tree_t*) * vm->capacity);

	for(unsigned i = 0; i < vm->capacity; i++) {
		vm->names[i] = 0;
		vm->trees[i] = 0;
	}

	return vm;
}

void variable_map_free(struct variable_map_t* vm) {
	for(unsigned i = 0; i < vm->capacity; i++) {
		if(vm->names[i]) {
			name_free(vm->names[i]);
			position_tree_free(vm->trees[i]);
		}
	}


	free(vm->names);
	free(vm->trees);
}
void print_position_tree(struct position_tree_t * tree) {
	if(tree == 0) return;
	
	switch(tree->kind) {
	case POSITION_HERE:
		printf("Here");
		break;
	case POSITION_JOIN:
		printf("Joint");
		printf("(");
		print_position_tree(tree->lhs);
		printf(")");
		printf("(");
		print_position_tree(tree->rhs);
		printf(")");
		break;
	}
}
void print_map(struct variable_map_t * map) {
	int printed = 0;
	
	for(int i = 0; i < map->capacity; i++) {
		if(map->names[i]) {
			printf("%s=", name_get_str(map->names[i]));
			print_position_tree(map->trees[i]);

			if(printed < map->size - 1) {
				printf(", ");
			}

			printed += 1;
		}
	}
}

void variable_map_rehash(struct variable_map_t * vm) {
	if(vm->size == 0) return;
	
	float load = vm->size / (float)vm->capacity;
	
	unsigned overloaded = load > 0.8f;
	unsigned underloaded = load < 0.5f;
	
	if (!overloaded && !underloaded) {
		return;
	}

	struct name_t ** names = vm->names;
	struct position_tree_t ** trees = vm->trees;

	unsigned old_cap = vm->capacity;
	unsigned new_cap = overloaded ? old_cap * 1.3f + 2 : underloaded ? vm->capacity * 0.7f : 0;

	if(new_cap == 0) return;
	
	vm->capacity = new_cap;
	
	vm->names = (struct name_t**)malloc(sizeof(name_t*) * vm->capacity);
	vm->trees = (struct position_tree_t**)malloc(sizeof(position_tree_t*) * vm->capacity);

	for(unsigned i = 0; i < vm->capacity; i++) {
		vm->names[i] = 0;
		vm->trees[i] = 0;
	}

	for(unsigned i = 0; i < old_cap; i++) {
		if(names[i]) {
			unsigned id = names[i]->hash.crc32 % vm->capacity;

			while(vm->names[id] != 0) {
				id += 1;
				id %= vm->capacity;
			}
			
			vm->names[id] = names[i];
			vm->trees[id] = trees[i];
		}
	}

	free(names);
	free(trees);
}

int variable_map_add(struct variable_map_t * vm, struct name_t* name, struct position_tree_t * pos_tree) {
	unsigned id = name->hash.crc32 % vm->capacity;

	while(vm->names[id]) {
		if (vm->names[id]->hash.crc32 == name->hash.crc32 && strcmp(name_get_str(vm->names[id]), name_get_str(name)) == 0) {
			return 0;
		}
		
		id = (id + 1) % vm->capacity;
	}
	
	vm->names[id] = name;
	vm->trees[id] = pos_tree;
	
	vm->size += 1;

	variable_map_rehash(vm);

	return 1;
}

struct position_tree_t * variable_map_rem(struct variable_map_t * vm, struct name_t* name) {
	if(name == 0) return 0;
	
	struct hash_t hash = name->hash;

	unsigned id = hash.crc32 % vm->capacity;
	unsigned tmp = id;
	
	while (vm->names[id]) {
		if(vm->names[id]->hash.crc32 == name->hash.crc32 && strcmp(vm->names[id]->identifier, name->identifier) == 0) {
			break;
		}
		
		id = (id + 1) % vm->capacity;
	}

	if(vm->names[id] == 0 ||vm->names[id]->hash.crc32 != name->hash.crc32)  return 0;
	
	name_free(vm->names[id]);

	struct position_tree_t * tree = vm->trees[id];

	vm->names[id] = 0;
	vm->trees[id] = 0;

	if(vm->size != vm->capacity && vm->names[(id + 1) % vm->capacity] != 0) {
		while(vm->names[(id + 1) % vm->capacity] != 0) {
			vm->names[id] = vm->names[(id + 1) % vm->capacity];
			vm->trees[id] = vm->trees[(id + 1) % vm->capacity];
 
			id = (id + 1) % vm->capacity;
		}

		vm->names[id] = vm->names[(id + 1) % vm->capacity];
		vm->trees[id] = vm->trees[(id + 1) % vm->capacity];
	}
	
	vm->size -= 1;

	variable_map_rehash(vm);

	return tree;
}

struct position_tree_t * variable_map_get(struct variable_map_t * vm, struct name_t* name) {
	if(name->identifier == 0) return 0;
	
	struct hash_t hash = name->hash;
	
	unsigned id = hash.crc32 % vm->capacity;
	
	while (vm->names[id]) {
		if(vm->names[id]->hash.crc32 == name->hash.crc32 && strcmp(vm->names[id]->identifier, name->identifier) == 0) {
			return vm->trees[id];
		}
		
		id = (id + 1) % vm->capacity;
	}
	
	return 0;
}

struct variable_map_t * variable_map_copy(struct variable_map_t * vm) {
	struct variable_map_t* copy = (struct variable_map_t*)malloc(sizeof(struct variable_map_t));

	copy->capacity = vm->capacity;
	copy->size = vm->size;

	copy->names = (name_t**)malloc(sizeof(name_t*) * copy->capacity);
	copy->trees = (position_tree_t**)malloc(sizeof(position_tree_t*) * copy->capacity);

	for(unsigned i = 0; i < vm->capacity; i++) {
		if(vm->names[i]) {
			copy->names[i] = name_copy(vm->names[i]);
			copy->trees[i] = position_tree_copy(vm->trees[i]);
		} else {
			copy->names[i] = 0;
			copy->trees[i] = 0;
		}
	}

	return copy;
}


struct variable_map_t * variable_map_merge(struct variable_map_t * bigger, struct variable_map_t * smaller) {
	struct variable_map_t* vm = variable_map_copy(bigger);

	for(unsigned i = 0; i < smaller->capacity; i++) {
		if(smaller->names[i]) {
			struct position_tree_t * tree = variable_map_rem(vm, smaller->names[i]);

			variable_map_add(vm, name_copy(smaller->names[i]), position_tree_join(tree, position_tree_copy(smaller->trees[i])));
		}
	}

	return vm;
}

struct summary_t * create_summary_var(struct ast_t * expr, struct name_t * name, struct position_tree_t* pos) {
	struct summary_t * summary = (summary_t*)malloc(sizeof(struct summary_t));

	summary->position = 0;
	
	summary->variable_map = variable_map_allocate();
	variable_map_add(summary->variable_map, name, pos);

	summary->structure = expr;

	summary->lhs = 0;
	summary->rhs = 0;
	summary->left_bigger = 0;
	
	return summary;
}


struct summary_t * create_summary_lambda(struct ast_t * expr, struct position_tree_t* pos, struct variable_map_t * vm, struct summary_t * lhs, struct summary_t * rhs) {
	struct summary_t * summary = (summary_t*)malloc(sizeof(struct summary_t));

	summary->position = pos;
	summary->variable_map = vm;
	summary->structure = expr;
	summary->tag = rhs->tag; // use the body tag
	summary->lhs = lhs;
	summary->rhs = rhs;
	summary->left_bigger = 0;

	return summary;
}

struct summary_t * create_summary_generic(struct ast_t * expr,  unsigned left_bigger, struct variable_map_t * vm, struct summary_t * lhs, struct summary_t * rhs) {
	struct summary_t * summary = (summary_t*)malloc(sizeof(struct summary_t));

	summary->position = 0;
	summary->structure = expr;
	summary->variable_map = vm;
	summary->lhs = lhs;
	summary->rhs = rhs;
	summary->left_bigger = left_bigger;
	summary->tag = (left_bigger ? lhs ? lhs->tag : 0 : rhs ? rhs->tag : 0) + 1;
	
	return summary;
}

struct variable_map_t* merge_summaries_variable_maps(struct summary_t * lhs_summary , struct summary_t * rhs_summary, int * left_bigger) {
	if(lhs_summary == 0 && rhs_summary == 0) {
		return 0;
	}
	
	if(lhs_summary == 0) {
		*left_bigger = 0;
		return variable_map_copy(rhs_summary->variable_map);
	}
	
	if(rhs_summary == 0) {
		*left_bigger = 1;
		return variable_map_copy(lhs_summary->variable_map);
	}
	
	*left_bigger = lhs_summary->variable_map->size >= rhs_summary->variable_map->size;

	struct variable_map_t * bigger_vm = *left_bigger ? lhs_summary->variable_map : rhs_summary->variable_map;
	struct variable_map_t * smaller_vm = *left_bigger ? rhs_summary->variable_map : lhs_summary->variable_map;

	return variable_map_merge(bigger_vm, smaller_vm);
}

void print_structure(struct summary_t * summary) {
	if(summary==0) return;
	switch(summary->structure->kind) {
	case VAR: {
		printf("*");
		return;
	}
	case LAMBDA: {
		printf("fn ");
		printf(" %s ", summary->left_bigger ? "lbigger" : "rbigger");
		print_position_tree(summary->position);
		printf(". ");
		print_structure(summary->rhs);
		return; 
	}
	case APP:{
		printf("%s[", summary->left_bigger ? "lbigger" : "rbigger");
		print_map(summary->variable_map);
		printf("]");
		printf("(");
		print_structure(summary->lhs);
		printf(")");
		printf("(");
		print_structure(summary->rhs);
		printf(")");
		return;
	}
	case STATEMENT:{
		print_structure(summary->lhs);
		printf("\n");
		print_structure(summary->rhs);
		return;
	}
	case BIND:{
		print_structure(summary->lhs);
		//print_position_tree(summary->position);
		printf(":");
		print_structure(summary->rhs);
		return;
	}
	case ARROW_TYPE:{
		print_structure(summary->lhs);
		printf("->");
		print_structure(summary->rhs);
		return;
	}
	case ASSIGNMENT:{
		printf(" %s ", summary->left_bigger ? "lbigger" : "rbigger");
		print_position_tree(summary->position);
		printf(" = ");
		print_structure(summary->rhs);
		return;
	}
	case DECLARATION: {
		print_structure(summary->lhs);
		printf("::");
		print_structure(summary->rhs);
		return;
	}
	case TOTAL_KINDS: return;
	}
}

void print_summary(struct summary_t * summary) {
	print_structure(summary);
}

struct summary_t* summaryse(struct ast_t * expr) {
	if(expr == 0) return 0;

	struct summary_t * lhs_summary = summaryse(expr->lhs);
	struct summary_t * rhs_summary = summaryse(expr->rhs);

	int left_bigger = 0;

	switch(expr->kind) {
	case VAR: {
		return create_summary_var(expr, name_copy(expr->name), position_tree_here());
	}

	case BIND: {
		struct name_t * x_name = expr->lhs->name;
		struct variable_map_t * vm = merge_summaries_variable_maps(lhs_summary, rhs_summary, &left_bigger);
		return create_summary_generic(expr, 0, vm, lhs_summary, rhs_summary);
	}

	case LAMBDA: {
		struct name_t * x_name = expr->lhs->lhs->name;
		struct variable_map_t * vm = merge_summaries_variable_maps(lhs_summary, rhs_summary, &left_bigger);
		struct position_tree_t * x_pos = variable_map_rem(vm, x_name);
		return create_summary_lambda(expr, x_pos, vm, lhs_summary, rhs_summary);	
	}
	case DECLARATION: {
		struct variable_map_t * vm = variable_map_copy(lhs_summary->variable_map);
		return create_summary_generic(expr, 0, vm, lhs_summary, rhs_summary);
	}
		
	// merge the two variable maps, the smaller into the bigger but setting joint position nodes as the value when key exist in both position trees
	case APP:
	case STATEMENT:
	case ASSIGNMENT:
	case ARROW_TYPE: {
		struct variable_map_t * vm = merge_summaries_variable_maps(lhs_summary, rhs_summary, &left_bigger);
		return create_summary_generic(expr, left_bigger, vm, lhs_summary, rhs_summary);
	}
	default:
		printf("Unknown kind to summaryse");
		abort();
	}
}

void summary_free(struct summary_t * summary) {
	if(summary == 0) return;
	
	summary_free(summary->lhs);
	summary_free(summary->rhs);

	variable_map_free(summary->variable_map);
	position_tree_free(summary->position);
}

void variable_map_to_name_name_map(struct variable_map_t * var_map, struct name_name_map_t * name_map) {
	for(int i = 0; i < var_map->capacity; i++) {
		if(var_map->names[i]) {
			unsigned len = position_tree_tokens_count(var_map->trees[i]);

			len *= 2;

			// round to next multiple of 8
			len = ((len + 7) & (-8));

			len /= 8;
			
			char * buffer = (char*)malloc(sizeof(char) * len);
			
			buffer[len - 1] = 0;

			position_tree_to_compressed_string(var_map->trees[i], buffer, len, 0);

			struct name_t * hash = allocate_name(buffer);
			
			name_name_map_add(name_map, var_map->names[i], hash);

			free(buffer);

			position_tree_free(var_map->trees[i]);
			
			var_map->names[i] = 0;
			var_map->trees[i] = 0;
		}
	}
}

struct hash_t hash_name_name_map(struct name_name_map_t * name_map) {
	struct hash_t result = hash("");
	
	for(int i = 0; i < name_map->capacity; i++) {
		if(name_map->keys[i]) {
			struct hash_t hash = hash_combine(name_map->vals[i]->hash, name_map->keys[i]->hash);
			result = hash_combine(hash, result);
		}
	}

	return result;
}

struct hash_t hash_structure(struct ast_t * ast) {
	if(ast == 0) return hash("");

	struct hash_t lh = ast->lhs ? ast->lhs->tag : hash("");
	struct hash_t rh = ast->rhs ? ast->rhs->tag : hash("");
	
	struct hash_t hash_ast = hash_combine(lh, rh);
	struct hash_t hash_app = hash(1607021125);
	struct hash_t hash_var = hash(4218930572);
	struct hash_t hash_lbd = hash(593836036);
	struct hash_t hash_stm = hash(2216251289);
	struct hash_t hash_bnd = hash(1884888807);
	struct hash_t hash_ass = hash(995776901);
	struct hash_t hash_dcl = hash(4154476586);
	struct hash_t hash_arw = hash(1540463079);
	
	switch(ast->kind) {
	case APP: return hash_combine(hash_app, hash_ast);
	case VAR: return hash_combine(hash_var, hash_ast);
	case LAMBDA: return hash_combine(hash_lbd, hash_ast);
	case STATEMENT: return hash_combine(hash_stm, hash_ast);
	case BIND: return hash_combine(hash_bnd, hash_ast);
	case ASSIGNMENT: return hash_combine(hash_ass, hash_ast);
	case DECLARATION: return hash_combine(hash_dcl, hash_ast);
	case ARROW_TYPE: return hash_combine(hash_arw, hash_ast);
	}
	
	abort();
}

void summary_hash_structure(struct summary_t* summary) {
	if(summary == 0) return;

	summary_hash_structure(summary->lhs);
	summary_hash_structure(summary->rhs);

	summary->structure->tag = hash_structure(summary->structure);
	summary->structure->tag = hash_combine(summary->structure->tag, hash(summary->left_bigger ? "L" : "R"));
}

void summary_hash_free_variables(struct summary_t* summary) {
	if(summary == 0) return;

	struct name_name_map_t* map = name_name_map_allocate();

	variable_map_to_name_name_map(summary->variable_map, map);

	summary->structure->fv_to_ctx_map = map;

	summary_hash_free_variables(summary->lhs);
	summary_hash_free_variables(summary->rhs);

	summary->structure->tag = hash_combine(summary->structure->tag, hash_name_name_map(summary->structure->fv_to_ctx_map));
}

void summary_hash_positions(struct summary_t* summary) {
	if(summary == 0) return;

	summary_hash_positions(summary->lhs);
	summary_hash_positions(summary->rhs);

	hash_t h = hash((unsigned)0);

	if(summary->position) {
		unsigned len = position_tree_tokens_count(summary->position);

		char * buffer = (char*)malloc(sizeof(char) * len);

		position_tree_to_compressed_string(summary->position, buffer, len, 0);

		h = hash(buffer);

		free(buffer);
	}
	
	summary->structure->tag = hash_combine(summary->structure->tag, h);
}


void ast_hash(struct ast_t * ast) {
 struct summary_t * summary =	summaryse(ast);

 summary_hash_structure(summary);
 summary_hash_free_variables(summary);
 //summary_hash_positions(summary);

 summary_free(summary);
}

void print_hashed_ast(struct ast_t * ast) {
	if(ast==0) return;
	printf("%u = hash of ", ast->tag.crc32);
	ast_print(ast);
}
