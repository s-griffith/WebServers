#include "queue.h"

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

typedef struct Node
{
    int connfd;
    struct Node *m_previous;
    struct Node *m_next;
} *Node;

struct Queue_t
{
    int size;
    int max;
    Node m_first;
    Node m_last;
};

Node NodeCreate()
{
    Node node = calloc(1, sizeof(*node));
    if (!node)
    {
        return NULL;
    }
    node->connfd = 0;
    node->m_previous = NULL;
    node->m_next = NULL;
    return node;
}
Node getHead(Queue queue)
{
    return queue->m_first;
}

Queue QueueCreate(int max)
{
    Node node = NodeCreate();
    Queue queue = calloc(1, sizeof(*queue));
    if (!queue)
    {
        return NULL;
    }
    queue->size = 0;
    queue->max = max;
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

// Add condition variables!
QueueResult enqueue(Queue queue, int item)
{
    if (!queue)
    {
        return QUEUE_NULL_ARGUMENT;
    }
    pthread_mutex_lock(&m);
    if (queue->size == queue->max)
    {
        pthread_mutex_unlock(&m);
        return QUEUE_FULL;
    }
    if (queue->size == 0)
    {
        queue->m_first->connfd = item;
        queue->size++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&m);
        return QUEUE_SUCCESS;
    }
    Node node = NodeCreate();
    if (!node)
    {
        return QUEUE_ERROR;
    }
    node->connfd = item;
    node->m_previous = queue->m_last;
    queue->m_last->m_next = node;
    queue->m_last = node;
    queue->size++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&m);
    return QUEUE_SUCCESS;
}

// Add condition variables!
int dequeue(Queue queue)
{
    if (!queue)
    {
        // return QUEUE_NULL_ARGUMENT;
        return -1;
    }
    pthread_mutex_lock(&m);
    while (queue->size == 0)
    {
        pthread_cond_wait(&cond, &m);
    }
    Node toRemove = queue->m_first;
    int item = toRemove->connfd;
    if (!toRemove->m_next) // same as: queue->size == 1
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
    queue->size--;
    pthread_mutex_unlock(&m);
    return item;
}

int isEmpty(Queue queue)
{
    if (queue->size == 0)
    {
        return 1;
    }
    return 0;
}

int isFull(Queue queue)
{
    // Check according to max size given
    // Add max size as parameter once know in what format received
    return 0;
}

int getSize(Queue queue)
{
    return queue->size;
}
