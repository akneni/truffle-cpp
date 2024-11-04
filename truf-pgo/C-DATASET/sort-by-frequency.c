#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 500000000  // Number of elements in the array (tunable)
#define RANGE 10000 // Range of random numbers (0 to RANGE-1)

// Structure to hold the key and its frequency
typedef struct {
    int key;
    int count;
} KeyCount;

// Comparator function for qsort
int cmp(const void *a, const void *b) {
    KeyCount *kc1 = (KeyCount *)a;
    KeyCount *kc2 = (KeyCount *)b;
    if (kc1->count != kc2->count) {
        return kc2->count - kc1->count; // Descending order of counts
    } else {
        return kc1->key - kc2->key;     // Ascending order of keys
    }
}

int main() {
    // Heap-allocate arrays to prevent stack overflow
    int *arr = (int *)malloc(N * sizeof(int));
    int *sorted_arr = (int *)malloc(N * sizeof(int));
    int *freq = (int *)calloc(RANGE, sizeof(int));
    KeyCount *counts = (KeyCount *)malloc(RANGE * sizeof(KeyCount));

    if (arr == NULL || sorted_arr == NULL || freq == NULL || counts == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Set random seed to 42 for reproducibility
    srand(42);

    // Generate random input data
    for (int i = 0; i < N; i++) {
        arr[i] = rand() % RANGE;
    }

    // Start benchmarking
    clock_t start_time = clock();

    // Count frequencies of each element
    for (int i = 0; i < N; i++) {
        freq[arr[i]]++;
    }

    // Build an array of KeyCount structures
    int unique_count = 0;
    for (int i = 0; i < RANGE; i++) {
        if (freq[i] > 0) {
            counts[unique_count].key = i;
            counts[unique_count].count = freq[i];
            unique_count++;
        }
    }

    // Sort the counts array using the custom comparator
    qsort(counts, unique_count, sizeof(KeyCount), cmp);

    // Reconstruct the sorted array based on frequencies
    int idx = 0;
    for (int i = 0; i < unique_count; i++) {
        for (int j = 0; j < counts[i].count; j++) {
            sorted_arr[idx++] = counts[i].key;
        }
    }

    // End benchmarking
    clock_t end_time = clock();
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Uncomment the following lines to print the sorted array (optional)
    /*
    for (int i = 0; i < N; i++) {
        printf("%d ", sorted_arr[i]);
    }
    printf("\n");
    */

    // Print the total time elapsed
    printf("Time Elapsed: %f\n", total_time);

    // Free allocated memory
    free(arr);
    free(sorted_arr);
    free(freq);
    free(counts);

    return 0;
}
