#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#define SOCKET_PATH "/tmp/smart_scheduler_socket"

int main(void)
{
    int sockets[2];

    /*
     * socketpair creates two connected Unix domain sockets.
     * This is a local IPC demonstration between parent and child.
     */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
        perror("socketpair");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        close(sockets[0]);
        close(sockets[1]);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        close(sockets[0]);

        char buffer[128];
        ssize_t n = read(sockets[1], buffer, sizeof(buffer) - 1);

        if (n < 0) {
            perror("socket read");
            close(sockets[1]);
            _exit(EXIT_FAILURE);
        }

        buffer[n] = '\0';
        printf("Child received: %s\n", buffer);

        const char *reply = "Scheduler acknowledgement received";
        if (write(sockets[1], reply, strlen(reply)) < 0) {
            perror("socket reply");
            close(sockets[1]);
            _exit(EXIT_FAILURE);
        }

        close(sockets[1]);
        _exit(EXIT_SUCCESS);
    }

    close(sockets[1]);

    const char *message = "Scheduling task P3";
    if (write(sockets[0], message, strlen(message)) < 0) {
        perror("socket write");
        close(sockets[0]);
        waitpid(pid, NULL, 0);
        return EXIT_FAILURE;
    }

    char reply[128];
    ssize_t n = read(sockets[0], reply, sizeof(reply) - 1);

    if (n < 0) {
        perror("socket reply read");
        close(sockets[0]);
        waitpid(pid, NULL, 0);
        return EXIT_FAILURE;
    }

    reply[n] = '\0';
    printf("Parent received: %s\n", reply);

    close(sockets[0]);

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "Child process failed.\n");
        return EXIT_FAILURE;
    }

    printf("Unix domain socket communication completed.\n");
    return EXIT_SUCCESS;
}
