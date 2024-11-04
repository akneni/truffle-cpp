// LINK: -lm
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 1000000
#define ITERATIONS 100

double complex_calculation(int* data, int size, int mode) {
    double result = 0.0;

    // Multiple branching paths based on mode
    switch(mode) {
        case 0:
            // Exponential calculations
            for(int i = 0; i < size; i++) {
                if(data[i] % 2 == 0) {
                    result += exp(sqrt((double)data[i]));
                } else if(data[i] % 3 == 0) {
                    result += pow(data[i], 1.5);
                } else if(data[i] % 5 == 0) {
                    result += log(data[i] + 1) * sqrt(data[i]);
                } else {
                    result += sin(data[i]) * cos(data[i]);
                }
            }
            break;

        case 1:
            // Nested conditional processing
            for(int i = 0; i < size; i++) {
                if(data[i] > size/2) {
                    if(data[i] % 4 == 0) {
                        result += pow(data[i], 2);
                    } else if(data[i] % 7 == 0) {
                        result += (data[i] / 2.0) * log(data[i]);
                    }
                } else {
                    if(data[i] % 3 == 0) {
                        result += exp(data[i] % 10);
                    } else {
                        result += pow(sin(data[i]), 2);
                    }
                }
            }
            break;

        case 2:
            // Recursive-style calculations
            for(int i = 0; i < size; i++) {
                double temp = data[i];
                for(int j = 0; j < 5; j++) {
                    if(temp < 100) {
                        temp = pow(temp, 1.1);
                    } else if(temp < 1000) {
                        temp = sqrt(temp) * log(temp);
                    } else {
                        temp = exp(sqrt(temp) / 10);
                    }
                }
                result += temp;
            }
            break;

        default:
            // Fallback intensive computation
            for(int i = 0; i < size; i++) {
                result += pow(data[i], 3) * sin(data[i]);
            }
    }

    return result;
}

int main() {
    int* data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    srand(42);

    // Initialize array with random data
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
    }

    clock_t start = clock();
    double final_result = 0.0;

    // Multiple iterations of different computation modes
    for(int i = 0; i < ITERATIONS; i++) {
        int mode = i % 3;
        double result = complex_calculation(data, ARRAY_SIZE, mode);
        final_result += result;

        // Additional branching based on intermediate results
        if(result > 1000000) {
            final_result = sqrt(final_result);
        } else if(result < -1000000) {
            final_result = pow(final_result, 2);
        }
    }

    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / (double) CLOCKS_PER_SEC;

    printf("Final Result: %e\n", final_result);
    printf("Execution Time: %.4f seconds\n", cpu_time);

    free(data);
    return 0;
}

