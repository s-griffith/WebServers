#include "queue.h"
#include <sys/time.h>
#include "segel.h"

// Locks and condition variables:
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_flush = PTHREAD_COND_INITIALIZER;
int sumOfProcess = 0;

/*
 * A struct representing the arguments needed for ThreadsHandle()
 * waiting: a queue holding all the waiting requests
 * stats: a struct holding thread statistics
*/
typedef struct Args
{
    Queue waiting;
    threads_stats stats;
} Args;

/*
 * Parses the arguments received from the command line
 * @param port: The port to listen to
 * @param threads: The number of threads to create
 * @param queue_size: The size of the queue to create
 * @param argc: The number of arguments received
 * @param argv: The arguments received
 * @return 
 *      void
*/
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

/*
 * Manages the handling of requests by threads
 * @param arguments: The struct of arguments containing the waiting requests and thread stats
 * @return 
 *      void
*/
void *ThreadsHandle(void *arguments)
{
    struct Args *queues = arguments;
    while (1)
    {
        struct timeval arrival;
        int connfd = dequeue(queues->waiting, &arrival); 
        struct timeval dispatch;
        if (gettimeofday(&dispatch, NULL))
        {
            QueueDelete(queues->waiting);
            unix_error("Get Time of Day Error\n");
        }
        struct timeval res;
        timersub(&dispatch, &arrival, &res);
        requestHandle(connfd, arrival, res, queues->stats);
        Close(connfd);
        pthread_mutex_lock(&mutex_1);
        sumOfProcess--;
        pthread_cond_signal(&c);
        if (sumOfProcess == 0)
        {
            pthread_cond_signal(&cond_flush);
        }
        pthread_mutex_unlock(&mutex_1);
    }
}

/*
 * Listens for requests and takes care of them according to a given algorithm:
*/
int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_size, queue_size;
    struct sockaddr_in clientaddr;
    getargs(&port, &threads_size, &queue_size, argc, argv);
    Queue waiting = QueueCreate(queue_size);
    pthread_t threads[threads_size];
    threads_stats stats[threads_size];
    struct Args queues[threads_size];
    //Creates threads according to the given size:
    for (int i = 0; i < threads_size; i++)
    {
        threads_stats stat = calloc(1, sizeof(struct Threads_stats));
        if (!stat)
        {
            QueueDestroy(waiting);
            app_error("Allocation Error\n");
        }
        stats[i] = stat;
        stats[i]->id = i;
        stats[i]->stat_req = 0;
        stats[i]->dynm_req = 0;
        stats[i]->total_req = 0;
        queues[i].waiting = waiting;
        queues[i].stats = stats[i];
        int err = pthread_create(&threads[i], NULL, ThreadsHandle, (void *)&queues[i]);
        if (err != 0)
        {
            QueueDestroy(waiting);
            posix_error(err, "pthread_create error");
        }
    }

    listenfd = Open_listenfd(port);
    int isFull = 0;

    //Listens for and accepts requests:
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        struct timeval arrival;
        if (gettimeofday(&arrival, NULL))
        {
            QueueDestroy(waiting);
            unix_error("Get Time of Day Error\n");
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
                if (queue_size <= threads_size)
                {
                    Close(connfd);
                    isFull = 1;
                    pthread_mutex_unlock(&mutex_1);
                    break;
                }
                int toClose = dequeue(waiting, NULL);
                if (toClose != -1)
                {
                    sumOfProcess--;
                    Close(toClose);
                }
            }
            if (!strcmp(argv[4], "bf"))
            {
                pthread_cond_wait(&cond_flush, &mutex_1);
            }
            if (!strcmp(argv[4], "random"))
            {
                if (queue_size <= threads_size)
                {
                    Close(connfd);
                    pthread_mutex_unlock(&mutex_1);
                    isFull = 1;
                    break;
                }
                sumOfProcess -= dequeueHalfRandom(waiting);
            }
        }

        pthread_mutex_unlock(&mutex_1);

        if (!isFull)
        {
            sumOfProcess++;
            enqueue(waiting, connfd, arrival);
        }
    }
}
