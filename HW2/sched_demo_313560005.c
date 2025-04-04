#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>

#define SCHE_FIFO 1
#define SCHE_NORMAL 0

typedef struct {
    pthread_t thread_id;
    int sched_policy;
    float time_wait;
} thread_info_t;

pthread_barrier_t barrier;

void *thread_func(void *args) {

    /* 1. Wait until all threads are ready */
    pthread_barrier_wait(&barrier);

    thread_info_t *thread_info = (thread_info_t *)args;
    int wait_time_ms = (int)(thread_info->time_wait * 1000);

    /* 2. Do the task */
    int count = 0;
    while (count < 3) {
        printf("Thread %d is starting\n", (int)thread_info->thread_id);

        // Busy-wait for the specified time
        struct timespec begin_time, current_time;
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &begin_time);
        int begin_ms = begin_time.tv_sec * 1000 + begin_time.tv_nsec / 1000000;

        int elapsed_ms = 0;
        do {
            clock_gettime(CLOCK_THREAD_CPUTIME_ID, &current_time);
            int current_ms = current_time.tv_sec * 1000 + current_time.tv_nsec / 1000000;
            elapsed_ms = current_ms - begin_ms;
        } while (elapsed_ms < wait_time_ms);

        sched_yield(); // Yield the CPU to other threads
        count++;
    }

    /* 3. Exit the function  */
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int opt;
    int num_threads = atoi(argv[2]);
    float time_wait = 0;
    char *policies[num_threads];
    int priorities[num_threads];

    // initialize barrier threads + main thread = num_threads + 1
    pthread_barrier_init(&barrier, NULL, num_threads + 1);

    /* 1. Parse program arguments */
    while ((opt = getopt(argc, argv, "n:t:s:p:")) != -1) {
        switch (opt) {
        case 'n':
            if (atoi(optarg) != num_threads) {
                exit(EXIT_FAILURE);
            }
            break;
        case 't':
            time_wait = atof(optarg);
            break;
        case 's':
            char *s_string = optarg;
            char *s_token = strtok(s_string, ",");
            for (int i = 0; i < num_threads; i++) {
                policies[i] = s_token;
                s_token = strtok(NULL, ",");
            }
            break;
        case 'p':
            char *p_string = optarg;
            char *p_token = strtok(p_string, ",");
            for (int i = 0; i < num_threads; i++) {
                priorities[i] = atoi(p_token);
                p_token = strtok(NULL, ",");
            }
            break;
        default:
            fprintf(stderr, "input usage error\n");
            exit(EXIT_FAILURE);
        }
    }

    /* 2. Create <num_threads> worker thread_info */
    pthread_attr_t attr[num_threads];       // thread attributes
    struct sched_param param[num_threads];  // thread parameters
    pthread_t thread[num_threads];          // thread identifiers
    thread_info_t thread_info[num_threads]; // thread information

    /* 3. Set CPU affinity */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);        
    CPU_SET(0, &cpuset); 
    sched_setaffinity(0, sizeof(cpuset), &cpuset);

    /* 4. Set the attributes to each thread */

    for (int i = 0; i < num_threads; i++) {
        thread_info[i].thread_id = i;
        thread_info[i].time_wait = time_wait;

        if (strcmp(policies[i], "FIFO") == 0) {
            thread_info[i].sched_policy = SCHE_FIFO;
            pthread_attr_init(&attr[i]);
            pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);

            // set the scheduling policy - FIFO
            if (pthread_attr_setschedpolicy(&attr[i], SCHE_FIFO) != 0) {
                perror("Error: pthread_attr_setschedpolicy");
                exit(EXIT_FAILURE);
            }

            param[i].sched_priority = priorities[i]; // set priority

            // set the scheduling paramters
            if (pthread_attr_setschedparam(&attr[i], &param[i]) != 0) {
                perror("Error: pthread_attr_setschedparam");
                exit(EXIT_FAILURE);
            }

            // create the thread
            if (pthread_create(&thread[i], &attr[i], thread_func,
                               &thread_info[i]) != 0) {
                perror("Error: pthread_create (FIFO)");
                exit(EXIT_FAILURE);
            }

        } else if (strcmp(policies[i], "NORMAL") == 0) {
            thread_info[i].sched_policy = SCHE_NORMAL;

            if (pthread_create(&thread[i], NULL, thread_func, &thread_info[i]) != 0) {
                perror("Error: pthread_create (NORMAL)");
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "Error: Unexpected scheduling policy input.\n");
            exit(EXIT_FAILURE);
        }
    }

    /* 5. Start all threads  at once */
    pthread_barrier_wait(&barrier);

    /* 6. Wait for all threads  to finish  */
    for (int i = 0; i < num_threads; i++) {
        pthread_join(thread[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    return 0;
}
