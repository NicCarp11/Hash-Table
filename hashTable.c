#include "hashTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

unsigned long hash(char *str, size_t size) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % size;
}

HashTable *create_table(size_t size) {
    HashTable *table = malloc(sizeof(HashTable));
    table->size = size;
    table->buckets = malloc(sizeof(HashNode*) * size);
    for (size_t i = 0; i < size; i++) {
        table->buckets[i] = NULL;
    }
    pthread_rwlock_init(&table->rwlock, NULL);
    return table;
}

void insert(HashTable *table, char *key, char *value) {
    unsigned long index = hash(key, table->size);
    pthread_rwlock_wrlock(&table->rwlock);
    HashNode *new_node = malloc(sizeof(HashNode));
    new_node->key = strdup(key);
    new_node->value = strdup(value);
    new_node->next = table->buckets[index];
    table->buckets[index] = new_node;
    pthread_rwlock_unlock(&table->rwlock);
}

void get_all(HashTable *table, char *key, char *buffer, size_t buffer_size) {
    unsigned long index = hash(key, table->size);
    pthread_rwlock_rdlock(&table->rwlock);
    HashNode *node = table->buckets[index];
    int found = 0;
    size_t len = 0;
    while (node) {
        if (strcmp(node->key, key) == 0) {
            len += snprintf(buffer + len, buffer_size - len, "Value for key %s: %s\n", key, node->value);
            found = 1;
        }
        node = node->next;
    }
    if (!found) {
        snprintf(buffer, buffer_size, "Key %s not found\n", key);
    }
    pthread_rwlock_unlock(&table->rwlock);
}

void delete_table(HashTable *table, char *key) {
    unsigned long index = hash(key, table->size);
    pthread_rwlock_wrlock(&table->rwlock);
    HashNode *node = table->buckets[index];
    HashNode *prev = NULL;

    while(node) {
        if (strcmp(node->key, key)==0) {
            if (prev) {
                prev->next = node->next;
            }
            else {
                table->buckets[index]= node->next;
            }
            free(node->key);
            free(node->value);
            free(node);
            pthread_rwlock_unlock(&table->rwlock);
            return;
        }
        prev = node;
        node = node->next;
    }
    pthread_rwlock_unlock(&table->rwlock);
}

void print_table(HashTable *table, char *buffer, size_t buffer_size) {
    pthread_rwlock_rdlock(&table->rwlock);
    size_t len = 0;
    for (size_t i = 0; i < table->size; i++) {
        HashNode *node = table->buckets[i];
        if (node) {
            len += snprintf(buffer + len, buffer_size - len, "bucket[%zu]: ", i);
            while (node) {
                len += snprintf(buffer + len, buffer_size - len, "%s=%s ", node->key, node->value);
                node = node->next;
            }
            len += snprintf(buffer + len, buffer_size - len, "NULL\n");
        }
    }
    pthread_rwlock_unlock(&table->rwlock);
}

void destroy_table(HashTable *table) {
    for (size_t i = 0; i < table->size; i++) {
        HashNode *node = table->buckets[i];
        while (node) {
            HashNode *temp = node;
            node = node->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(table->buckets);
    pthread_rwlock_destroy(&table->rwlock);
    free(table);
}
