#include <stdbool.h>
#include <stdio.h>
#define NUM_FRAMES 8

#define MAX_SIZE NUM_FRAMES

// Defining the Queue structure
typedef struct {
    int items[MAX_SIZE];
    int front;
    int rear;
    int size;
} Queue;

void initializeQueue(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

bool isEmpty(Queue *q) {
    return (q->size == 0);
}

bool isFull(Queue *q) {
    return (q->size == MAX_SIZE);
}

void enqueue(Queue *q, int value) {
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }

    q->rear = (q->rear + 1) % MAX_SIZE;
    q->items[q->rear] = value;
    q->size++;
}

int dequeue(Queue *q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return -1;
    }

    int value = q->items[q->front];
    q->front = (q->front + 1) % MAX_SIZE;
    q->size--;
    return value;
}

void printQueue(Queue *q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }

    printf("Current Queue: ");
    for (int i = 0; i < q->size; i++) {
        int index = (q->front + i) % MAX_SIZE;
        printf("%d ", q->items[index]);
    }
    printf("\n");
}