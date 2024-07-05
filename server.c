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
#define RESPONSE_SHM_NAME "/response_shm"
#define RESPONSE_SHM_SIZE 1024
#define THREAD_POOL_SIZE 4

void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    int head; // the point producer insert the new product
    int tail; // the point consumer find the next product
    int size;
    char buffer[SHM_SIZE - 3 * sizeof(int)];
    pthread_mutex_t mutex; 
} CircularBuffer;

typedef struct {
    int response_ready; // 
    char response[RESPONSE_SHM_SIZE - sizeof(int)];
} SharedResponse;

typedef struct {
    HashTable *table;
    CircularBuffer *cbuf;
    SharedResponse *shared_response;
} ThreadData;

void *handle_client(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    CircularBuffer *cbuf = data->cbuf;
    SharedResponse *shared_response = data->shared_response;

    while (1) {
        pthread_mutex_lock(&cbuf->mutex);

        if (cbuf->head != cbuf->tail) { // if the buffer is not empty and if there are new commands written by the clients
            char command[256];
            int len = strlen(&cbuf->buffer[cbuf->head]) + 1;
            strncpy(command, &cbuf->buffer[cbuf->head], len);
            cbuf->head = (cbuf->head + len) % cbuf->size;

            pthread_mutex_unlock(&cbuf->mutex);

            char key[256], value[256];
            if (sscanf(command, "insert %s %s", key, value) == 2) {
                insert(data->table, key, value);
                snprintf(shared_response->response, sizeof(shared_response->response), "Inserted key %s with value %s", key, value);
            } else if (sscanf(command, "get %s", key) == 1) {
                get_all(data->table, key, shared_response->response, sizeof(shared_response->response));
            } else if (sscanf(command, "delete %s", key) == 1) {
                delete_table(data->table, key);
                snprintf(shared_response->response, sizeof(shared_response->response), "Deleted key %s", key);
            } else if (strncmp(command, "print", 5) == 0) {
                print_table(data->table, shared_response->response, sizeof(shared_response->response));
            } else {
                snprintf(shared_response->response, sizeof(shared_response->response), "Unknown command: %s", command);
            }
            shared_response->response_ready = 1;
        } else {
            pthread_mutex_unlock(&cbuf->mutex);
            usleep(100000); // 100ms
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <size>\n", argv[0]);
        exit(1);
    }

    size_t size = atoi(argv[1]);
    HashTable *table = create_table(size);

    // 0666 permission: read and write for owner, group, and others
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        error("shm_open");
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        error("ftruncate");
    }

    // the memory created by mmap is sufficient to store the CircularBuffer structure
    CircularBuffer *cbuf = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (cbuf == MAP_FAILED) {
        error("mmap");
    }

    // Initialize circular buffer
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->size = SHM_SIZE - 3 * sizeof(int);
    pthread_mutex_init(&cbuf->mutex, NULL); // mutex initialization

    int response_shm_fd = shm_open(RESPONSE_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (response_shm_fd == -1) {
        error("shm_open response");
    }

    if (ftruncate(response_shm_fd, RESPONSE_SHM_SIZE) == -1) {
        error("ftruncate response");
    }

    SharedResponse *shared_response = mmap(0, RESPONSE_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, response_shm_fd, 0);
    if (shared_response == MAP_FAILED) {
        error("mmap response");
    }

    shared_response->response_ready = 0; // 

    pthread_t threads[THREAD_POOL_SIZE];
    ThreadData data = {table, cbuf, shared_response};

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&threads[i], NULL, handle_client, (void *)&data);
    }

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(threads[i], NULL);
    }

    munmap(cbuf, SHM_SIZE);
    shm_unlink(SHM_NAME);
    close(shm_fd);

    munmap(shared_response, RESPONSE_SHM_SIZE);
    shm_unlink(RESPONSE_SHM_NAME);
    close(response_shm_fd);

    destroy_table(table);
    return 0;
}
