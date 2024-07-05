#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 1024
#define RESPONSE_SHM_NAME "/response_shm"
#define RESPONSE_SHM_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    int head;
    int tail;
    int size;
    char buffer[SHM_SIZE - 3 * sizeof(int)];
} CircularBuffer;

typedef struct {
    int response_ready; 
    char response[RESPONSE_SHM_SIZE - sizeof(int)];
} SharedResponse;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        error("shm_open");
    }

    CircularBuffer *cbuf = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (cbuf == MAP_FAILED) {
        error("mmap");
    }

    int response_shm_fd = shm_open(RESPONSE_SHM_NAME, O_RDWR, 0666);
    if (response_shm_fd == -1) {
        error("shm_open response");
    }

    SharedResponse *shared_response = mmap(0, RESPONSE_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, response_shm_fd, 0);
    if (shared_response == MAP_FAILED) {
        error("mmap response");
    }

    char command[256];
    while (1) {
        printf("Enter command (insert <key> <value>, get <key>, delete <key>, print, exit): ");
        fgets(command, sizeof(command), stdin);

        if (strncmp(command, "exit", 4) == 0) {
            break;
        }

        int len = strlen(command) + 1;
        if ((cbuf->tail + len) % cbuf->size != cbuf->head) { // check if there is enough space in the buffer
            strncpy(&cbuf->buffer[cbuf->tail], command, len);
            cbuf->tail = (cbuf->tail + len) % cbuf->size;
        } else {
            printf("Buffer is full, please try again later.\n");
        }

        usleep(100000);

        while (!shared_response->response_ready) {
            usleep(10000); // waiting for response
        }
        printf("Server response: %s\n", shared_response->response);
        shared_response->response_ready = 0; // Reset del flag
    }

    munmap(cbuf, SHM_SIZE);
    close(shm_fd);

    munmap(shared_response, RESPONSE_SHM_SIZE);
    close(response_shm_fd);

    return 0;
}
