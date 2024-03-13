#include "queue.h"
#include <sys/time.h>
#include "segel.h"

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
pthread_cond_t cond_flush = PTHREAD_COND_INITIALIZER;
int sumOfProcess = 0;

typedef struct Args
{
    Queue waiting;
    Queue handled;
    threads_stats stats;
} Args;

// HW3: Parse the new arguments too
void getargs(int *port, int *threads, int *queue_size, int argc, char *argv[])
{
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s <port> <threads> <queue_size> <schedalg>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
}

// Check for errors and check QUEUESTATUS
void *ThreadsHandle(void *arguments)
{
    struct Args *queues = arguments;
    while (1)
    {
        struct timeval arrival;
        int connfd = dequeue(queues->waiting, &arrival); // good story:)
        struct timeval dispatch;
        if (gettimeofday(&dispatch, NULL))
        {
            // error!
        }
        struct timeval res;
        timersub(&dispatch, &arrival, &res);
        requestHandle(connfd, arrival, res, queues->stats);
        Close(connfd);
        pthread_mutex_lock(&mutex_1);
        sumOfProcess--;
        pthread_cond_signal(&c);
        if(sumOfProcess == 0){
            pthread_cond_signal(&cond_flush);
        }
        pthread_mutex_unlock(&mutex_1);
        // printf("ID: %d | stat_req: %d | dynm_req: %d | total_req: %d\n", queues->stats.id, queues->stats.stat_req, queues->stats.dynm_req, queues->stats.total_req);
    }
    // How do we want to break this loop????????????????????????????????????????????????????
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_size, queue_size;
    struct sockaddr_in clientaddr;
    getargs(&port, &threads_size, &queue_size, argc, argv);
    Queue waiting = QueueCreate(queue_size);
    Queue handled = QueueCreate(queue_size);
    pthread_t threads[threads_size];
    threads_stats stats[threads_size];
    struct Args queues[threads_size];
    for (int i = 0; i < threads_size; i++)
    {
        threads_stats stat = calloc(1, sizeof(struct Threads_stats));
        if (!stat)
        {
            return -1; // ERROR!
        }
        stats[i] = stat;
        stats[i]->id = i;
        stats[i]->stat_req = 0;
        stats[i]->dynm_req = 0;
        stats[i]->total_req = 0;
        queues[i].waiting = waiting;
        queues[i].handled = handled;
        queues[i].stats = stats[i];
        int err = pthread_create(&threads[i], NULL, ThreadsHandle, (void *)&queues[i]);
        if (err != 0)
        {
            posix_error(err, "pthread_create error");
        }
    }
    listenfd = Open_listenfd(port);
    int isFull = 0;

    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        struct timeval arrival;
        if (gettimeofday(&arrival, NULL))
        {
            // error!
        }
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
            {
                pthread_cond_wait(&c, &mutex_1);
            }
            if (!strcmp(argv[4], "dh"))
            {
                Close(dequeue(waiting, NULL)); // by piazza it cannot be empty
                sumOfProcess--;
            }
            if (!strcmp(argv[4], "bf"))
            {
                pthread_cond_wait(&cond_flush, &mutex_1);
            }
             if (!strcmp(argv[4], "dr"))
            {
                sumOfProcess -= dequeueHalfRandom(waiting);
            }
        }
        pthread_mutex_unlock(&mutex_1);

        if (!isFull)
        { // maybe sync???
            sumOfProcess++;
            if (enqueue(waiting, connfd, arrival) != QUEUE_SUCCESS)
            {
                perror("Enqueue Error!"); // Decide what to do with errors!
            }
        }
    }
}
