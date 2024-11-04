
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_DEGREE 3    // B-tree minimum degree
#define NUM_SIZE 4000000   // Number of elements to insert and search

// B-tree node structure
typedef struct BTreeNode {
    int *keys;              // An array of keys
    int t;                  // Minimum degree (defines the range for number of keys)
    struct BTreeNode **C;   // An array of child pointers
    int n;                  // Current number of keys
    int leaf;               // Is true when node is leaf. Otherwise false
} BTreeNode;

// Function to create a new B-tree node
BTreeNode *createNode(int t, int leaf) {
    BTreeNode *node = (BTreeNode *)malloc(sizeof(BTreeNode));
    node->t = t;
    node->leaf = leaf;

    // Allocate memory for maximum number of possible keys and child pointers
    node->keys = (int *)malloc((2 * t - 1) * sizeof(int));
    node->C = (BTreeNode **)malloc(2 * t * sizeof(BTreeNode *));
    node->n = 0;

    return node;
}

// Function to traverse the tree (for debugging purposes)
void traverse(BTreeNode *root) {
    if (root != NULL) {
        int i;
        for (i = 0; i < root->n; i++) {
            if (!root->leaf)
                traverse(root->C[i]);
            printf("%d ", root->keys[i]);
        }
        if (!root->leaf)
            traverse(root->C[i]);
    }
}

// Function to search a key in the B-tree
BTreeNode *search(BTreeNode *root, int k) {
    int i = 0;
    while (i < root->n && k > root->keys[i])
        i++;

    if (i < root->n && k == root->keys[i])
        return root;

    if (root->leaf)
        return NULL;

    return search(root->C[i], k);
}

// Function to split the child y of node x at index i
void splitChild(BTreeNode *x, int i, BTreeNode *y) {
    int t = y->t;
    BTreeNode *z = createNode(t, y->leaf);
    z->n = t - 1;

    // Copy the last (t-1) keys of y to z
    int j;
    for (j = 0; j < t - 1; j++)
        z->keys[j] = y->keys[j + t];

    // Copy the last t children of y to z
    if (!y->leaf) {
        for (j = 0; j < t; j++)
            z->C[j] = y->C[j + t];
    }

    y->n = t - 1;

    // Create space for new child
    for (j = x->n; j >= i + 1; j--)
        x->C[j + 1] = x->C[j];

    x->C[i + 1] = z;

    // Move keys in x to make space for new key
    for (j = x->n - 1; j >= i; j--)
        x->keys[j + 1] = x->keys[j];

    x->keys[i] = y->keys[t - 1];
    x->n = x->n + 1;
}

// Function to insert a key into a non-full node
void insertNonFull(BTreeNode *x, int k) {
    int i = x->n - 1;

    if (x->leaf) {
        // Insert the new key into the node
        while (i >= 0 && k < x->keys[i]) {
            x->keys[i + 1] = x->keys[i];
            i--;
        }

        x->keys[i + 1] = k;
        x->n = x->n + 1;
    } else {
        // Find the child that is going to have the new key
        while (i >= 0 && k < x->keys[i])
            i--;

        i++;
        if (x->C[i]->n == 2 * x->t - 1) {
            splitChild(x, i, x->C[i]);

            if (k > x->keys[i])
                i++;
        }
        insertNonFull(x->C[i], k);
    }
}

// Function to insert a key into the B-tree
void insert(BTreeNode **root, int k) {
    BTreeNode *r = *root;

    // If root is full, then tree grows in height
    if (r->n == 2 * r->t - 1) {
        BTreeNode *s = createNode(r->t, 0);
        s->C[0] = r;
        splitChild(s, 0, r);

        int i = 0;
        if (s->keys[0] < k)
            i++;
        insertNonFull(s->C[i], k);
        *root = s;
    } else {
        insertNonFull(r, k);
    }
}

int main() {
    BTreeNode *root = createNode(MIN_DEGREE, 1);

    int i;
    srand(42);

    // Benchmark insertion
    clock_t start_insert = clock();
    for (i = 0; i < NUM_SIZE; i++) {
        int num = rand() % (NUM_SIZE * 10);
        insert(&root, num);
    }
    clock_t end_insert = clock();
    double time_insert = (double)(end_insert - start_insert) / CLOCKS_PER_SEC;
    printf("Time taken to insert %d elements: %f seconds\n", NUM_SIZE, time_insert);

    // Benchmark search
    clock_t start_search = clock();
    int found = 0;
    for (i = 0; i < NUM_SIZE; i++) {
        int num = rand() % (NUM_SIZE * 10);
        if (search(root, num) != NULL)
            found++;
    }
    clock_t end_search = clock();
    double time_search = (double)(end_search - start_search) / CLOCKS_PER_SEC;
    printf("Time taken to search %d elements: %f seconds\n", NUM_SIZE, time_search);
    printf("Number of elements found: %d\n", found);

    printf("Time Elapsed: %f\n", time_insert + time_search);

    return 0;
}
