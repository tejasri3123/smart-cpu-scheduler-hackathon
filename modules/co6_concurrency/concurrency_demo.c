#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define WORKERS 3
#define TASKS_PER_WORKER 4

static pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t task_ready = PTHREAD_COND_INITIALIZER;
static sem_t resource_slots;

static int completed_tasks = 0;
static int ready = 0;

static void *worker(void *arg)
{
    int id = *(int *)arg;

    /* Wait until the coordinator signals that work is ready. */
    pthread_mutex_lock(&ready_mutex);
    while (!ready)
        pthread_cond_wait(&task_ready, &ready_mutex);
    pthread_mutex_unlock(&ready_mutex);

    for (int i = 0; i < TASKS_PER_WORKER; i++) {
        /* Limit concurrent access to two resource slots. */
        while (sem_wait(&resource_slots) == -1) {
            if (errno != EINTR) {
                perror("sem_wait");
                return NULL;
            }
        }

        printf("Worker %d processing task %d\n", id, i + 1);

        /* Protect the shared counter against race conditions. */
        pthread_mutex_lock(&stats_mutex);
        completed_tasks++;
        pthread_mutex_unlock(&stats_mutex);

        sem_post(&resource_slots);
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[WORKERS];
    int ids[WORKERS];

    if (sem_init(&resource_slots, 0, 2) == -1) {
        perror("sem_init");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < WORKERS; i++) {
        ids[i] = i + 1;
        int rc = pthread_create(&threads[i], NULL, worker, &ids[i]);

        if (rc != 0) {
            fprintf(stderr, "pthread_create failed: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    printf("Coordinator: releasing workers using a condition variable.\n");

    pthread_mutex_lock(&ready_mutex);
    ready = 1;
    pthread_cond_broadcast(&task_ready);
    pthread_mutex_unlock(&ready_mutex);

    for (int i = 0; i < WORKERS; i++) {
        int rc = pthread_join(threads[i], NULL);

        if (rc != 0) {
            fprintf(stderr, "pthread_join failed: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    printf("\nAll workers finished.\n");
    printf("Expected tasks: %d\n", WORKERS * TASKS_PER_WORKER);
    printf("Completed tasks: %d\n", completed_tasks);

    sem_destroy(&resource_slots);
    pthread_cond_destroy(&task_ready);
    pthread_mutex_destroy(&ready_mutex);
    pthread_mutex_destroy(&stats_mutex);

    return completed_tasks == WORKERS * TASKS_PER_WORKER
        ? EXIT_SUCCESS : EXIT_FAILURE;
}
