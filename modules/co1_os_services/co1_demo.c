#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

int main(void)
{
    char buffer[100];
    char message[] = "CO1: OS Services and System Calls\n";

    write(STDOUT_FILENO, message, strlen(message));

    printf("Process ID: %ld\n", (long)getpid());
    printf("Enter a short command or message: ");
    fflush(stdout);

    ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

    if (n < 0) {
        perror("read");
        return 1;
    }

    if (n == 0) {
        printf("No input received.\n");
        return 0;
    }

    buffer[n] = '\0';
    printf("You entered: %s", buffer);

    return 0;
}
