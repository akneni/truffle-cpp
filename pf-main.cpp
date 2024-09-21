#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <vector>
#include <random>
#include <chrono>

// Simulate a process that makes heavy use of branching and decision-making
int simulateProcess(int input) {
    if (input % 3 == 0) {
        if (input % 5 == 0) {
            return input * 2;
        } else {
            return input / 2;
        }
    } else if (input % 7 == 0) {
        if (input % 2 == 0) {
            return input + 10;
        } else {
            return input - 10;
        }
    } else {
        return input * input;
    }
}

// Simulate a decision-heavy process based on large input data
int runSimulation(const std::vector<int>& data) {
    int result = 0;
    for (auto d : data) {
        result += simulateProcess(d);
    }
    return result;
}

int main() {
    // Create a large dataset of random inputs

    const int dataSize = 100000000;
    std::vector<int> data(dataSize);
    std::mt19937 rng(42); // Random number generator
    std::uniform_int_distribution<int> dist(1, 10000);

    for (int i = 0; i < dataSize; ++i) {
        data[i] = dist(rng);
    }

    // Run the simulation and measure time
    auto start = std::chrono::high_resolution_clock::now();
    int result = runSimulation(data);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = end - start;
    std::cout << "Simulation result: " << result << "\n";
    std::cout << "Execution time: " << diff.count() << " s\n";



    return 0;
}
