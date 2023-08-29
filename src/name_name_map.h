#ifndef NAME_NAME_MAP_H
#define NAME_NAME_MAP_H

#include "name.h"

typedef struct name_name_map_t {
	unsigned size;
	unsigned capacity;
	
	struct name_t ** keys;
	struct name_t ** vals;
} name_name_map_t;

struct name_name_map_t* name_name_map_allocate() {
	struct name_name_map_t * vm = (struct name_name_map_t*)malloc(sizeof(struct name_name_map_t));
	
	vm->capacity = 4;
	vm->size = 0;

	vm->keys = (struct name_t**)malloc(sizeof(name_t*) * vm->capacity);
	vm->vals = (struct name_t**)malloc(sizeof(name_t*) * vm->capacity);

	for(unsigned i = 0; i < vm->capacity; i++) {
		vm->keys[i] = 0;
		vm->vals[i] = 0;
	}

	return vm;
}

void name_name_map_free(struct name_name_map_t* vm) {
	for(unsigned i = 0; i < vm->capacity; i++) {
		if(vm->keys[i]) {
			name_free(vm->keys[i]);
			name_free(vm->vals[i]);
		}
	}


	free(vm->keys);
	free(vm->vals);
}


void name_name_map_rehash(struct name_name_map_t * vm) {
	if(vm->size == 0) return;
	
	float load = vm->size / (float)vm->capacity;

	
	unsigned overloaded = load > 0.8f;
	unsigned underloaded = load < 0.5f;
		
	if (!overloaded && !underloaded) {
		return;
	}

	struct name_t ** names = vm->keys;
	struct name_t ** trees = vm->vals;

	unsigned old_cap = vm->capacity;
	unsigned new_cap = overloaded ? old_cap * 1.3f + 2 : underloaded ? vm->capacity * 0.7f : 0;

	if(new_cap == 0) return;
	
	vm->capacity = new_cap;
	
	vm->keys = (struct name_t**)malloc(sizeof(name_t*) * vm->capacity);
	vm->vals = (struct name_t**)malloc(sizeof(name_t*) * vm->capacity);

	for(unsigned i = 0; i < vm->capacity; i++) {
		vm->keys[i] = 0;
		vm->vals[i] = 0;
	}

	for(unsigned i = 0; i < old_cap; i++) {
		if(names[i]) {
			unsigned id = names[i]->hash.crc32 % vm->capacity;

			while(vm->keys[id] != 0) {
				id += 1;
				id %= vm->capacity;
			}
			
			vm->keys[id] = names[i];
			vm->vals[id] = trees[i];
		}
	}

	free(names);
	free(trees);
}

int name_name_map_add(struct name_name_map_t * vm, struct name_t* name, struct name_t * pos_tree) {
	unsigned id = name->hash.crc32 % vm->capacity;

	while(vm->keys[id]) {
		if (vm->keys[id]->hash.crc32 == name->hash.crc32 && strcmp(name_get_str(vm->keys[id]), name_get_str(name)) == 0) {
			return 0;
		}
		
		id = (id + 1) % vm->capacity;
	}
	
	vm->keys[id] = name;
	vm->vals[id] = pos_tree;
	
	vm->size += 1;

	name_name_map_rehash(vm);

	return 1;

}

struct name_t * name_name_map_rem(struct name_name_map_t * vm, struct name_t* name) {
	if(name == 0) return 0;
	
	struct hash_t hash = name->hash;

	unsigned id = hash.crc32 % vm->capacity;
	unsigned tmp = id;
	
	while (vm->keys[id]) {
		if(vm->keys[id]->hash.crc32 == name->hash.crc32 && strcmp(vm->keys[id]->identifier, name->identifier) == 0) {
			break;
		}
		
		id = (id + 1) % vm->capacity;
	}

	if(vm->keys[id] == 0 || vm->keys[id]->hash.crc32 != name->hash.crc32) return 0;


	name_free(vm->keys[id]);

	struct name_t * tree = vm->vals[id];

	vm->keys[id] = 0;
	vm->vals[id] = 0;

	if(vm->size != vm->capacity && vm->keys[(id + 1) % vm->capacity] != 0) {
		while(vm->keys[(id + 1) % vm->capacity] != 0) {
			vm->keys[id] = vm->keys[(id + 1) % vm->capacity];
			vm->vals[id] = vm->vals[(id + 1) % vm->capacity];
 
			id = (id + 1) % vm->capacity;
		}

		vm->keys[id] = vm->keys[(id + 1) % vm->capacity];
		vm->vals[id] = vm->vals[(id + 1) % vm->capacity];
	}
	
	vm->size -= 1;

	name_name_map_rehash(vm);

	return tree;
}

struct name_t * name_name_map_get(struct name_name_map_t * vm, struct name_t* name) {
	if(name->identifier == 0) return 0;
	
	struct hash_t hash = name->hash;
	
	unsigned id = hash.crc32 % vm->capacity;
	
	while (vm->keys[id]) {
		if(vm->keys[id]->hash.crc32 == name->hash.crc32 && strcmp(vm->keys[id]->identifier, name->identifier) == 0) {
			return vm->vals[id];
		}
		
		id = (id + 1) % vm->capacity;
	}
	
	return 0;
}

struct name_name_map_t * name_name_map_copy(struct name_name_map_t * vm) {
	struct name_name_map_t* copy = (struct name_name_map_t*)malloc(sizeof(struct name_name_map_t));

	copy->capacity = vm->capacity;
	copy->size = vm->size;

	copy->keys = (name_t**)malloc(sizeof(name_t*) * copy->capacity);
	copy->vals = (name_t**)malloc(sizeof(name_t*) * copy->capacity);

	for(unsigned i = 0; i < vm->capacity; i++) {
		if(vm->keys[i]) {
			copy->keys[i] = name_copy(vm->keys[i]);
			copy->vals[i] = name_copy(vm->vals[i]);
		} else {
			copy->keys[i] = 0;
			copy->vals[i] = 0;
		}
	}

	return copy;
}

#endif
