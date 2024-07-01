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

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        error("shm_open");
    }

    char *shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        error("mmap");
    }

    char *command_ptr = shm_ptr + sizeof(int);  
    int *flag_ptr = (int *)shm_ptr;

    char command[256];
    while (1) {
        printf("Enter command (insert <key> <value>, get <key>, delete <key>, print, exit): ");
        fgets(command, sizeof(command), stdin);

        if (strncmp(command, "exit", 4) == 0) {
            break;
        }

        strncpy(command_ptr, command, SHM_SIZE - sizeof(int));

        *flag_ptr = 1;

        while (*flag_ptr != 0) {
            usleep(100000); 
        }

        printf("Server response: %s", command_ptr);
    }

    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    return 0;
}
