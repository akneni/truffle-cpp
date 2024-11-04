#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HASH_MAP_SIZE 1000  // Size of the hashmap
#define NUM_SIZE 300000      // Number of elements to insert and search

// Hashmap node structure
typedef struct HashNode {
    int key;
    int value;
    struct HashNode* next;
} HashNode;

// Hashmap represented as an array of pointers to HashNode
HashNode* hashMap[HASH_MAP_SIZE];

// Simple hash function
unsigned int hash(int key) {
    return key % HASH_MAP_SIZE;
}

// Function to insert a key-value pair into the hashmap
void insert(int key, int value) {
    unsigned int hashIndex = hash(key);
    HashNode* newNode = (HashNode*) malloc(sizeof(HashNode));
    newNode->key = key;
    newNode->value = value;
    newNode->next = hashMap[hashIndex];
    hashMap[hashIndex] = newNode;
}

// Function to search for a key in the hashmap
HashNode* search(int key) {
    unsigned int hashIndex = hash(key);
    HashNode* node = hashMap[hashIndex];
    while(node != NULL) {
        if(node->key == key)
            return node;
        node = node->next;
    }
    return NULL;
}

int main() {
    int i;
    srand(42);

    // Initialize hashmap
    for(i = 0; i < HASH_MAP_SIZE; i++) {
        hashMap[i] = NULL;
    }

    // Benchmark insertion
    clock_t start_insert = clock();
    for(i = 0; i < NUM_SIZE; i++) {
        int key = rand();
        int value = rand();
        insert(key, value);
    }
    clock_t end_insert = clock();
    double time_insert = (double)(end_insert - start_insert) / CLOCKS_PER_SEC;
    printf("Time taken to insert %d elements: %f seconds\n", NUM_SIZE, time_insert);

    // Benchmark search
    clock_t start_search = clock();
    int found = 0;
    for(i = 0; i < NUM_SIZE; i++) {
        int key = rand();
        if(search(key) != NULL)
            found++;
    }
    clock_t end_search = clock();
    double time_search = (double)(end_search - start_search) / CLOCKS_PER_SEC;
    printf("Time taken to search %d elements: %f seconds\n", NUM_SIZE, time_search);
    printf("Number of elements found: %d\n", found);

    // Calculate total time
    double total_time = time_insert + time_search;
    printf("Time Elapsed: %f\n", total_time);

    // Cleanup code to free allocated memory
    for(i = 0; i < HASH_MAP_SIZE; i++) {
        HashNode* node = hashMap[i];
        while(node != NULL) {
            HashNode* temp = node;
            node = node->next;
            free(temp);
        }
    }

    return 0;
}
