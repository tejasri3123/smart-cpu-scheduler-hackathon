#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid;
    int status;

    printf("CO2: Process Control Demonstration\n");
    printf("Parent PID: %ld\n", (long)getpid());

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        printf("Child process created. PID: %ld\n", (long)getpid());
        printf("Child is executing a separate program using exec.\n");
        execlp("echo", "echo", "Child program executed successfully!", (char *)NULL);

        perror("execlp");
        _exit(127);
    }

    printf("Parent is waiting for child PID %ld\n", (long)pid);

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf("Child exited with status: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Child terminated by signal: %d\n", WTERMSIG(status));
    }

    printf("Parent process completed.\n");
    return EXIT_SUCCESS;
}
