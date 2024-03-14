#include "queue.h"

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/*
 * Internal helper functions for
 */
int isInArray(int arr[], int size, int x);
void randArray(int *array, int size, int range);
int compare(const void *a, const void *b);
void dequeueByNode(Queue queue, Node current);

/*
 * This struct represents a single Node in the Queue.
 * Node's internal fields:
 *   connfd: The given connfd of the request received
 *   m_arrival: The time the request arrived at
 *   m_previous: The previous Node in the Queue
 *   m_next: The next Node in the Queue
 */
struct Node_t
{
    int connfd;
    struct timeval m_arrival;
    Node m_previous;
    Node m_next;
};

/*
 * This struct represents a Queue.
 * Queue's internal fields:
 *    m_size: The size of the queue
 *    m_max: The given max size of the queue
 *    m_first: The first Node in the queue
 *    m_last: The last Node in the queue
 */
struct Queue_t
{
    int m_size;
    int m_max;
    Node m_first;
    Node m_last;
};

Node NodeCreate()
{
    Node node = calloc(1, sizeof(*node));
    if (!node)
    {
        app_error("Allocation Error\n");
    }
    node->connfd = 0;
    node->m_previous = NULL;
    node->m_next = NULL;
    return node;
}

Queue QueueCreate(int max)
{
    Node node = NodeCreate();
    Queue queue = calloc(1, sizeof(*queue));
    if (!queue)
    {
        app_error("Allocation Error\n");
    }
    queue->m_size = 0;
    queue->m_max = max;
    queue->m_first = node;
    queue->m_last = node;
    return queue;
}

void QueueDestroy(Queue queue)
{
    Node temp = queue->m_first;
    Node runner = queue->m_first;
    while (runner)
    {
        runner = runner->m_next;
        free(temp);
        temp = runner;
    }
    free(queue);
}

void enqueue(Queue queue, int item, struct timeval arrival)
{
    if (!queue)
    {
        app_error("Received Null Argument\n");
    }
    pthread_mutex_lock(&m);
    if (queue->m_size == queue->m_max)
    {
        pthread_mutex_unlock(&m);
        return;
    }
    if (queue->m_size == 0)
    {
        queue->m_first->connfd = item;
        queue->m_size++;
        queue->m_first->m_arrival = arrival;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&m);
        return;
    }
    Node node = NodeCreate();
    if (!node)
    {
        app_error("Allocation Error\n");
    }
    node->m_arrival = arrival;
    node->connfd = item;
    node->m_previous = queue->m_last;
    queue->m_last->m_next = node;
    queue->m_last = node;
    queue->m_size++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&m);
}

int dequeue(Queue queue, struct timeval *arrival)
{
    if (!queue)
    {
        app_error("Received Null Argument\n");
    }
    pthread_mutex_lock(&m);
    while (queue->m_size == 0)
    {
        pthread_cond_wait(&cond, &m);
    }
    Node toRemove = queue->m_first;
    int item = toRemove->connfd;
    if (arrival != NULL)
    {
        *arrival = toRemove->m_arrival;
    }
    if (queue->m_size == 1)
    {
        // leave an empty node:
        toRemove->connfd = 0;
    }
    else
    {
        toRemove->m_next->m_previous = NULL;
        queue->m_first = toRemove->m_next;
        free(toRemove);
    }
    queue->m_size--;
    pthread_mutex_unlock(&m);
    return item;
}

int getSize(Queue queue)
{
    return queue->m_size;
}

int dequeueHalfRandom(Queue queue)
{
    pthread_mutex_lock(&m);
    int numToRemove = (queue->m_size + 1) / 2;
    if (!numToRemove)
    {
        pthread_mutex_unlock(&m);
        return 0;
    }
    int *chosenIndices = (int *)malloc((numToRemove) * sizeof(int));
    randArray(chosenIndices, numToRemove, queue->m_size);

    Node current = queue->m_first;

    qsort(chosenIndices, numToRemove, sizeof(int), compare);
    for (int j = 0; j < chosenIndices[0]; ++j)
    {
        current = current->m_next;
    }
    dequeueByNode(queue, current);
    for (int i = 1; i < numToRemove; ++i)
    {
        for (int j = chosenIndices[i - 1]; j < chosenIndices[i]; ++j)
        {
            current = current->m_next;
        }
        dequeueByNode(queue, current);
    }
    pthread_mutex_unlock(&m);
    return numToRemove;
}

//----------------------------------------------Internal Helper Functions-----------------------------------------------

int isInArray(int arr[], int size, int x)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i] == x)
        {
            // Return the index of the element if found
            return 1;
        }
    }
    // Return -1 if the element is not found in the array
    return 0;
}
void randArray(int *array, int size, int range)
{
    int index;
    srand(time(NULL));
    for (int i = 0; i < size; i++)
    {
        array[i] = -1;
        do
        {
            index = rand() % range;
        } while (isInArray(array, i, index));

        array[i] = index;
        // printf("I: %d, Index: %d\n", i, array[i]);
    }
}

int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

void dequeueByNode(Queue queue, Node current)
{
    Close(current->connfd);

    Node next = current->m_next;
    if (queue->m_size == 1)
    {
        current->connfd = 0; // maybe -1
        queue->m_size--;
        return; //????? look in piazza
    }
    if (!current->m_previous)
    { // current is first
        queue->m_first = next;
        queue->m_first->m_previous = NULL; // current->m_next is not null if queue->m_size != 1
    }
    else if (!current->m_next)
    {                                        // current is last
        queue->m_last = current->m_previous; // current->m_previous is not null if queue->m_size != 1
        queue->m_last->m_next = NULL;
    }
    else
    {
        current->m_previous->m_next = current->m_next;
        current->m_next->m_previous = current->m_previous;
    }
    // printf("deque: %d\n", count);
    queue->m_size--;
    free(current);
}