#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

static volatile sig_atomic_t signal_received = 0;

static void handle_signal(int signo)
{
    signal_received = signo;
}

int main(void)
{
    struct sigaction action = {0};
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        printf("Child: waiting for scheduler notification...\n");

        while (signal_received == 0)
            pause();

        printf("Child: received signal %d (SIGUSR1).\n",
               (int)signal_received);
        _exit(EXIT_SUCCESS);
    }

    /* Notify the child process. */
    sleep(1);

    if (kill(pid, SIGUSR1) == -1) {
        perror("kill");
        waitpid(pid, NULL, 0);
        return EXIT_FAILURE;
    }

    printf("Parent: sent SIGUSR1 to child process.\n");

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "Child process failed.\n");
        return EXIT_FAILURE;
    }

    printf("Signal demonstration completed.\n");
    return EXIT_SUCCESS;
}
