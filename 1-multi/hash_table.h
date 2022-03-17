#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 20
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
THashTable *create_hash_table();

// get the value from the hash table based on the given key
char *get(THashTable *hash_table, char *key);

// put the value in the hash map
int put(THashTable *hash_table, char *key, char *value);

// free memory for the hash_table
void destroy_hash_table(THashTable *hash_table);

// remove the key from the hash table
void remove_key(THashTable *hash_table, char *key);