#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define NUM_NODES 1000000     // Number of vertices in the graph
#define NUM_EDGES 5000000     // Number of edges in the graph

// Structure to represent a node in the adjacency list
typedef struct AdjListNode {
    int dest;
    int weight;
    struct AdjListNode* next;
} AdjListNode;

// Structure to represent the adjacency list
typedef struct AdjList {
    AdjListNode* head;
} AdjList;

// Structure to represent the graph
typedef struct Graph {
    int V;
    AdjList* array;
} Graph;

// Structure to represent a min heap node
typedef struct MinHeapNode {
    int v;
    int dist;
} MinHeapNode;

// Structure to represent a min heap
typedef struct MinHeap {
    int size;      // Number of heap nodes present
    int capacity;  // Capacity of min heap
    int *pos;      // Positions of nodes in min heap
    MinHeapNode *array; // Array of MinHeapNode structures
} MinHeap;

// Function to create a new adjacency list node
AdjListNode* newAdjListNode(int dest, int weight) {
    AdjListNode* newNode = 
        (AdjListNode*) malloc(sizeof(AdjListNode));
    newNode->dest = dest;
    newNode->weight = weight;
    newNode->next = NULL;
    return newNode;
}

// Function to create a graph with V vertices
Graph* createGraph(int V) {
    Graph* graph = 
        (Graph*) malloc(sizeof(Graph));
    graph->V = V;
    graph->array = 
        (AdjList*) malloc(V * sizeof(AdjList));

    int i;
    for (i = 0; i < V; ++i)
        graph->array[i].head = NULL;

    return graph;
}

// Function to add an edge to the graph
void addEdge(Graph* graph, int src, int dest, int weight) {
    // Add edge from src to dest
    AdjListNode* newNode = newAdjListNode(dest, weight);
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;

    // Since it's an undirected graph, add an edge from dest to src
    newNode = newAdjListNode(src, weight);
    newNode->next = graph->array[dest].head;
    graph->array[dest].head = newNode;
}

// Function to create a new MinHeap
MinHeap* createMinHeap(int capacity) {
    MinHeap* minHeap = 
        (MinHeap*) malloc(sizeof(MinHeap));
    minHeap->pos = 
        (int *)malloc(capacity * sizeof(int));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = 
        (MinHeapNode*) malloc(capacity * sizeof(MinHeapNode));
    return minHeap;
}

// Function to swap two nodes of min heap
void swapMinHeapNode(MinHeapNode* a, MinHeapNode* b) {
    MinHeapNode t = *a;
    *a = *b;
    *b = t;
}

// Standard minHeapify function
void minHeapify(MinHeap* minHeap, int idx) {
    int smallest, left, right;
    smallest = idx;
    left = 2 * idx + 1;
    right = 2 * idx + 2;

    if (left < minHeap->size && 
        minHeap->array[left].dist < minHeap->array[smallest].dist )
        smallest = left;

    if (right < minHeap->size && 
        minHeap->array[right].dist < minHeap->array[smallest].dist )
        smallest = right;

    if (smallest != idx) {
        // Swap positions
        minHeap->pos[minHeap->array[smallest].v] = idx;
        minHeap->pos[minHeap->array[idx].v] = smallest;

        // Swap nodes
        swapMinHeapNode(&minHeap->array[smallest], 
                        &minHeap->array[idx]);

        minHeapify(minHeap, smallest);
    }
}

// Function to check if minHeap is empty
int isEmpty(MinHeap* minHeap) {
    return minHeap->size == 0;
}

// Function to extract minimum node from heap
MinHeapNode extractMin(MinHeap* minHeap) {
    if (isEmpty(minHeap)) {
        MinHeapNode emptyNode = { -1, INT_MAX };
        return emptyNode;
    }

    // Store the root node
    MinHeapNode root = minHeap->array[0];

    // Replace root node with last node
    MinHeapNode lastNode = 
        minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;

    // Update positions
    minHeap->pos[root.v] = minHeap->size - 1;
    minHeap->pos[lastNode.v] = 0;

    // Reduce heap size and heapify root
    minHeap->size--;
    minHeapify(minHeap, 0);

    return root;
}

