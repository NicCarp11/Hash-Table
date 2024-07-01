#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "hashTable.h"

HashTable *table;

void* perform_operations(void *arg) {
    int thread_id = *(int*)arg;
    int num_operations = ((int*)arg)[1];
    for (int i = 0; i < num_operations; i++) {
        char key[20];
        char value[20];
        sprintf(key, "key%d_%d", thread_id, i);
        sprintf(value, "value%d_%d", thread_id, i);
        insert(table, key, value);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <number_of_threads> <number_of_operations_per_thread>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int num_operations = atoi(argv[2]);

    table = create_table(1000);

    pthread_t threads[num_threads];
    int thread_args[num_threads][2];

    clock_t start = clock();
    
    for (int i = 0; i < num_threads; i++) {
        thread_args[i][0] = i;
        thread_args[i][1] = num_operations;
        pthread_create(&threads[i], NULL, perform_operations, thread_args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t end = clock();

    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time taken for %d operations with %d threads: %f seconds\n", num_operations * num_threads, num_threads, time_spent);

    return 0;
}
