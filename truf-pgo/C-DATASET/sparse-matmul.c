#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 2500      // Size of the square matrices
#define SPARSITY 0.01         // Sparsity percentage (e.g., 0.01 for 1% non-zero elements)

typedef struct {
    int row;
    int col;
    double value;
} Element;

typedef struct {
    int rows;
    int cols;
    int num_nonzero;
    Element *elements; // Array of non-zero elements
} SparseMatrix;

// Function to create a random sparse matrix
SparseMatrix *createSparseMatrix(int rows, int cols, double sparsity) {
    SparseMatrix *matrix = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    matrix->rows = rows;
    matrix->cols = cols;

    int max_nonzero = (int)(rows * cols * sparsity);
    matrix->num_nonzero = max_nonzero;
    matrix->elements = (Element *)malloc(max_nonzero * sizeof(Element));

    int i;
    for (i = 0; i < max_nonzero; i++) {
        matrix->elements[i].row = rand() % rows;
        matrix->elements[i].col = rand() % cols;
        matrix->elements[i].value = (double)(rand() % 100 + 1);
    }

    return matrix;
}

// Function to multiply two sparse matrices
SparseMatrix *multiplySparseMatrices(SparseMatrix *A, SparseMatrix *B) {
    if (A->cols != B->rows) {
        printf("Incompatible matrix dimensions for multiplication.\n");
        return NULL;
    }

    // Resultant sparse matrix
    SparseMatrix *result = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    result->rows = A->rows;
    result->cols = B->cols;
    result->num_nonzero = 0;
    result->elements = NULL; // We'll dynamically allocate as we find non-zero elements

    // Temporary array to store intermediate results
    double *temp = (double *)calloc(A->rows * B->cols, sizeof(double));

    // Multiply non-zero elements
    int i, j;
    for (i = 0; i < A->num_nonzero; i++) {
        for (j = 0; j < B->num_nonzero; j++) {
            if (A->elements[i].col == B->elements[j].row) {
                int row = A->elements[i].row;
                int col = B->elements[j].col;
                double value = A->elements[i].value * B->elements[j].value;
                temp[row * B->cols + col] += value;
            }
        }
    }

    // Count non-zero elements in the result
    int count = 0;
    for (i = 0; i < result->rows * result->cols; i++) {
        if (temp[i] != 0.0) {
            count++;
        }
    }

    result->num_nonzero = count;
    result->elements = (Element *)malloc(count * sizeof(Element));

    // Populate the result elements
    int index = 0;
    for (i = 0; i < result->rows; i++) {
        for (j = 0; j < result->cols; j++) {
            double value = temp[i * result->cols + j];
            if (value != 0.0) {
                result->elements[index].row = i;
                result->elements[index].col = j;
                result->elements[index].value = value;
                index++;
            }
        }
    }

    free(temp);
    return result;
}

// Function to free a sparse matrix
void freeSparseMatrix(SparseMatrix *matrix) {
    free(matrix->elements);
    free(matrix);
}

int main() {
    srand(42);

    // Create two random sparse matrices
    SparseMatrix *A = createSparseMatrix(MATRIX_SIZE, MATRIX_SIZE, SPARSITY);
    SparseMatrix *B = createSparseMatrix(MATRIX_SIZE, MATRIX_SIZE, SPARSITY);

    // Benchmark multiplication
    clock_t start_multiply = clock();
    SparseMatrix *C = multiplySparseMatrices(A, B);
    clock_t end_multiply = clock();
    double time_multiply = (double)(end_multiply - start_multiply) / CLOCKS_PER_SEC;
    printf("Time taken to multiply two sparse matrices: %f seconds\n", time_multiply);

    // Calculate total time
    double total_time = time_multiply;
    printf("Time Elapsed: %f\n", total_time);

    // Cleanup code to free allocated memory
    freeSparseMatrix(A);
    freeSparseMatrix(B);
    freeSparseMatrix(C);

    return 0;
}
