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

void error(const char *msg) {
    perror(msg);
    exit(1);
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

    char *command_ptr = shm_ptr + sizeof(int);  
    int *flag_ptr = (int *)shm_ptr;

    while (1) {
        
        if (*flag_ptr == 1) {
            printf("Received command: %s\n", command_ptr);

            char key[256];
            char value[256];
            if (sscanf(command_ptr, "insert %s %s", key, value) == 2) {
                insert(table, key, value);
                strcpy(command_ptr, "Inserted\n");
            } else if (sscanf(command_ptr, "get %s", key) == 1) {
                get_all(table, key, command_ptr, SHM_SIZE);
            } else if (sscanf(command_ptr, "delete %s", key) == 1) {
                delete_table(table, key);
                strcpy(command_ptr, "Deleted\n");
            } else if (strncmp(command_ptr, "print", 5) == 0) {
                print_table(table, command_ptr, SHM_SIZE);
            } else {
                strcpy(command_ptr, "Unknown command\n");
            }

            *flag_ptr = 0;
        }

        usleep(100000); // 100ms
    }

    munmap(shm_ptr, SHM_SIZE);
    shm_unlink(SHM_NAME);
    close(shm_fd);
    destroy_table(table);
    return 0;
}
