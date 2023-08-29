#ifndef HASH_HPP
#define HASH_HPP

#include <string.h>
#include <malloc/malloc.h>

typedef struct hash_t {
	unsigned crc32;
} hash_t;

struct hash_t hash(const char *str) {
	// crc32 algorithm
    unsigned int byte, crc, mask;
    int i = 0, j;
    crc = 0xFFFFFFFF;
    while (str[i] != 0) {
        byte = str[i];
        crc = crc ^ byte;
        for (j = 7; j >= 0; j--) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }

		struct hash_t result;

		result.crc32 = ~crc;
		
		return result;
}

struct hash_t hash(unsigned i) {
	struct hash_t result;
	result.crc32 = i;
	return result;
}

struct hash_t hash_combine(hash_t a, hash_t b) {
	unsigned seed = a.crc32;

	seed ^= b.crc32 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

	hash_t result;
	
	result.crc32 = seed;
	
	return result;
}

#endif
