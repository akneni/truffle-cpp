#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_SIZE 50000  // Number of elements to insert and traverse
#define NUM_TRAVERSALS 10000

// Linked list node structure
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Function to insert a new node at the end of the list
void insert(Node** head_ref, int new_data) {
    Node* new_node = (Node*) malloc(sizeof(Node));
    Node* last = *head_ref;

    new_node->data = new_data;
    new_node->next = NULL;

    if (*head_ref == NULL) {
        *head_ref = new_node;
        return;
    }

    while (last->next != NULL)
        last = last->next;

    last->next = new_node;
}

// Function to traverse the linked list
void traverse(Node* head) {
    Node* current = head;
    while (current != NULL) {
        // For benchmarking purposes, we won't print the data
        // printf("%d ", current->data);
        current = current->next;
    }
}

int main() {
    Node* head = NULL;
    int i;
    srand(42);

    // Benchmark insertion
    clock_t start_insert = clock();
    for (i = 0; i < NUM_SIZE; i++) {
        int num = rand();
        insert(&head, num);
    }
    clock_t end_insert = clock();
    double time_insert = (double)(end_insert - start_insert) / CLOCKS_PER_SEC;
    printf("Time taken to insert %d elements: %f seconds\n", NUM_SIZE, time_insert);

    // Benchmark traversal
    clock_t start_traverse = clock();
    for (int i = 0; i < NUM_TRAVERSALS; i++) {
        traverse(head);
    }

    clock_t end_traverse = clock();
    double time_traverse = (double)(end_traverse - start_traverse) / CLOCKS_PER_SEC;
    printf("Time taken to traverse %d elements: %f seconds\n", NUM_SIZE, time_traverse);

    // Calculate total time
    double total_time = time_insert + time_traverse;
    printf("Time Elapsed: %f\n", total_time);

    // Cleanup code to free allocated memory
    Node* current = head;
    Node* next_node;
    while (current != NULL) {
        next_node = current->next;
        free(current);
        current = next_node;
    }

    return 0;
}
