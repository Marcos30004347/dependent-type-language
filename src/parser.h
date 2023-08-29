#ifndef PARSER
#define PARSER

#include "ast.h"

#include <cstdlib>


enum token_type_t {
	TOKEN_EOF = (1 << 0),
	TOKEN_LET_KEYWORD = (1 << 1),
	TOKEN_CASE_KEYWORD = (1 << 2),
	TOKEN_IN_KEYWORD = (1 << 3),
	TOKEN_FN_KEYWORD = (1 << 4),
	TOKEN_CONST_KEYWORD = (1 << 5),
	TOKEN_SEMICOLON = (1 << 6),
	TOKEN_IDENTIFIER = (1 << 7),
	TOKEN_EQUAL = (1 << 8),
	TOKEN_COLON = (1 << 9),
	TOKEN_PIPE = (1 << 10),
	TOKEN_COMMA = (1 << 11),
	TOKEN_ARROW_TYPE = (1 << 12),
	TOKEN_DOT = (1 << 13),
	TOKEN_OPENING_PARENTESIS = (1 << 14),
	TOKEN_CLOSING_PARENTESIS = (1 << 15),
	TOKEN_THEN_KEYWORD = (1 << 16),
};

typedef struct token_t {
	enum token_type_t type;
	char data[8];
	unsigned row;
	unsigned col;
	unsigned at;
} token_t;

typedef struct lexer_t {
	const char * src;
	unsigned head;

	unsigned row;
	unsigned col;
	
	token_t previous;
	token_t current;
	token_t next;
 
} lexer_t;

unsigned lexer_read_keyword(struct lexer_t* lex, const char * str) {
	unsigned i = 0;

	for(i = 0; str[i] != '\0' && i < 7; i++) {
		if(lex->src[lex->head + i] != str[i]) return 0;
	}

	if(str[i] != '\0') return 0; // TODO: report error

	lex->head += i;
	
	return i;
}

void copy_keyword_to_token(struct lexer_t * lex, struct token_t * token, unsigned len) {
	for(unsigned i = 0; i < 8; i++) {
		token->data[i] = '\0';
	}

	for(unsigned i = 0; i < len; i++) {
		token->data[i] = lex->src[lex->head - len + i];
	}
}

struct lexer_t* lexer_create(const char* src) {
	struct lexer_t* lex = (struct lexer_t*)malloc(sizeof(lexer_t));
	lex->col = 1;
	lex->row = 1;
	lex->src = src;
	lex->head = 0;
	return lex;
}

void lexer_destroy(struct lexer_t* lex) {
	free(lex);
}

unsigned lexer_is_at_stopping_symbol(struct lexer_t* lex) {
	return lex->src[lex->head] == '\0' || lex->src[lex->head] == ' ' ||	lex->src[lex->head] == '\n' || lex->src[lex->head] == ';' || lex->src[lex->head] == '(' || lex->src[lex->head] == ')' || lex->src[lex->head] == ',' || lex->src[lex->head] == '{' || lex->src[lex->head] == '}' || lex->src[lex->head] == ':' || lex->src[lex->head] == '.' || lex->src[lex->head] == '|' || lex->src[lex->head] == '=';
}

struct token_t lexer_eat(struct lexer_t * lex) {
	while(lex->src[lex->head] == '\n' || lex->src[lex->head] == ' ') {
		if(lex->src[lex->head] == '\n') {
			lex->row += 1;
			lex->col = 1;
		}
		
		lex->col += 1;
		lex->head += 1;
	}
	
	struct token_t tok;
	
	tok.col = lex->col;
	tok.row = lex->row;
	tok.at = lex->head;
	
	if(lex->src[lex->head] == '\0') {
		tok.type = TOKEN_EOF;
	}
	else if(unsigned len = lexer_read_keyword(lex, "->")) {
		tok.type = TOKEN_ARROW_TYPE;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, ".")) {
		tok.type = TOKEN_DOT;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, "let")) {
		tok.type = TOKEN_LET_KEYWORD;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, "in")) {
		tok.type = TOKEN_IN_KEYWORD;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, "fn")) {
		tok.type = TOKEN_FN_KEYWORD;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, ":")) {
		tok.type = TOKEN_COLON;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, ";")) {
		tok.type = TOKEN_SEMICOLON;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, "=")) {
		tok.type = TOKEN_EQUAL;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, "(")) {
		tok.type = TOKEN_OPENING_PARENTESIS;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, ")")) {
		tok.type = TOKEN_CLOSING_PARENTESIS;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, "|")) {
		tok.type = TOKEN_PIPE;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, ",")) {
		tok.type = TOKEN_COMMA;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, "case")) {
		tok.type = TOKEN_CASE_KEYWORD;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, "const")) {
		tok.type = TOKEN_CONST_KEYWORD;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else if(unsigned len = lexer_read_keyword(lex, "then")) {
		tok.type = TOKEN_THEN_KEYWORD;
		copy_keyword_to_token(lex, &tok, len);
		lex->col += len;
	}
	else {
		unsigned i = 0;

		for(i = 0; i < 8; i++) {
			tok.data[i] = '\0';
		}
		
		for(i = 0; lexer_is_at_stopping_symbol(lex) == 0 && i < 7; i++) {
			tok.data[i] = lex->src[lex->head++];
		}
		
		tok.type = TOKEN_IDENTIFIER;
		
		lex->col += i;
	}


	lex->previous = lex->current;
	lex->current = lex->next;
	lex->next = tok;
	
	return lex->current;
}

