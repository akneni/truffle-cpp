#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_NODES 10000000     // Number of nodes in the graph
#define NUM_EDGES 15000000     // Number of edges in the graph

// Node structure for adjacency list
typedef struct Node {
    int dest;
    struct Node* next;
} Node;

// Adjacency list structure
typedef struct AdjList {
    Node* head;
} AdjList;

// Graph structure
typedef struct Graph {
    int V;
    AdjList* array;
} Graph;

// Queue structure for BFS
typedef struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
} Queue;

// Function to create a new node
Node* newNode(int dest) {
    Node* node = (Node*) malloc(sizeof(Node));
    node->dest = dest;
    node->next = NULL;
    return node;
}

// Function to create a graph
Graph* createGraph(int V) {
    Graph* graph = (Graph*) malloc(sizeof(Graph));
    graph->V = V;
    graph->array = (AdjList*) malloc(V * sizeof(AdjList));
    int i;
    for(i = 0; i < V; i++) {
        graph->array[i].head = NULL;
    }
    return graph;
}

// Function to add an edge to the graph
void addEdge(Graph* graph, int src, int dest) {
    Node* node = newNode(dest);
    node->next = graph->array[src].head;
    graph->array[src].head = node;

    // Since the graph is undirected, add an edge from dest to src also
    node = newNode(src);
    node->next = graph->array[dest].head;
    graph->array[dest].head = node;
}

// Function to create a queue
Queue* createQueue(unsigned capacity) {
    Queue* queue = (Queue*) malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;  // Important - see enqueue
    queue->array = (int*) malloc(queue->capacity * sizeof(int));
    return queue;
}

// Queue is full when size equals capacity
int isFull(Queue* queue) {
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(Queue* queue) {
    return (queue->size == 0);
}

// Function to add an item to the queue
void enqueue(Queue* queue, int item) {
    if(isFull(queue))
        return;
    queue->rear = (queue->rear + 1)%queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

// Function to remove an item from queue
int dequeue(Queue* queue) {
    if(isEmpty(queue))
        return -1;
    int item = queue->array[queue->front];
    queue->front = (queue->front +1)%queue->capacity;
    queue->size = queue->size -1;
    return item;
}

// BFS function
void BFS(Graph* graph, int startVertex) {
    int V = graph->V;
    int* visited = (int*) malloc(V * sizeof(int));
    int i;
    for(i = 0; i < V; i++) {
        visited[i] = 0;
    }

    Queue* queue = createQueue(V);

    visited[startVertex] = 1;
    enqueue(queue, startVertex);

    while(!isEmpty(queue)) {
        int currentVertex = dequeue(queue);
        // printf("Visited %d\n", currentVertex);  // Uncomment to see traversal

        Node* adjList = graph->array[currentVertex].head;
        while(adjList != NULL) {
            int adjVertex = adjList->dest;
            if(visited[adjVertex] == 0) {
                visited[adjVertex] = 1;
                enqueue(queue, adjVertex);
            }
            adjList = adjList->next;
        }
    }

    free(visited);
    free(queue->array);
    free(queue);
}

int main() {
    srand(42);

    // Create a graph with NUM_NODES vertices
    Graph* graph = createGraph(NUM_NODES);

    int i;
    // Randomly add NUM_EDGES edges to the graph
    for(i = 0; i < NUM_EDGES; i++) {
        int src = rand() % NUM_NODES;
        int dest = rand() % NUM_NODES;
        addEdge(graph, src, dest);
    }

    // Benchmark BFS
    clock_t start_bfs = clock();
    BFS(graph, 0);  // Starting BFS from vertex 0
    clock_t end_bfs = clock();

    double total_time = (double)(end_bfs - start_bfs) / CLOCKS_PER_SEC;
    printf("Time taken to perform BFS: %f seconds\n", total_time);
    printf("Time Elapsed: %f\n", total_time);

    // Cleanup
    for(i = 0; i < graph->V; i++) {
        Node* node = graph->array[i].head;
        while(node != NULL) {
            Node* temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);

    return 0;
}
