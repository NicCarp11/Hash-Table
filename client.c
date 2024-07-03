#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 1024

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

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        error("shm_open");
    }

    CircularBuffer *cbuf = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (cbuf == MAP_FAILED) {
        error("mmap");
    }

    char command[256];
    while (1) {
        printf("Enter command (insert <key> <value>, get <key>, delete <key>, print, exit): ");
        fgets(command, sizeof(command), stdin);

        if (strncmp(command, "exit", 4) == 0) {
            break;
        }

        int len = strlen(command) + 1;
        if ((cbuf->tail + len) % (SHM_SIZE - 3 * sizeof(int)) != cbuf->head) { // check is buffer is full
            strncpy(&cbuf->buffer[cbuf->tail], command, len);
            cbuf->tail = (cbuf->tail + len) % (SHM_SIZE - 3 * sizeof(int));
        } else {
            printf("Buffer is full, please try again later.\n");
        }

        usleep(100000); // 100ms
        printf("Server response: %s", command);
    }

    munmap(cbuf, SHM_SIZE);
    close(shm_fd);
    return 0;
}
