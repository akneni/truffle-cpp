#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_SIZE 20000000  // Number of elements to sort

// Function to swap two elements
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Partition function used by quicksort
int partition(int arr[], int low, int high) {
    int pivot = arr[high];  // Pivot element
    int i = (low - 1);      // Index of smaller element

    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

// Quicksort function
void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        // Separately sort elements before and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// Function to print the array (optional)
void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

int main() {
    int* arr = (int*)malloc(NUM_SIZE * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed\n");
        return -1;
    }

    srand(42);

    // Generate random array
    for (int i = 0; i < NUM_SIZE; i++) {
        arr[i] = rand();
    }

    // Benchmark quicksort
    clock_t start_time = clock();
    quickSort(arr, 0, NUM_SIZE - 1);
    clock_t end_time = clock();
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Optional: Verify that the array is sorted (uncomment to use)
    /*
    for (int i = 1; i < NUM_SIZE; i++) {
        if (arr[i - 1] > arr[i]) {
            printf("Array is not sorted correctly\n");
            break;
        }
    }
    */

    // Optional: Print the sorted array (uncomment to use)
    // printArray(arr, NUM_SIZE);

    printf("Time taken to sort %d elements: %f seconds\n", NUM_SIZE, total_time);
    printf("Time Elapsed: %f\n", total_time);

    free(arr);
    return 0;
}
