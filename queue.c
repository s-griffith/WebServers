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

//----------------------------------------------Create and Destroy------------------------------------------------------

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

//----------------------------------------------Enqueue and Dequeue-----------------------------------------------------

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
        QueueDestroy(queue);
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

/*
 * Determines if an element is in the array
 * @param arr: The array to search
 * @param size: The size of the array
 * @param x: The value to search for
 * @return
 *      0 if not found, 1 if found
 */
int isInArray(int arr[], int size, int x)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i] == x)
        {
            return 1;
        }
    }
    return 0;
}

/*
 * Creates a random array
 * @param array: The array to be filled
 * @param size: The size of the array to be filled
 * @param range: The range of numbers to be created
 * @return
 *      void
 */
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
    }
}

/*
 * Compares two numbers
 * @param a: The first number to be checked, as void*
 * @param b: The second number to be checked, as void*
 * @return
 *      The difference between the two numbers
 */
int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

/*
 * Removes a specific node from the queue.
 * @param queue: The queue to remove from
 * @param current: The node to be removed
 * @return
 *      void
 */
void dequeueByNode(Queue queue, Node current)
{
    Close(current->connfd);

    Node next = current->m_next;
    if (queue->m_size == 1)
    {
        current->connfd = 0;
        queue->m_size--;
        return;
    }
    if (!current->m_previous)
    {
        queue->m_first = next;
        queue->m_first->m_previous = NULL;
    }
    else if (!current->m_next)
    {
        queue->m_last = current->m_previous;
        queue->m_last->m_next = NULL;
    }
    else
    {
        current->m_previous->m_next = current->m_next;
        current->m_next->m_previous = current->m_previous;
    }
    queue->m_size--;
    free(current);
}