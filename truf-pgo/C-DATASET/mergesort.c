#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_SIZE 15000000  // Number of elements to sort

// Function to merge two subarrays of arr[]
void merge(int arr[], int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    // Create temporary arrays
    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    // Copy data to temporary arrays L[] and R[]
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    // Merge the temporary arrays back into arr[l..r]
    i = 0;   // Initial index of first subarray
    j = 0;   // Initial index of second subarray
    k = l;   // Initial index of merged subarray

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy any remaining elements of L[], if there are any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy any remaining elements of R[], if there are any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    // Free temporary arrays
    free(L);
    free(R);
}

// Main function implementing Merge Sort
void mergeSort(int arr[], int l, int r) {
    if (l < r) {
        // Find the middle point to divide the array into two halves
        int m = l + (r - l) / 2;

        // Recursively sort the first and second halves
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);

        // Merge the sorted halves
        merge(arr, l, m, r);
    }
}

int main() {
    int *arr = (int *)malloc(NUM_SIZE * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Set seed to 42 for reproducibility
    srand(42);

    // Generate an array of random integers
    for (int i = 0; i < NUM_SIZE; i++) {
        arr[i] = rand();
    }

    // Benchmark the Merge Sort algorithm
    clock_t start_sort = clock();
    mergeSort(arr, 0, NUM_SIZE - 1);
    clock_t end_sort = clock();
    double total_time = (double)(end_sort - start_sort) / CLOCKS_PER_SEC;
    printf("Time taken to sort %d elements: %f seconds\n", NUM_SIZE, total_time);

    // Optional: Verify that the array is sorted (uncomment to use)
    /*
    for (int i = 0; i < NUM_SIZE - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            printf("Array is not sorted at index %d\n", i);
            break;
        }
    }
    */

    // Free allocated memory
    free(arr);

    // Final output as specified
    printf("Time Elapsed: %f\n", total_time);
    return 0;
}
