// LINK: -lm
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define N_BODIES 3500       // Number of bodies in the simulation
#define TIME_STEPS 17       // Number of time steps to simulate
#define DELTA_T 0.01        // Time increment per step
#define G 6.67430e-11       // Gravitational constant

// Structure to represent a body in space
typedef struct {
    double x, y, z;       // Position
    double vx, vy, vz;    // Velocity
    double mass;
} Body;

int main() {
    int i, j, step;
    srand(42);

    // Allocate memory for bodies
    Body* bodies = (Body*) malloc(N_BODIES * sizeof(Body));

    // Initialize bodies with random positions, velocities, and masses
    for (i = 0; i < N_BODIES; i++) {
        bodies[i].x = ((double) rand() / RAND_MAX) * 1e5;
        bodies[i].y = ((double) rand() / RAND_MAX) * 1e5;
        bodies[i].z = ((double) rand() / RAND_MAX) * 1e5;
        bodies[i].vx = ((double) rand() / RAND_MAX) * 1e3;
        bodies[i].vy = ((double) rand() / RAND_MAX) * 1e3;
        bodies[i].vz = ((double) rand() / RAND_MAX) * 1e3;
        bodies[i].mass = ((double) rand() / RAND_MAX) * 1e25 + 1e20;
    }

    // Benchmark simulation
    clock_t start_simulation = clock();

    // Simulate for a given number of time steps
    for (step = 0; step < TIME_STEPS; step++) {
        // For each body, calculate the net force from all other bodies
        for (i = 0; i < N_BODIES; i++) {
            double ax = 0.0, ay = 0.0, az = 0.0;
            for (j = 0; j < N_BODIES; j++) {
                if (i != j) {
                    double dx = bodies[j].x - bodies[i].x;
                    double dy = bodies[j].y - bodies[i].y;
                    double dz = bodies[j].z - bodies[i].z;
                    double distance = sqrt(dx*dx + dy*dy + dz*dz) + 1e-10;  // Add small value to avoid division by zero
                    double force = G * bodies[i].mass * bodies[j].mass / (distance * distance);
                    double acceleration = force / bodies[i].mass;
                    ax += acceleration * (dx / distance);
                    ay += acceleration * (dy / distance);
                    az += acceleration * (dz / distance);
                }
            }
            // Update velocities
            bodies[i].vx += ax * DELTA_T;
            bodies[i].vy += ay * DELTA_T;
            bodies[i].vz += az * DELTA_T;
        }

        // Update positions
        for (i = 0; i < N_BODIES; i++) {
            bodies[i].x += bodies[i].vx * DELTA_T;
            bodies[i].y += bodies[i].vy * DELTA_T;
            bodies[i].z += bodies[i].vz * DELTA_T;
        }
    }

    clock_t end_simulation = clock();
    double total_time = (double)(end_simulation - start_simulation) / CLOCKS_PER_SEC;
    printf("Simulation completed for %d bodies and %d time steps.\n", N_BODIES, TIME_STEPS);
    printf("Time Elapsed: %f\n", total_time);

    // Cleanup
    free(bodies);

    return 0;
}
