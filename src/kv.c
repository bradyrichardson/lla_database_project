#include "kv.h"
#include "string.h"


size_t hash(const char *val, size_t capacity) {
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
    kv_entry_t *entry = &db->entries[idx];

    // key is already set, update
    if (entry->key && entry->key != TOMBSTONE && !strcmp(entry->key, key)) {
        free(entry->value);
        char *valCopy = strdup(value);
        if (!valCopy) return -1;
        entry->value = valCopy;
        return 0;
    }

    // collision -> set next on entry to new entry
    if (entry->key && entry->key != TOMBSTONE && strcmp(entry->key, key)) {
        kv_entry_t *next = entry->next;
        while (next != NULL && next->key != TOMBSTONE) {
            next = next->next;
        }
        char *keyCopy = strdup(key);
        char *valCopy = strdup(value);
        next->key = keyCopy;
        next->value = valCopy;
        return 0;
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

        return 0;
    }

    return -1;
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
    table->entries = calloc(capacity, sizeof(kv_entry_t));
    if (table->entries == NULL) {
        // TODO: free malloc'd memory, is this necessary because I calloc'd it?
        free(table->entries);
        free(table);
        return NULL;
    }

    return table;
}

char *kv_get(kv_t *db, const char *key) {
    size_t idx = hash(key, db->capacity);
    
    kv_entry_t *entry = &db->entries[idx];

    if (!entry) {
        return NULL;
    }

    kv_entry_t *next = entry->next;
    
    while (next != NULL) {
        if (!strcmp(next->key, key)) {
            return next->value;
        }
        next = next->next;
    }
    if (!next) {
        return NULL;
    }

    return next->value;
}

int kv_delete(kv_t *db, const char *key) {
    size_t idx = hash(key, db->capacity);
    
    kv_entry_t *curr = &db->entries[idx];

    if (curr == NULL) {
        return -1;
    }

    while (curr->key != NULL && strcmp(curr->key, key)) {
        curr = curr->next;
    }

    free(curr->key);
    free(curr->value);
    // free(curr->next);

    curr->key = TOMBSTONE;
    curr->value = NULL;
    db->count--;

    return idx;
}

void kv_free(kv_t *table) {
    for (int i = 0; i < table->capacity; i += sizeof(kv_entry_t)) {
        kv_entry_t *curr = &table->entries[i];
        kv_entry_t *prev = curr;

        while (curr != NULL) {
            curr = curr->next;
            free(prev->key);
            free(prev->value);
            prev = curr;
        }
    }
    free(table->entries);
    free(table);
}