// Function to decrease distance value of a vertex v
void decreaseKey(MinHeap* minHeap, int v, int dist) {
    // Get the index of v in heap array
    int i = minHeap->pos[v];

    // Update its dist value
    minHeap->array[i].dist = dist;

    // Travel up while the complete tree is not heapified
    while (i && minHeap->array[i].dist < 
                   minHeap->array[(i - 1) / 2].dist) {
        // Swap this node with its parent
        minHeap->pos[minHeap->array[i].v] = (i - 1) / 2;
        minHeap->pos[minHeap->array[(i - 1) / 2].v] = i;
        swapMinHeapNode(&minHeap->array[i],  
                        &minHeap->array[(i - 1) / 2]);

        // Move to parent index
        i = (i - 1) / 2;
    }
}

// Function to check if a given vertex is in min heap
int isInMinHeap(MinHeap *minHeap, int v) {
   if (minHeap->pos[v] < minHeap->size)
     return 1;
   return 0;
}

// Dijkstra's algorithm
void dijkstra(Graph* graph, int src) {
    int V = graph->V;
    int dist[V];      // Output array. dist[i] will hold the shortest distance from src to i

    // Min heap
    MinHeap* minHeap = createMinHeap(V);

    int v;
    for (v = 0; v < V; ++v) {
        dist[v] = INT_MAX;
        minHeap->array[v].v = v;
        minHeap->array[v].dist = dist[v];
        minHeap->pos[v] = v;
    }

    // Make distance value of source vertex as 0
    dist[src] = 0;
    decreaseKey(minHeap, src, dist[src]);

    // Initially, size of min heap is equal to V
    minHeap->size = V;

    // While min heap is not empty
    while (!isEmpty(minHeap)) {
        // Extract vertex with minimum distance value
        MinHeapNode minHeapNode = extractMin(minHeap);
        int u = minHeapNode.v; // Store the extracted vertex number

        // Traverse through all adjacent vertices of u
        AdjListNode* pCrawl = graph->array[u].head;
        while (pCrawl != NULL) {
            int v = pCrawl->dest;

            // If shortest distance to v is not finalized and distance to v through u is less
            if (isInMinHeap(minHeap, v) && dist[u] != INT_MAX && 
                pCrawl->weight + dist[u] < dist[v]) {
                dist[v] = dist[u] + pCrawl->weight;

                // Update distance value in min heap
                decreaseKey(minHeap, v, dist[v]);
            }
            pCrawl = pCrawl->next;
        }
    }

    // Print the calculated shortest distances
    // Uncomment the next line to print distances
    // printArr(dist, V);

    // Free memory
    free(minHeap->pos);
    free(minHeap->array);
    free(minHeap);
}

int main() {
    srand(42);
    int V = NUM_NODES;
    int E = NUM_EDGES;
    Graph* graph = createGraph(V);

    // Generate a random graph
    int i;
    for (i = 0; i < E; i++) {
        int src = rand() % V;
        int dest = rand() % V;
        int weight = rand() % 100 + 1; // Weight between 1 and 100
        addEdge(graph, src, dest, weight);
    }

    // Benchmark Dijkstra's algorithm
    clock_t start_time = clock();
    dijkstra(graph, 0);
    clock_t end_time = clock();
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Print the total time elapsed
    printf("Time Elapsed: %f\n", total_time);

    // Cleanup: Free graph memory
    for (i = 0; i < V; i++) {
        AdjListNode* adjNode = graph->array[i].head;
        while (adjNode != NULL) {
            AdjListNode* temp = adjNode;
            adjNode = adjNode->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);

    return 0;
}
