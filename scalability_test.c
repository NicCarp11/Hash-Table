#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 1024
#define NUM_COMMANDS 4

typedef struct {
    int head;
    int tail;
    int size;
    char buffer[SHM_SIZE - 3 * sizeof(int)];
} CircularBuffer;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void* perform_operations(void *arg) {
    int thread_id = *(int*)arg;
    int num_operations = ((int*)arg)[1];

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        error("shm_open");
    }

    CircularBuffer *cbuf = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (cbuf == MAP_FAILED) {
        error("mmap");
    }

    char *commands[NUM_COMMANDS] = {"insert", "get", "delete", "print"};
    char command[256];
    char key[20];
    char value[20];

    srand(time(NULL) + thread_id);

    for (int i = 0; i < num_operations; i++) {
        int cmd_index = rand() % NUM_COMMANDS;
        sprintf(key, "key%d_%d", thread_id, i);
        sprintf(value, "value%d_%d", thread_id, i);

        if (cmd_index == 0) { // insert command
            snprintf(command, sizeof(command), "%s %s %s", commands[cmd_index], key, value);
        } else if (cmd_index == 1 || cmd_index == 2) { // get or delete command
            snprintf(command, sizeof(command), "%s %s", commands[cmd_index], key);
        } else { // print command
            snprintf(command, sizeof(command), "%s", commands[cmd_index]);
        }

        int len = strlen(command) + 1;
        if ((cbuf->tail + len) % cbuf->size != cbuf->head) {
            strncpy(&cbuf->buffer[cbuf->tail], command, len);
            cbuf->tail = (cbuf->tail + len) % cbuf->size;
        } else {
            printf("Buffer is full, please try again later.\n");
        }

        usleep(100000); // 100ms
    }

    munmap(cbuf, SHM_SIZE);
    close(shm_fd);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <number_of_threads> <number_of_operations_per_thread>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int num_operations = atoi(argv[2]);

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
