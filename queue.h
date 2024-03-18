#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "request.h"
#include "segel.h"
#include <sys/time.h>

/*
 * Typedefs for defining Queue and Node:
*/
typedef struct Queue_t* Queue;
typedef struct Node_t* Node;

/*
 * Creates a Node
 * Receives no parameters
 * @return
 *      A new Node
*/
Node NodeCreate();

/*
 * Creates a Queue
 * @param max: The maximum allowed size of the queue
 * @return 
 *      A new Queue
*/
Queue QueueCreate(int max);

/*
 * Destroys a Queue
 * @param queue: The queue to be destroyed
 * @return 
 *      void
*/
void QueueDestroy(Queue queue);

/*
 * Adds an item to the end of a queue
 * @param queue: The queue to add the object to
 * @param item: The item to be added
 * @param arrival: The time the item reached the system
 * @return
 *      void
*/
void enqueue(Queue queue, int item, struct timeval arrival);

/*
 * Removes the head of the queue
 * @param queue: The queue to be removed from
 * @param arrival: A variable used to return the request's arrival time
 * @return
 *      The connfd of the request removed
*/
int dequeue(Queue queue, struct timeval* arrival);

/*
 * Randomly removes half of the items in the queue
 * @param queue: The queue to be diluted
 * @return
 *      The number of items removed
*/
int dequeueHalfRandom(Queue queue);

#endif //QUEUE_H