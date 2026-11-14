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

    // 1. If the bucket is empty or contains a tombstone, claim it directly
    if (!entry->key || entry->key == TOMBSTONE) {
        // If it was a tombstone, free its old value first if necessary
        if (entry->key == TOMBSTONE) {
            free(entry->value);
        }

        char *keyCopy = strdup(key);
        if (!keyCopy) return -1;
        
        char *valueCopy = strdup(value);
        if (!valueCopy) {
            free(keyCopy);
            return -1;
        }

        entry->key = keyCopy;
        entry->value = valueCopy;
        db->count++;
        return 0;
    }

    // 2. Traverse the primary entry and any collision nodes
    kv_entry_t *curr = entry;
    while (curr != NULL) {
        // If key already exists, update its value
        if (curr->key && curr->key != TOMBSTONE && !strcmp(curr->key, key)) {
            free(curr->value);
            char *valCopy = strdup(value);
            if (!valCopy) return -1;
            curr->value = valCopy;
            return 0;
        }

        // If we reached the end of the chain, break out to insert a new node
        if (curr->next == NULL) {
            break;
        }
        curr = curr->next;
    }

    // 3. Key wasn't found in the chain; append a new collision node at the end
    kv_entry_t *new_node = malloc(sizeof(kv_entry_t));
    if (!new_node) return -1;

    char *keyCopy = strdup(key);
    if (!keyCopy) {
        free(new_node);
        return -1;
    }

    char *valCopy = strdup(value);
    if (!valCopy) {
        free(keyCopy);
        free(new_node);
        return -1;
    }

    new_node->key = keyCopy;
    new_node->value = valCopy;
    new_node->next = NULL;

    // Link the new node to the end of the collision chain
    curr->next = new_node;
    db->count++;

    return 0;
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
    for (int i = 0; i < table->capacity; i++) {
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
