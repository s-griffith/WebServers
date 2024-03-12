#include "queue.h"

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int sumOfProcess = 0;

typedef struct Args
{
    Queue waiting;
    Queue handled;
} Args;

// HW3: Parse the new arguments too
void getargs(int *port, int *threads, int *queue_size, char *schedalg, int argc, char *argv[])
{
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s <port> <threads> <queue_size> <schedalg>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    schedalg = argv[4];
}

// Check for errors and check QUEUESTATUS
void *ThreadsHandle(void *arguments)
{
    struct Args *queues = arguments;
    while (1)
    {
        int connfd = dequeue(queues->waiting); // good story:)

        requestHandle(connfd);
        Close(connfd);

        pthread_mutex_lock(&mutex_1);
        sumOfProcess--;
        pthread_cond_signal(&c);
        pthread_mutex_unlock(&mutex_1);
    }
    // How do we want to break this loop????????????????????????????????????????????????????
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_size, queue_size;
    struct sockaddr_in clientaddr;
    char *schedalg; // to fix
    getargs(&port, &threads_size, &queue_size, schedalg, argc, argv);
    Queue waiting = QueueCreate(queue_size);
    Queue handled = QueueCreate(queue_size);
    struct Args queues;
    queues.waiting = waiting;
    queues.handled = handled;
    pthread_t threads[threads_size];
    for (int i = 0; i < threads_size; i++)
    {
        int err = pthread_create(&threads[i], NULL, ThreadsHandle, (void *)&queues);
        if (err != 0)
        {
            posix_error(err, "pthread_create error");
        }
    }
    listenfd = Open_listenfd(port);
    int isFull = 0; // bool?

    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        isFull = 0;
        pthread_mutex_lock(&mutex_1);

        while (sumOfProcess >= queue_size)
        {
            if (!strcmp(argv[4], "dt"))
            {
                Close(connfd);
                isFull = 1;
                pthread_mutex_unlock(&mutex_1);
                break;
            }
            if (!strcmp(argv[4], "block"))
            { // schedalg, not argv[4]
                pthread_cond_wait(&c, &mutex_1);
            }
            if (!strcmp(argv[4], "dh"))
            {
                Close(dequeue(queues.waiting)); // by piazza it cannot be empty
                sumOfProcess--;
            }
        }
        pthread_mutex_unlock(&mutex_1);

        if (!isFull)
        { // maybe sync???
            sumOfProcess++;
            if (enqueue(waiting, connfd) != QueueResult::Queue_SUCCESS)
            {
                perror("Enqueue Error!"); //Decide what to do with errors!
            }
        }
    }
}
