#include "hash_table.h"

THashTable* create_hash_table() {
    THashTable* hash_table = (THashTable *)calloc(1, sizeof(THashTable));
    if (hash_table == NULL)
        return NULL;
    
    hash_table->entries = (TEntry *)calloc(CAPACITY, sizeof(TEntry));
    if (hash_table->entries == NULL) {
        free(hash_table);
        return NULL;
    }
    
    hash_table->length = 0;
    hash_table->size = CAPACITY;

    return hash_table;
}

// FNV-1a hash algorithm
unsigned long long hash(char *key) {
    unsigned long long hash = FNV_OFFSET;
    unsigned int i, length = strlen(key);
    for (i = 0; i < length; i++) {
        hash ^= (unsigned long long)(unsigned char)key[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

char* get(THashTable* hash_table, char* key) {
    unsigned long long key_hash = hash(key);
    unsigned int index = key_hash % (hash_table->size - 1);

    if (hash_table->entries[index].key != NULL) {
        return hash_table->entries[index].value;
    }

    return NULL;
}

int put_entry_in_hash_table(THashTable *hash_table, char *key, char *value) {
    unsigned long long key_hash = hash(key);
    unsigned int index = key_hash % (hash_table->size - 1);

    if (hash_table->entries[index].key != NULL) {
        // free the previous value and update with the new one
        free(hash_table->entries[index].value);
        hash_table->entries[index].value = strdup(value);
        if (hash_table->entries[index].value == NULL)
            return -1;
        return 1;
    }

    hash_table->entries[index].key = strdup(key);
    if (hash_table->entries[index].key == NULL)
        return -1;

    hash_table->entries[index].value = strdup(value);
    if (hash_table->entries[index].value == NULL) {
        free(hash_table->entries[index].key);
        return -1;
    }

    hash_table->length++;
    return 1;
}

int realloc_hash_table(THashTable *hash_table) {
    TEntry *old_entries = (TEntry *)calloc(hash_table->size, sizeof(TEntry));
    if (old_entries == NULL)
        return -1;
    
    TEntry *new_entries = (TEntry *)calloc(hash_table->size * 2, sizeof(TEntry));
    if (new_entries == NULL) {
        free(old_entries);
        return -1;
    }

    unsigned int i;
    for (i = 0; i < hash_table->size; i++) {
        if (hash_table->entries[i].key != NULL) {
            old_entries[i].key = (char *)calloc(strlen(hash_table->entries[i].key) + 1, sizeof(char));
            memcpy(old_entries[i].key, hash_table->entries[i].key, strlen(hash_table->entries[i].key));
            free(hash_table->entries[i].key);
        }
        
        if (hash_table->entries[i].value != NULL) {
            old_entries[i].value = (char *)calloc(strlen(hash_table->entries[i].value) + 1, sizeof(char));
            memcpy(old_entries[i].value, hash_table->entries[i].value, strlen(hash_table->entries[i].value));
            free(hash_table->entries[i].value);
        }
    }
    
    free(hash_table->entries);
    unsigned int old_size = hash_table->size;
    hash_table->size *= 2;
    hash_table->length = 0;
    hash_table->entries = new_entries;
    for (i = 0; i < old_size; i++) {
        if (old_entries[i].key != NULL) {
            int ret_value = put_entry_in_hash_table(hash_table, old_entries[i].key, old_entries[i].value);
            if (ret_value == -1)
                return -1;
            free(old_entries[i].key);
            free(old_entries[i].value);
        }
    }

    free(old_entries);
    return 1;
}

int put(THashTable *hash_table, char *key, char *value) {
    // value and key should not be null
    if (key == NULL)
        return -1;

    if (hash_table->length >= hash_table->size / 2) {
        int ret_value = realloc_hash_table(hash_table);
        if (ret_value == -1)
            return -1;
    }

    return put_entry_in_hash_table(hash_table, key, value);
}

void remove_key(THashTable *hash_table, char *key) {
    unsigned long long key_hash = hash(key);
    unsigned int index = key_hash % (hash_table->size - 1);
    if (hash_table->entries[index].key == NULL)
        return;

    free(hash_table->entries[index].key);
    if (hash_table->entries[index].value)
        free(hash_table->entries[index].value);
    
    hash_table->entries[index].key = NULL;
    hash_table->entries[index].value = NULL;
    hash_table->length--;   
}

void destroy_hash_table(THashTable *hash_table) {
    unsigned int i;
    for (i = 0; i < hash_table->size; i++) {
        if (hash_table->entries[i].key != NULL)
            free(hash_table->entries[i].key);
        if (hash_table->entries[i].value != NULL)
            free(hash_table->entries[i].value);
    }

    if (hash_table->entries != NULL)
        free(hash_table->entries);
    free(hash_table);
}
