#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MATRIX_SIZE 2000  // Size of the square matrix

int main() {
    // Set random seed to 42 for reproducibility
    srand(42);

    // Allocate memory for matrix A (MATRIX_SIZE x MATRIX_SIZE), vector b, and solution vector x
    double **A = (double **)malloc(MATRIX_SIZE * sizeof(double *));
    double *b = (double *)malloc(MATRIX_SIZE * sizeof(double));
    double *x = (double *)malloc(MATRIX_SIZE * sizeof(double));

    for (int i = 0; i < MATRIX_SIZE; i++) {
        A[i] = (double *)malloc(MATRIX_SIZE * sizeof(double));
    }

    // Initialize matrix A and vector b with random values
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = ((double)rand()) / RAND_MAX;
        }
        b[i] = ((double)rand()) / RAND_MAX;
    }

    // Start timing the Gaussian elimination process
    clock_t start = clock();

    // Gaussian elimination with partial pivoting
    for (int i = 0; i < MATRIX_SIZE - 1; i++) {
        // Partial pivoting
        int maxRow = i;
        double maxVal = fabs(A[i][i]);
        for (int k = i + 1; k < MATRIX_SIZE; k++) {
            if (fabs(A[k][i]) > maxVal) {
                maxVal = fabs(A[k][i]);
                maxRow = k;
            }
        }

        // Swap the maximum row with the current row (if needed)
        if (maxRow != i) {
            double *tempRow = A[i];
            A[i] = A[maxRow];
            A[maxRow] = tempRow;

            double tempVal = b[i];
            b[i] = b[maxRow];
            b[maxRow] = tempVal;
        }

        // Forward elimination
        for (int k = i + 1; k < MATRIX_SIZE; k++) {
            if (A[i][i] == 0) {
                fprintf(stderr, "Division by zero detected during elimination at row %d!\n", i);
                exit(EXIT_FAILURE);
            }
            double factor = A[k][i] / A[i][i];
            for (int j = i; j < MATRIX_SIZE; j++) {
                A[k][j] -= factor * A[i][j];
            }
            b[k] -= factor * b[i];
        }
    }

    // Back substitution
    for (int i = MATRIX_SIZE - 1; i >= 0; i--) {
        x[i] = b[i];
        for (int j = i + 1; j < MATRIX_SIZE; j++) {
            x[i] -= A[i][j] * x[j];
        }
        if (A[i][i] == 0) {
            fprintf(stderr, "Division by zero detected during back substitution at row %d!\n", i);
            exit(EXIT_FAILURE);
        }
        x[i] /= A[i][i];
    }

    // Stop timing
    clock_t end = clock();

    // Calculate total time taken
    double total_time = (double)(end - start) / CLOCKS_PER_SEC;

    // Output the total time elapsed
    printf("Time Elapsed: %f\n", total_time);

    // Free allocated memory
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(A[i]);
    }
    free(A);
    free(b);
    free(x);

    return 0;
}
