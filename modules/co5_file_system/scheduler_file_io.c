#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define MAX_PROCESSES 50
#define AGING_INTERVAL 3
#define LOG_FILE "results/scheduler_results.txt"

typedef struct {
    int pid;
    int arrival;
    int burst;
    int remaining;
    int priority;
    int first_start;
    int completion;
} Process;

static int write_all(int fd, const char *buffer, size_t length)
{
    size_t written = 0;

    while (written < length) {
        ssize_t result = write(fd, buffer + written, length - written);

        if (result < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (result == 0)
            return -1;

        written += (size_t)result;
    }

    return 0;
}

static int read_all(int fd, char *buffer, size_t capacity)
{
    size_t used = 0;

    while (used + 1 < capacity) {
        ssize_t result = read(fd, buffer + used, capacity - used - 1);

        if (result < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (result == 0)
            break;

        used += (size_t)result;
    }

    buffer[used] = '\0';
    return (int)used;
}

static int effective_priority(const Process *p, int time)
{
    int executed = p->burst - p->remaining;
    int waiting = time - p->arrival - executed;
    int priority = p->priority - waiting / AGING_INTERVAL;

    return priority < 0 ? 0 : priority;
}

int main(void)
{
    Process processes[MAX_PROCESSES];
    int n;
    int completed = 0;
    int time = 0;
    double total_waiting = 0.0;
    double total_turnaround = 0.0;
    char output[16384];
    size_t used = 0;

    printf("=== CO5: File System and Scheduler Results ===\n");
    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);

    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_PROCESSES) {
        fprintf(stderr, "Invalid number of processes.\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        processes[i].first_start = -1;
        processes[i].completion = 0;

        printf("P%d: enter arrival burst priority (0-10): ", i + 1);

        if (scanf("%d %d %d",
                  &processes[i].arrival,
                  &processes[i].burst,
                  &processes[i].priority) != 3 ||
            processes[i].arrival < 0 ||
            processes[i].burst < 1 ||
            processes[i].priority < 0 ||
            processes[i].priority > 10) {
            fprintf(stderr, "Invalid process values.\n");
            return EXIT_FAILURE;
        }

        processes[i].remaining = processes[i].burst;
    }

    used += (size_t)snprintf(output + used, sizeof(output) - used,
        "Smart CPU Scheduler - CO5 File I/O Results\n"
        "Aging interval: %d time units\n\n", AGING_INTERVAL);

    used += (size_t)snprintf(output + used, sizeof(output) - used,
        "PID\tArrival\tBurst\tPriority\tCompletion\tTurnaround\tWaiting\n");

    while (completed < n) {
        int chosen = -1;
        int best_priority = 100000;

        for (int i = 0; i < n; i++) {
            if (processes[i].remaining > 0 &&
                processes[i].arrival <= time) {
                int current = effective_priority(&processes[i], time);

                if (chosen == -1 || current < best_priority ||
                    (current == best_priority &&
                     processes[i].arrival < processes[chosen].arrival)) {
                    chosen = i;
                    best_priority = current;
                }
            }
        }

        if (chosen == -1) {
            time++;
            continue;
        }

        if (processes[chosen].first_start == -1)
            processes[chosen].first_start = time;

        processes[chosen].remaining--;
        time++;

        if (processes[chosen].remaining == 0) {
            processes[chosen].completion = time;
            completed++;
        }
    }

    for (int i = 0; i < n; i++) {
        int turnaround = processes[i].completion - processes[i].arrival;
        int waiting = turnaround - processes[i].burst;

        total_turnaround += turnaround;
        total_waiting += waiting;

        int count = snprintf(output + used, sizeof(output) - used,
            "P%d\t%d\t%d\t%d\t\t%d\t\t%d\t\t%d\n",
            processes[i].pid, processes[i].arrival, processes[i].burst,
            processes[i].priority, processes[i].completion,
            turnaround, waiting);

        if (count < 0 || (size_t)count >= sizeof(output) - used) {
            fprintf(stderr, "Results buffer is too small.\n");
            return EXIT_FAILURE;
        }

        used += (size_t)count;
    }

    {
        int count = snprintf(output + used, sizeof(output) - used,
            "\nAverage turnaround time: %.2f\n"
            "Average waiting time: %.2f\n",
            total_turnaround / n, total_waiting / n);

        if (count < 0 || (size_t)count >= sizeof(output) - used) {
            fprintf(stderr, "Results buffer is too small.\n");
            return EXIT_FAILURE;
        }

        used += (size_t)count;
    }

    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        perror("open results file");
        return EXIT_FAILURE;
    }

    if (write_all(fd, output, used) < 0) {
        perror("write results file");
        close(fd);
        return EXIT_FAILURE;
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) < 0) {
        perror("close");
        return EXIT_FAILURE;
    }

    fd = open(LOG_FILE, O_RDONLY);

    if (fd < 0) {
        perror("reopen results file");
        return EXIT_FAILURE;
    }

    char readback[16384];
    int bytes = read_all(fd, readback, sizeof(readback));

    if (bytes < 0) {
        perror("read results file");
        close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) < 0) {
        perror("close");
        return EXIT_FAILURE;
    }

    printf("\nResults saved to %s\n\n", LOG_FILE);
    printf("%s", readback);

    return EXIT_SUCCESS;
}
