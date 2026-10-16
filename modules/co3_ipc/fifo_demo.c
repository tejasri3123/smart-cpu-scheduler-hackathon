#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define FIFO_PATH "/tmp/smart_scheduler_fifo"

int main(void)
{
    if (mkfifo(FIFO_PATH, 0600) == -1 && errno != EEXIST) {
        perror("mkfifo");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        unlink(FIFO_PATH);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        int fd = open(FIFO_PATH, O_RDONLY);

        if (fd == -1) {
            perror("child open");
            _exit(EXIT_FAILURE);
        }

        char buffer[128];
        ssize_t n = read(fd, buffer, sizeof(buffer) - 1);

        if (n < 0) {
            perror("FIFO read");
            close(fd);
            _exit(EXIT_FAILURE);
        }

        buffer[n] = '\0';
        printf("Child received scheduler message: %s\n", buffer);

        close(fd);
        _exit(EXIT_SUCCESS);
    }

    int fd = open(FIFO_PATH, O_WRONLY);

    if (fd == -1) {
        perror("parent open");
        waitpid(pid, NULL, 0);
        unlink(FIFO_PATH);
        return EXIT_FAILURE;
    }

    const char *message = "Process P2 is ready for scheduling";
    size_t length = strlen(message);
    size_t written = 0;

    while (written < length) {
        ssize_t n = write(fd, message + written, length - written);

        if (n < 0) {
            if (errno == EINTR)
                continue;

            perror("FIFO write");
            close(fd);
            waitpid(pid, NULL, 0);
            unlink(FIFO_PATH);
            return EXIT_FAILURE;
        }

        written += (size_t)n;
    }

    close(fd);
    waitpid(pid, NULL, 0);
    unlink(FIFO_PATH);

    printf("FIFO communication completed.\n");
    return EXIT_SUCCESS;
}
