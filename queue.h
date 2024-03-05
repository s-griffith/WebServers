#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Queue_t* Queue;

typedef enum {
    QUEUE_SUCCESS,
    QUEUE_NULL_ARGUMENT,
    QUEUE_EMPTY,
    QUEUE_ERROR
} QueueResult;

Queue QueueCreate();

void QueueDestroy(Queue queue);

QueueResult enqueue(Queue queue, int item);

QueueResult dequeue(Queue queue);

int isEmpty(Queue queue);

int isFull(Queue queue);

int getSize(Queue queue);




#endif //QUEUE_H