const char* token_type_to_str(token_type_t type) {
  switch(type) {
	case TOKEN_EOF: return "EOF";
	case TOKEN_LET_KEYWORD: return "let";
	case TOKEN_IN_KEYWORD: return "in";
	case TOKEN_FN_KEYWORD: return "fn";
	case TOKEN_SEMICOLON: return ";";
	case TOKEN_COLON: return ":";
	case TOKEN_IDENTIFIER: return "variable name";
	case TOKEN_EQUAL: return "=";
	case TOKEN_ARROW_TYPE: return "->";
	case TOKEN_DOT: return ".";
	case TOKEN_OPENING_PARENTESIS: return "(";
	case TOKEN_CLOSING_PARENTESIS: return ")";
	case TOKEN_PIPE: return "|";
	case TOKEN_COMMA: return ",";
	case TOKEN_CASE_KEYWORD: return "case";
	case TOKEN_THEN_KEYWORD: return "then";
	case TOKEN_CONST_KEYWORD: return "const";
	default: return "unknown type";
	}
}

token_t lexer_peek(struct lexer_t * lex) {
	return lex->next;
}

token_t lexer_read(struct lexer_t* lex, token_type_t type) {
	struct token_t tok = lexer_eat(lex);
	//printf("read : '%s'\n", tok.data);
	
	if(tok.type != type) {
		printf("expecting '%s', found '%s' at line %u, column %u\n", token_type_to_str(type),  token_type_to_str(tok.type), tok.row, tok.col);

		char buffer0[33] = {'\0'};
		char buffer1[33] = {'\0'};

		unsigned length = 0;

		for(length = 0; length < 8; length++) {
			if(tok.data[length] == '\0') break;
		}

		int i = 0;

		for(i = 0; i < 32; i++) {
			int at = lex->head - 16 + i;

			buffer0[i] = lex->src[at > 0 ? at : 0];

			if(buffer0[i] == '\0') break;
			
			buffer1[i] = at >= tok.at && at < tok.at + length ? '^' : '-';
		}
		
		buffer0[i + 1] = '\0';
		buffer1[i + 1] = '\0';
		
		printf("'...%s...'\n", buffer0);
		printf(" ---%s--- \n", buffer1);
		
		abort();
	}
	
	return tok;
}

int is_at_stopping_token(struct lexer_t* lex) {
	return lexer_peek(lex).type & (
															TOKEN_ARROW_TYPE |
															TOKEN_EQUAL |
															TOKEN_DOT |
															TOKEN_SEMICOLON |
															TOKEN_IN_KEYWORD |
															TOKEN_EOF |
															TOKEN_CLOSING_PARENTESIS |
															TOKEN_PIPE |
														  TOKEN_COMMA |
															TOKEN_THEN_KEYWORD);
}

struct ast_t * parse_lambda(struct lexer_t * lex);
struct ast_t * parse_app(struct lexer_t * lex);
struct ast_t * parse_bind(struct lexer_t * lex);

struct ast_t * parse_var(struct lexer_t * lex) {
	return var(lexer_read(lex, TOKEN_IDENTIFIER).data);
}

struct ast_t * parse_primary(struct lexer_t * lex) {
	if(lexer_peek(lex).type == TOKEN_OPENING_PARENTESIS) {
		lexer_read(lex, TOKEN_OPENING_PARENTESIS);

		struct ast_t * lhs = parse_app(lex);

		lexer_read(lex, TOKEN_CLOSING_PARENTESIS);

		return lhs;
	}
	
	return parse_lambda(lex);
}

struct ast_t * parse_app(struct lexer_t* lex) {
	struct ast_t * lhs = parse_primary(lex);

	if (is_at_stopping_token(lex)) return lhs;
	
