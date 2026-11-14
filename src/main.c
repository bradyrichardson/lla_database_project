#include <stdio.h>
#include "kv.h"

int main(int argc, char** argv) {
    kv_t *table = kv_init(1024);
    
    kv_put(table, "hehe", "haha");
    kv_put(table, "hoho", "laugh");
    kv_put(table, "hehe", "anotha one");
    kv_delete(table, "hoho");
    
    for (int i = 0; i < table->capacity; i++) {
        if (table->entries[i].key != NULL) {
            kv_entry_t *curr = &table->entries[i];
            while (curr != NULL && curr->key != NULL && curr->key != TOMBSTONE) {
                printf("%d: %s -> %s\n", i, table->entries[i].key, table->entries[i].value);
                curr = curr->next;
            }
        }
    }

    kv_free(table);

    printf("Finish.");

    return 0;
}