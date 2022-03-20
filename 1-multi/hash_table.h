#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define CAPACITY 383
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

typedef struct hash_table_entry {
	char *key;
	char *value;
} TEntry;

typedef struct hash_table {
	TEntry *entries;
	unsigned int size;
	unsigned int length;
} THashTable;

// allocate memory for the hash table structure
THashTable *create_hash_table(void);

// get the value mapped for the given key
char *get(THashTable *hash_table, char *key);

// put the key-value entry in the hashmap
int put(THashTable *hash_table, char *key, char *value);

// free the allocated memory for the hash_table
void destroy_hash_table(THashTable *hash_table);

// remove the key from the hash table
void remove_key(THashTable *hash_table, char *key);