	struct ast_t * rhs = parse_primary(lex);

	if (is_at_stopping_token(lex)) return app(lhs, rhs);

	return app(app(lhs, rhs), parse_app(lex));
}

struct ast_t * parse_union(struct lexer_t * lex) {
	struct ast_t * head = parse_app(lex);

	/*
	if(lexer_peek(lex).type == TOKEN_PIPE) {
		lexer_read(lex, TOKEN_PIPE);

		struct ast_t * tail = parse_union(lex);

		return union_list(head, tail->kind == UNION_LIST ? tail : union_list(tail, 0));
	}
	*/
	
	return head;
}

		/*
struct ast_t * parse_pattern_list(struct lexer_t * lex) {
		struct ast_t * head = parse_app(lex);
		if(lexer_peek(lex).type == TOKEN_THEN_KEYWORD) {
			return pattern_list(head, 0);
		}
		lexer_read(lex, TOKEN_DOT);
		
		return pattern_list(head, parse_pattern_list(lex));
}

struct ast_t* parse_case(struct lexer_t * lex) {
		lexer_read(lex, TOKEN_CASE_KEYWORD);

		struct ast_t * head = parse_pattern_list(lex);

		lexer_read(lex, TOKEN_THEN_KEYWORD);
		
		struct ast_t * body = parse_app(lex);

		struct ast_t * match = case_statement(head, body);

		if(lexer_peek(lex).type == TOKEN_COMMA) {
			lexer_read(lex, TOKEN_COMMA);

			struct ast_t * tail = parse_case(lex);

			return case_list(match, tail);
		}
		
		return case_list(match, 0);
}
/**/

struct ast_t * parse_case_list(struct lexer_t * lex) {
	/*
	if(lexer_peek(lex).type == TOKEN_CASE_KEYWORD) {
		return parse_case(lex);
	}
	*/
	
	return parse_union(lex);
}

struct ast_t * parse_type(struct lexer_t * lex) {
	if(lexer_peek(lex).type == TOKEN_CONST_KEYWORD) {
		lexer_read(lex, TOKEN_CONST_KEYWORD);
	}

	struct ast_t * lhs = parse_app(lex);

	if(lexer_peek(lex).type == TOKEN_ARROW_TYPE) {
		lexer_read(lex, TOKEN_ARROW_TYPE);
		
		struct ast_t * rhs = parse_type(lex);

		return arrow(lhs, rhs);
	}
	
	return lhs;
}

struct ast_t * parse_bind(struct lexer_t * lex) {
	
	struct ast_t * lhs = parse_var(lex);

	if (lexer_peek(lex).type == TOKEN_COLON) {
		lexer_read(lex, TOKEN_COLON);
		
		struct ast_t * rhs = parse_type(lex);
	
		return bind(lhs, rhs);
	}
	
	return lhs;
}

struct ast_t * parse_lambda(struct lexer_t * lex) {
	if(lexer_peek(lex).type == TOKEN_FN_KEYWORD) {
		lexer_read(lex, TOKEN_FN_KEYWORD);
		
	  struct ast_t * bind = parse_bind(lex);

		lexer_read(lex, TOKEN_DOT);

		struct ast_t * body = parse_app(lex);

		return lambda(bind, body);
	}

	return parse_bind(lex);
}


struct ast_t * parse_program(struct lexer_t * lex) {
	if (lexer_peek(lex).type == TOKEN_LET_KEYWORD) {
		lexer_read(lex, TOKEN_LET_KEYWORD);

		struct ast_t * lhs = parse_bind(lex);

		if(lexer_peek(lex).type == TOKEN_EQUAL) {
		
			lexer_read(lex, TOKEN_EQUAL);
		
			struct ast_t * rhs = parse_case_list(lex);

			struct ast_t * let = assign(lhs, rhs);

			if(lexer_peek(lex).type == TOKEN_SEMICOLON) {
				return statement(let, 0);
			}

			lexer_read(lex, TOKEN_IN_KEYWORD);
		
			return statement(let, parse_program(lex));
		} else {
			struct ast_t * let = declaration(lhs);

			if(lexer_peek(lex).type == TOKEN_SEMICOLON) {
				return statement(let, 0);
			}

			lexer_read(lex, TOKEN_IN_KEYWORD);
		
			return statement(let, parse_program(lex));
		}
	}

	return parse_app(lex);
}

struct ast_t * parse(const char * src) {
	struct lexer_t * lex = lexer_create(src);

	lexer_eat(lex);
	
	struct ast_t * program = parse_program(lex);

	lexer_destroy(lex);

	return program;
} 

#endif
