#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "request.h"
//#include "segel.h"
#include <sys/time.h>


typedef struct Queue_t* Queue;
typedef struct Node_t* Node;

struct Node_t
{
    int connfd;
    struct timeval m_arrival;
    Node m_previous;
    Node m_next;
};

struct Queue_t
{
    int size;
    int max;
    Node m_first;
    Node m_last;
};

typedef enum {
    QUEUE_SUCCESS,
    QUEUE_NULL_ARGUMENT,
    QUEUE_EMPTY,
    QUEUE_ERROR,
    QUEUE_FULL
} QueueResult;

Queue QueueCreate(int max);

Node NodeCreate();

void QueueDestroy(Queue queue);

QueueResult enqueue(Queue queue, int item, struct timeval arrival);

int dequeue(Queue queue, struct timeval* arrival);

int isEmpty(Queue queue);

int isFull(Queue queue);

int getSize(Queue queue);

Node getHead(Queue queue);




#endif //QUEUE_H