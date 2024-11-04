#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ITEMS 100000     // Number of items
#define MAX_WEIGHT 4200   // Maximum weight of the knapsack

// Structure to represent an item
typedef struct {
    int value;
    int weight;
} Item;

// Function to solve the Knapsack Problem using dynamic programming
int knapsack(Item items[], int n, int W) {
    int i, w;
    int **K = (int **)malloc((n + 1) * sizeof(int *));
    for (i = 0; i <= n; i++)
        K[i] = (int *)malloc((W + 1) * sizeof(int));

    // Build table K[][] in bottom-up manner
    for (i = 0; i <= n; i++) {
        for (w = 0; w <= W; w++) {
            if (i == 0 || w == 0)
                K[i][w] = 0;
            else if (items[i - 1].weight <= w)
                K[i][w] = (items[i - 1].value + K[i - 1][w - items[i - 1].weight] > K[i - 1][w])
                              ? items[i - 1].value + K[i - 1][w - items[i - 1].weight]
                              : K[i - 1][w];
            else
                K[i][w] = K[i - 1][w];
        }
    }

    int result = K[n][W];

    // Free allocated memory
    for (i = 0; i <= n; i++)
        free(K[i]);
    free(K);

    return result;
}

int main() {
    int i;
    srand(42);

    // Generate random items
    Item *items = (Item *)malloc(NUM_ITEMS * sizeof(Item));
    for (i = 0; i < NUM_ITEMS; i++) {
        items[i].value = rand() % 100 + 1;    // Random value between 1 and 100
        items[i].weight = rand() % 100 + 1;   // Random weight between 1 and 100
    }

    int capacity = MAX_WEIGHT;

    // Benchmark the knapsack function
    clock_t start_time = clock();
    int max_value = knapsack(items, NUM_ITEMS, capacity);
    clock_t end_time = clock();

    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Maximum value in Knapsack = %d\n", max_value);
    printf("Time Elapsed: %f\n", total_time);

    // Free allocated memory
    free(items);

    return 0;
}
