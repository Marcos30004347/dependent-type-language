#include "parser.h"
#include "ast_hash.h"

int main() {
	const char * src =
		"let f : t -> t = fn x:a. x in\n"
		"let g : t -> t = fn x:a. f x in\n"
		"let h : t -> t = fn x:a. g f x in\n"
		"let r : t -> t = fn x:a. r f x in\n"
		"let q : t -> t -> z = fn x:a. fn y:a. (f x) (f y);\n";
		
	const char * indexed =
		"let Nat : Type in \n"
		"let Zero : Nat in \n"
		"let Succ : Nat -> Nat in \n"
		"let Vec  : A:Type -> Nat -> Type in \n"
		"let Empty : Vec A zero in \n"
		"let Cons  : A -> Vec A n -> Vec A (Succ n);";
	
	struct ast_t * prog = parse(src);
	struct ast_t * indexed_prog = parse(indexed);

	printf("_____\n");
	ast_print(prog);
	printf("_____\n");
	ast_print(indexed_prog);


	const char * A_src = "let f : t = fn x:a. x;";
	const char * B_src = "let g : t = fn y:a. y;";
	const char * C_src = "let f : j = fn x:a. x;";
	const char * D_src = "let g : t = fn y:a. k y;";
	const char * E_src = "let g : t = fn y:a. fn k:b. k y;";
	const char * F_src = "let g : t = fn y:a. fn u:b. u y;";
	
	struct ast_t * A_prog = parse(A_src);
	struct ast_t * B_prog = parse(B_src);
	struct ast_t * C_prog = parse(C_src);
	struct ast_t * D_prog = parse(D_src);
	struct ast_t * E_prog = parse(E_src);
	struct ast_t * F_prog = parse(F_src);

	ast_hash(A_prog);
	printf(">> ");
	print_hashed_ast(A_prog->lhs->rhs);
	printf("\n");
	
	ast_hash(B_prog);
	printf(">> ");
	print_hashed_ast(B_prog->lhs->rhs);
	printf("\n");

	ast_hash(C_prog);
	printf(">> ");
	print_hashed_ast(C_prog->lhs->rhs);
	printf("\n");

	ast_hash(D_prog);
	printf(">> ");
	print_hashed_ast(D_prog->lhs->rhs);
	printf("\n");

	ast_hash(E_prog);
	printf(">> ");
	print_hashed_ast(E_prog->lhs->rhs);
	printf("\n");

	ast_hash(F_prog);
	printf(">> ");
	print_hashed_ast(F_prog->lhs->rhs);
	printf("\n");
}
