#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
    int pid;
    int arrival;
    int burst;
    int priority;
} SchedulerTask;

int main(void)
{
    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t child = fork();

    if (child < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return EXIT_FAILURE;
    }

    if (child == 0) {
        close(fd[1]);

        SchedulerTask task;
        size_t received = 0;

        while (received < sizeof(task)) {
            ssize_t n = read(fd[0], (char *)&task + received,
                             sizeof(task) - received);

            if (n < 0) {
                perror("read");
                close(fd[0]);
                _exit(EXIT_FAILURE);
            }

            if (n == 0) {
                fprintf(stderr, "Pipe closed before task was received.\n");
                close(fd[0]);
                _exit(EXIT_FAILURE);
            }

            received += (size_t)n;
        }

        close(fd[0]);

        printf("CO3 scheduler worker received task:\n");
        printf("PID: P%d | Arrival: %d | Burst: %d | Priority: %d\n",
               task.pid, task.arrival, task.burst, task.priority);

        _exit(EXIT_SUCCESS);
    }

    close(fd[0]);

    SchedulerTask task = {
        .pid = 1,
        .arrival = 0,
        .burst = 5,
        .priority = 2
    };

    const char *description = "Sending scheduler task to child through pipe";
    printf("%s\n", description);

    size_t sent = 0;

    while (sent < sizeof(task)) {
        ssize_t n = write(fd[1], (const char *)&task + sent,
                          sizeof(task) - sent);

        if (n < 0) {
            perror("write");
            close(fd[1]);
            waitpid(child, NULL, 0);
            return EXIT_FAILURE;
        }

        sent += (size_t)n;
    }

    close(fd[1]);

    int status;
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "Scheduler worker failed.\n");
        return EXIT_FAILURE;
    }

    printf("Scheduler task communication completed.\n");
    return EXIT_SUCCESS;
}
