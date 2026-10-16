#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 50
#define AGING_INTERVAL 3

typedef struct {
    int pid;
    int arrival;
    int burst;
    int remaining;
    int priority;
    int first_start;
    int completion;
} Process;

static int effective_priority(const Process *p, int time)
{
    int executed = p->burst - p->remaining;
    int waited = time - p->arrival - executed;

    if (waited < 0)
        waited = 0;

    int priority = p->priority - waited / AGING_INTERVAL;

    return priority < 0 ? 0 : priority;
}

int main(void)
{
    Process p[MAX_PROCESSES];
    int n;
    int completed = 0;
int previous_effective_priority[MAX_PROCESSES];

for (int i = 0; i < MAX_PROCESSES; i++) {
    previous_effective_priority[i] = -1;
}
    int time = 0;
    double total_wait = 0.0;
    double total_turnaround = 0.0;
    double total_response = 0.0;

    printf("SMART CPU SCHEDULER WITH DYNAMIC PRIORITY AGING\n");
    printf("Lower priority number means higher priority.\n");
    printf("Aging interval: %d time units\n", AGING_INTERVAL);
    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);

    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_PROCESSES) {
        fprintf(stderr, "Invalid process count.\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;
        p[i].first_start = -1;
        p[i].completion = 0;

        printf("Process P%d — arrival burst priority: ", p[i].pid);

        if (scanf("%d %d %d", &p[i].arrival,
                  &p[i].burst, &p[i].priority) != 3 ||
            p[i].arrival < 0 || p[i].burst <= 0 ||
            p[i].priority < 0 || p[i].priority > 10) {
            fprintf(stderr,
                    "Invalid input. Arrival >= 0, burst > 0, priority 0-10.\n");
            return EXIT_FAILURE;
        }

        p[i].remaining = p[i].burst;
    }

    printf("\nGANTT CHART\n");

    while (completed < n) {
        int selected = -1;
        int best_priority = 1000000;

        for (int i = 0; i < n; i++) {
            if (p[i].remaining <= 0 || p[i].arrival > time)
                continue;

            int ep = effective_priority(&p[i], time);
if (previous_effective_priority[i] != -1 &&
    ep < previous_effective_priority[i]) {
    printf("\n[AGING] Time %d: P%d priority improved from %d to %d\n",
           time, p[i].pid, previous_effective_priority[i], ep);
}

previous_effective_priority[i] = ep;
            if (selected == -1 || ep < best_priority ||
                (ep == best_priority && p[i].arrival < p[selected].arrival) ||
                (ep == best_priority && p[i].arrival == p[selected].arrival &&
                 p[i].pid < p[selected].pid)) {
                selected = i;
                best_priority = ep;
            }
        }

        if (selected == -1) {
            printf("| idle(%d) ", time);
            time++;
            continue;
        }

        if (p[selected].first_start == -1)
    p[selected].first_start = time;

printf("| P%d(Pri=%d,%d-%d) ",
       p[selected].pid,
       effective_priority(&p[selected], time),
       time,
       time + 1);

        p[selected].remaining--;
        time++;

        if (p[selected].remaining == 0) {
            p[selected].completion = time;
            completed++;
        }
    }

    printf("|\n\n");
    printf("PID\tAT\tBT\tPriority\tCT\tTAT\tWT\tRT\n");

    for (int i = 0; i < n; i++) {
        int turnaround = p[i].completion - p[i].arrival;
        int waiting = turnaround - p[i].burst;
        int response = p[i].first_start - p[i].arrival;

        total_turnaround += turnaround;
        total_wait += waiting;
        total_response += response;

        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\t%d\t%d\n",
               p[i].pid, p[i].arrival, p[i].burst, p[i].priority,
               p[i].completion, turnaround, waiting, response);
    }

    printf("\nAverage waiting time: %.2f\n", total_wait / n);
    printf("Average turnaround time: %.2f\n", total_turnaround / n);
    printf("Average response time: %.2f\n", total_response / n);

    return EXIT_SUCCESS;
}
