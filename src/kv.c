#include "kv.h"
#include "string.h"

size_t hash(char *val, size_t capacity) {
    size_t hash = 0x13371337deadbeef;
    while (*val) {
        hash ^= *val;
        hash = hash << 8;
        hash += *val;
        val++;
    }
    return hash % capacity;
}

int kv_put(kv_t *db, char *key, char *value) {
    if (!db || !key || !value) return -1;
    
    size_t idx = hash(key, db->capacity);
    
    for (int i = 0; i < db->capacity - 1; i++) {
        size_t real_idx = (idx + i) % db->capacity;

        kv_entry_t *entry = &db->entries[real_idx];

        // key is already set, update
        if (entry->key && entry->key != TOMBSTONE && !strcmp(entry->key, key)) {
            char *valCopy = strdup(value);
            if (!valCopy) return -1;
            entry->value = valCopy;
            return real_idx;
        }

        // entry is null or tombstone
        if (!entry->key || entry->key == TOMBSTONE) {
            char *keyCopy = strdup(key);
            char *valueCopy = strdup(value);
            if (!keyCopy || !valueCopy) {
                free(keyCopy);
                free(valueCopy);
                return -1;
            }
            entry->key = keyCopy;
            entry->value = valueCopy;
            db->count++;
            return real_idx;
        }
    }    
    // database is occupied
    return -2;
} 

kv_t *kv_init(size_t capacity) {
    if (capacity == 0) {
        return NULL;
    }

    kv_t *table = malloc(sizeof(kv_t));

    if (table == NULL) {
        return NULL;
    }

    table->capacity = capacity;
    table->count = 0;
    table->entries = calloc(sizeof(kv_entry_t), capacity);
    if (table->entries == NULL) {
        return NULL;
    }

    return table;
}
