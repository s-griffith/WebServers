#include "queue.h"

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

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
QueueResult enqueue(Queue queue, int item, struct timeval arrival)
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
        queue->m_first->m_arrival = arrival;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&m);
        return QUEUE_SUCCESS;
    }
    Node node = NodeCreate();
    if (!node)
    {
        return QUEUE_ERROR;
    }
    node->m_arrival = arrival;
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
int dequeue(Queue queue, struct timeval* arrival)
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
    if (arrival != NULL) {
        *arrival = toRemove->m_arrival;
    }
    if (queue->size == 1) // same as: queue->size == 1
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


int isInArray(int arr[], int size, int x) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == x) {
            // Return the index of the element if found
            return 1;
        }
    }
    // Return -1 if the element is not found in the array
    return 0;
}
void randArray(int *array, int size, int range){
    int index;
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array[i] = -1;
        do {
            index = rand() % range;
        } while (isInArray(array, i, index));
        
        array[i] = index;
        //printf("I: %d, Index: %d\n", i, array[i]);
    }
}
int dequeueHalfRandom(Queue queue) {
    int numToRemove = (queue->size+1)/2;
    pthread_mutex_lock(&m);
    //printf("Size: %d\n", numToRemove);
    int *chosenIndices = (int *)malloc((numToRemove) * sizeof(int));
    randArray(chosenIndices, numToRemove, queue->size);

    int count = 0;
    Node current = queue->m_first;
    Node next;
    if(queue->size == 1){
        current->connfd = 0; //maybe -1
        queue->size--;
        return 1; //????? look in piazza
    }
    while (current != NULL) {
        next = current->m_next;
        if (isInArray(chosenIndices, (queue->size+1/2), count)) {
            if(!current->m_previous){//current is first
                queue->m_first = next;
                queue->m_first->m_previous = NULL; //current->m_next is not null if queue->size != 1
            }
            else if(!current->m_next){//current is last
                queue->m_last = current->m_previous; //current->m_previous is not null if queue->size != 1
                queue->m_last->m_next = NULL;
            }
            else{
                current->m_previous->m_next = current->m_next;
                current->m_next->m_previous = current->m_previous;
            }
            //printf("deque: %d\n", count);
            queue->size--;
            free(current);
        }
        current = next;
        count++;
    }
    pthread_mutex_unlock(&m);
    return numToRemove;
}

