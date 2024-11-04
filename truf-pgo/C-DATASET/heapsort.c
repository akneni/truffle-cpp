#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_SIZE 10000000  // Number of elements to sort

// Function to swap two elements
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Heapify a subtree rooted with node i
void heapify(int arr[], int n, int i) {
    int largest = i;       // Initialize largest as root
    int left = 2 * i + 1;  // Left child
    int right = 2 * i + 2; // Right child

    // If left child is larger than root
    if (left < n && arr[left] > arr[largest])
        largest = left;

    // If right child is larger than largest so far
    if (right < n && arr[right] > arr[largest])
        largest = right;

    // If largest is not root
    if (largest != i) {
        swap(&arr[i], &arr[largest]);

        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

// Main function to perform heap sort
void heapSort(int arr[], int n) {
    int i;

    // Build a maxheap
    for (i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    // Extract elements from heap one by one
    for (i = n - 1; i >= 0; i--) {
        // Move current root to end
        swap(&arr[0], &arr[i]);

        // Call max heapify on the reduced heap
        heapify(arr, i, 0);
    }
}

int main() {
    int *arr;
    int i;

    // Allocate memory dynamically
    arr = (int *)malloc(NUM_SIZE * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // Set seed for reproducibility
    srand(42);

    // Generate array with random numbers
    for (i = 0; i < NUM_SIZE; i++) {
        arr[i] = rand();
    }

    // Benchmark heap sort
    clock_t start_sort = clock();
    heapSort(arr, NUM_SIZE);
    clock_t end_sort = clock();

    double time_sort = (double)(end_sort - start_sort) / CLOCKS_PER_SEC;
    printf("Time taken to sort %d elements: %f seconds\n", NUM_SIZE, time_sort);

    // Calculate total time
    double total_time = time_sort;
    printf("Time Elapsed: %f\n", total_time);

    // Free allocated memory
    free(arr);

    return 0;
}
