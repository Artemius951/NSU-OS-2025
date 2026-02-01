#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifndef NUM_STEPS
#define NUM_STEPS 200000000
#endif

typedef struct {
    long start;
    long end;
    double partial_sum;
} thread_data_t;

void* calculate_partial_pi(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    double sum = 0.0;
    
    for (long i = data->start; i < data->end; i++) {
        sum += 1.0 / (i * 4.0 + 1.0);
        sum -= 1.0 / (i * 4.0 + 3.0);
    }
    
    data->partial_sum = sum;
    
    double* result = malloc(sizeof(double));
    if (result == NULL) {
        perror("Failed to allocate memory for result");
        pthread_exit(NULL);
    }
    
    *result = sum;
    pthread_exit((void*)result);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Number of threads must be positive\n");
        return EXIT_FAILURE;
    }
    
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    thread_data_t* thread_data = malloc(num_threads * sizeof(thread_data_t));
    
    if (threads == NULL || thread_data == NULL) {
        perror("Failed to allocate memory for threads");
        free(threads);
        free(thread_data);
        return EXIT_FAILURE;
    }
    long steps_per_thread = NUM_STEPS / num_threads;
    long remainder = NUM_STEPS % num_threads;
    long current_start = 0;
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].start = current_start;
        thread_data[i].end = current_start + steps_per_thread;
        
        if (i < remainder) {
            thread_data[i].end++;
        }
        
        current_start = thread_data[i].end;
        
        if (pthread_create(&threads[i], NULL, calculate_partial_pi, &thread_data[i]) != 0) {
            perror("Failed to create thread");
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
            }
            free(threads);
            free(thread_data);
            return EXIT_FAILURE;
        }
    }
    
    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        double* partial_result;
        
        if (pthread_join(threads[i], (void**)&partial_result) != 0) {
            perror("Failed to join thread");
        } else if (partial_result != NULL) {
            pi += *partial_result;
            free(partial_result);
        }
    }
    
    pi = pi * 4.0;
    printf("pi = %.15g\n", pi);
    
    free(threads);
    free(thread_data);
    
    return EXIT_SUCCESS;
}