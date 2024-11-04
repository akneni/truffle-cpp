// LINK: -lm
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MATRIX_SIZE 1000     // Size of the square matrix
#define NUM_ITERATIONS 1200 // Number of iterations for power method

int main() {
    int i, j;
    srand(42);  // Set seed for reproducibility

    // Allocate memory for MATRIX_SIZE x MATRIX_SIZE matrix
    double **matrix = (double **)malloc(MATRIX_SIZE * sizeof(double *));
    for (i = 0; i < MATRIX_SIZE; i++) {
        matrix[i] = (double *)malloc(MATRIX_SIZE * sizeof(double));
    }

    // Generate random matrix entries between -1 and 1
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        }
    }

    // Initialize a random vector
    double *vector = (double *)malloc(MATRIX_SIZE * sizeof(double));
    for (i = 0; i < MATRIX_SIZE; i++) {
        vector[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
    }

    // Start timing
    clock_t start_time = clock();

    double eigenvalue = 0.0;
    double eigenvalue_old = 0.0;

    // Power iteration method
    int iteration;
    for (iteration = 0; iteration < NUM_ITERATIONS; iteration++) {
        // Multiply matrix by vector: result = matrix * vector
        double *result = (double *)malloc(MATRIX_SIZE * sizeof(double));
        for (i = 0; i < MATRIX_SIZE; i++) {
            result[i] = 0.0;
            for (j = 0; j < MATRIX_SIZE; j++) {
                result[i] += matrix[i][j] * vector[j];
            }
        }

        // Compute norm of result vector
        double norm = 0.0;
        for (i = 0; i < MATRIX_SIZE; i++) {
            norm += result[i] * result[i];
        }
        norm = sqrt(norm);

        // Normalize result vector
        for (i = 0; i < MATRIX_SIZE; i++) {
            vector[i] = result[i] / norm;
        }

        // Estimate eigenvalue using Rayleigh quotient
        double numerator = 0.0;
        double denominator = 0.0;
        for (i = 0; i < MATRIX_SIZE; i++) {
            double mv = 0.0;
            for (j = 0; j < MATRIX_SIZE; j++) {
                mv += matrix[i][j] * vector[j];
            }
            numerator += vector[i] * mv;
            denominator += vector[i] * vector[i];
        }
        eigenvalue = numerator / denominator;

        // Check for convergence
        if (fabs(eigenvalue - eigenvalue_old) < 1e-6) {
            break;
        }
        eigenvalue_old = eigenvalue;

        free(result);
    }

    // End timing
    clock_t end_time = clock();
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Print the computed dominant eigenvalue
    printf("Computed dominant eigenvalue: %f\n", eigenvalue);

    // Print the time elapsed
    printf("Time Elapsed: %f\n", total_time);

    // Free allocated memory
    for (i = 0; i < MATRIX_SIZE; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(vector);

    return 0;
}
