#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 550   // Size of the matrices (MATRIX_SIZE x MATRIX_SIZE)
#define NUM_ITERATIONS 10 // Number of times to repeat the multiplication for benchmarking

// Function to allocate a matrix of size rows x cols
double** allocateMatrix(int rows, int cols) {
    double** matrix = (double**) malloc(rows * sizeof(double*));
    if (matrix == NULL) {
        perror("Failed to allocate matrix rows");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < rows; i++) {
        matrix[i] = (double*) malloc(cols * sizeof(double));
        if (matrix[i] == NULL) {
            perror("Failed to allocate matrix columns");
            exit(EXIT_FAILURE);
        }
    }
    return matrix;
}

// Function to free a matrix
void freeMatrix(double** matrix, int rows) {
    for(int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Function to initialize a matrix with random values
void initializeMatrix(double** matrix, int rows, int cols) {
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            matrix[i][j] = ((double) rand() / (double) RAND_MAX);
        }
    }
}

// Function to perform matrix multiplication: C = A * B
void multiplyMatrices(double** A, double** B, double** C, int size) {
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            C[i][j] = 0.0;
            for(int k = 0; k < size; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    srand(42);

    // Allocate matrices
    double** A = allocateMatrix(MATRIX_SIZE, MATRIX_SIZE);
    double** B = allocateMatrix(MATRIX_SIZE, MATRIX_SIZE);
    double** C = allocateMatrix(MATRIX_SIZE, MATRIX_SIZE);

    // Initialize matrices A and B with random values
    initializeMatrix(A, MATRIX_SIZE, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE, MATRIX_SIZE);

    // Benchmark matrix multiplication
    clock_t start_time = clock();
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        multiplyMatrices(A, B, C, MATRIX_SIZE);
    }
    clock_t end_time = clock();

    // Calculate total time
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Time taken to perform matrix multiplication %d times: %f seconds\n", NUM_ITERATIONS, total_time);

    // Cleanup allocated memory
    freeMatrix(A, MATRIX_SIZE);
    freeMatrix(B, MATRIX_SIZE);
    freeMatrix(C, MATRIX_SIZE);

    // Print the total time elapsed
    printf("Time Elapsed: %f\n", total_time);

    return 0;
}
