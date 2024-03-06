#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "request.h"
#include "segel.h"

typedef struct Queue_t* Queue;
typedef struct Node* Node;

typedef enum {
    QUEUE_SUCCESS,
    QUEUE_NULL_ARGUMENT,
    QUEUE_EMPTY,
    QUEUE_ERROR,
    QUEUE_FULL
} QueueResult;

Queue QueueCreate(int max);

void QueueDestroy(Queue queue);

QueueResult enqueue(Queue queue, int item);

int dequeue(Queue queue);

int isEmpty(Queue queue);

int isFull(Queue queue);

int getSize(Queue queue);

Node getHead(Queue queue);




#endif //QUEUE_H