#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

typedef struct {
    int process_id;
    int burst_time;
    int priority;
    char status[64];
} SchedulerSharedData;

int main(void)
{
    SchedulerSharedData *shared = mmap(
        NULL,
        sizeof(*shared),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );

    if (shared == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    shared->process_id = 2;
    shared->burst_time = 5;
    shared->priority = 3;
    snprintf(shared->status, sizeof(shared->status), "Waiting");

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        munmap(shared, sizeof(*shared));
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        printf("Child: received shared scheduler data:\n");
        printf("  Process ID: %d\n", shared->process_id);
        printf("  Burst time: %d\n", shared->burst_time);
        printf("  Priority: %d\n", shared->priority);

        shared->priority = 1;
        snprintf(shared->status, sizeof(shared->status),
                 "Ready for scheduling");

        _exit(EXIT_SUCCESS);
    }

    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        munmap(shared, sizeof(*shared));
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "Child process failed.\n");
        munmap(shared, sizeof(*shared));
        return EXIT_FAILURE;
    }

    printf("\nParent: values after child updated shared memory:\n");
    printf("  Updated priority: %d\n", shared->priority);
    printf("  Updated status: %s\n", shared->status);

    if (munmap(shared, sizeof(*shared)) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }

    printf("Shared-memory demonstration completed.\n");
    return EXIT_SUCCESS;
}
