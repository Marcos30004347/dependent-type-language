#ifndef NAMES_HPP
#define NAMES_HPP

#include "hash.h"
#include <cstring>
#include <malloc/_malloc.h>
#include <string.h>
#include <stdlib.h>

typedef struct name_t {
	char* identifier;
	unsigned length;
	struct hash_t hash;
} name_t;


struct name_t * allocate_name(const char * id) {
	struct name_t * name = (struct name_t*)malloc(sizeof(struct name_t));

	name->length = strlen(id);
	name->identifier = (char*)malloc(sizeof(char) * name->length);

	strcpy(name->identifier, id);
	
	name->hash = hash(id);
	
	return name;
}

void name_free(struct name_t * name) {
	free(name->identifier);
	free(name);
}

const char* name_get_str(const name_t * name) {
	return name->identifier;
}

unsigned name_get_length(const name_t * name) {
	return name->length;
}

struct name_t * name_copy(struct name_t * name) {
	struct name_t * copy = (struct name_t*)malloc(sizeof(struct name_t));

	copy->hash = name->hash;
	copy->identifier = (char*)malloc(sizeof(char) * name->length);
	copy->length = name->length;
	
	strcpy(copy->identifier, name->identifier);
	
	return copy;
}

#endif
