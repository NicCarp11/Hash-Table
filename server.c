#include "hashTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 1024
#define THREAD_POOL_SIZE 4


void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    HashTable *table;
    int shm_fd;
    char *shm_ptr;
    pthread_mutex_t *mutex;
} ThreadData;


void *handle_client (void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char *command_ptr = data->shm_ptr + sizeof(int);
    int *flag_ptr = (int *)data->shm_ptr;

    while (1) {
        pthread_mutex_lock(data->mutex);

        if (*flag_ptr == 1) {
            printf("Received command: %s\n", command_ptr);

            char key[256];
            char value[256];
            if (sscanf(command_ptr, "insert %s %s", key, value) == 2) {
                insert(data->table, key, value);
                strcpy(command_ptr, "Inserted\n");
            } else if (sscanf(command_ptr, "get %s", key) == 1) {
                get_all(data->table, key, command_ptr, SHM_SIZE);
            } else if (sscanf(command_ptr, "delete %s", key) == 1) {
                delete_table(data->table, key);
                strcpy(command_ptr, "Deleted\n");
            } else if (strncmp(command_ptr, "print", 5) == 0) {
                print_table(data->table, command_ptr, SHM_SIZE);
            } else {
                strcpy(command_ptr, "Unknown command\n");
            }

            *flag_ptr = 0;
        }
        pthread_mutex_unlock(data->mutex);
        usleep(100000); // 100ms
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <size>\n", argv[0]);
        exit(1);
    }

    size_t size = atoi(argv[1]);
    HashTable *table = create_table(size);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        error("shm_open");
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        error("ftruncate");
    }

    char *shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        error("mmap");
    }

    pthread_t threads[THREAD_POOL_SIZE];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    ThreadData data = {table, shm_fd, shm_ptr, &mutex};

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&threads[i], NULL, handle_client, (void *)&data);
    }

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(threads[i], NULL);
    }



    munmap(shm_ptr, SHM_SIZE);
    shm_unlink(SHM_NAME);
    close(shm_fd);
    destroy_table(table);
    return 0;
}
