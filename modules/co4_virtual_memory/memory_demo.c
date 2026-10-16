#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <string.h>

#define REGION_SIZE (1024 * 1024)

int main(void)
{
    long page_size = sysconf(_SC_PAGESIZE);

    if (page_size <= 0) {
        fprintf(stderr, "Could not determine page size.\n");
        return EXIT_FAILURE;
    }

    printf("CO4: Virtual Memory Demonstration\n");
    printf("Process ID: %ld\n", (long)getpid());
    printf("Page size: %ld bytes\n", page_size);

    char *memory = mmap(NULL, REGION_SIZE,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (memory == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    struct rusage before, after;

    if (getrusage(RUSAGE_SELF, &before) == -1) {
        perror("getrusage");
        munmap(memory, REGION_SIZE);
        return EXIT_FAILURE;
    }

    printf("Writing to one memory location per page...\n");

    for (size_t offset = 0; offset < REGION_SIZE;
         offset += (size_t)page_size) {
        memory[offset] = 'A';
    }

    if (getrusage(RUSAGE_SELF, &after) == -1) {
        perror("getrusage");
        munmap(memory, REGION_SIZE);
        return EXIT_FAILURE;
    }

    printf("Minor page faults before: %ld\n", before.ru_minflt);
    printf("Minor page faults after:  %ld\n", after.ru_minflt);
    printf("Major page faults before: %ld\n", before.ru_majflt);
    printf("Major page faults after:  %ld\n", after.ru_majflt);

    printf("\nCreating a child process to demonstrate copy-on-write...\n");

    pid_t child = fork();

    if (child < 0) {
        perror("fork");
        munmap(memory, REGION_SIZE);
        return EXIT_FAILURE;
    }

    if (child == 0) {
        memory[0] = 'C';
        printf("Child changed its private mapping to '%c'.\n", memory[0]);
        printf("Child PID: %ld\n", (long)getpid());
        _exit(EXIT_SUCCESS);
    }

    int status;

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        munmap(memory, REGION_SIZE);
        return EXIT_FAILURE;
    }

    printf("Parent still sees '%c' at the first location.\n", memory[0]);

    if (munmap(memory, REGION_SIZE) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }

    printf("Memory mapping released successfully.\n");
    return EXIT_SUCCESS;
}
