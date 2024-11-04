#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MATRIX_SIZE 1750  // Size of the square matrix

// Function to generate a random square matrix of size MATRIX_SIZE x MATRIX_SIZE
void generate_random_matrix(double **matrix, int size) {
    int i, j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            // Generate random double between -10 and 10
            matrix[i][j] = ((double)rand() / RAND_MAX) * 20.0 - 10.0;
        }
    }
}

// Function to perform LU Decomposition on a square matrix
void lu_decomposition(double **matrix, double **L, double **U, int size) {
    int i, j, k;

    // Initialize L and U matrices
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            L[i][j] = 0.0;
            U[i][j] = 0.0;
        }
    }

    for (i = 0; i < size; i++) {
        // Upper Triangular Matrix U
        for (k = i; k < size; k++) {
            double sum = 0.0;
            for (j = 0; j < i; j++)
                sum += L[i][j] * U[j][k];
            U[i][k] = matrix[i][k] - sum;
        }

        // Lower Triangular Matrix L
        for (k = i; k < size; k++) {
            if (i == k)
                L[i][i] = 1.0;  // Diagonal as 1
            else {
                double sum = 0.0;
                for (j = 0; j < i; j++)
                    sum += L[k][j] * U[j][i];
                L[k][i] = (matrix[k][i] - sum) / U[i][i];
            }
        }
    }
}

int main() {
    int i;

    // Allocate memory for matrices
    double **matrix = (double **)malloc(MATRIX_SIZE * sizeof(double *));
    double **L = (double **)malloc(MATRIX_SIZE * sizeof(double *));
    double **U = (double **)malloc(MATRIX_SIZE * sizeof(double *));
    for (i = 0; i < MATRIX_SIZE; i++) {
        matrix[i] = (double *)malloc(MATRIX_SIZE * sizeof(double));
        L[i] = (double *)malloc(MATRIX_SIZE * sizeof(double));
        U[i] = (double *)malloc(MATRIX_SIZE * sizeof(double));
    }

    // Set seed to 42 for reproducibility
    srand(42);

    // Generate random matrix
    generate_random_matrix(matrix, MATRIX_SIZE);

    // Benchmark LU Decomposition
    clock_t start_time = clock();
    lu_decomposition(matrix, L, U, MATRIX_SIZE);
    clock_t end_time = clock();

    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Print time elapsed
    printf("Time Elapsed: %f\n", total_time);

    // Cleanup allocated memory
    for (i = 0; i < MATRIX_SIZE; i++) {
        free(matrix[i]);
        free(L[i]);
        free(U[i]);
    }
    free(matrix);
    free(L);
    free(U);

    return 0;
}
