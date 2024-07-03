#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdlib.h>
#include <pthread.h>

typedef struct HashNode {
    char *key;
    char *value;
    struct HashNode *next;
    pthread_mutex_t lock;
} HashNode;

typedef struct HashTable {
    size_t size;
    HashNode **buckets;
    pthread_rwlock_t *bucket_locks; // Array di RW locks per i buckets
} HashTable;


HashTable *create_table(size_t size);
void destroy_table(HashTable *table);
void insert(HashTable *table, char *key, char *value);
void get_all(HashTable *table, char *key, char *buffer, size_t buffer_size);
void delete_table(HashTable *table, char *key);
void print_table(HashTable *table, char *buffer, size_t buffer_size);

#endif
