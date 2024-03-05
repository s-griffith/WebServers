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


struct Args
{
    Queue waiting;
    Queue handled;
}

// HW3: Parse the new arguments too
void
getargs(int *port, int *threads, int *queue_size, int argc, char *argv[])
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
        //These two lines must happen together, otherwise can have situation where we have too many requests
        int connfd = dequeue(queues->waiting);
        enqueue(queues->handled, connfd);

        requestHandle(connfd);
        Close(connfd);
        dequeue(queues->handled); //does this need to in a critical section????
    }
    //How do we want to break this loop????????????????????????????????????????????????????
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_size, queue_size;
    struct sockaddr_in clientaddr;

    getargs(&port, &threads_size, &queue_size, argc, argv);
    // To add: arg[4] is the schedalg!!!!!!!!
    Queue waiting = QueueCreate(queue_size);
    Queue handled = QueueCreate(queue_size);
    struct Args queues;
    queues.waiting = waiting;
    queues.handled = handled;
    pthread_t threads[threads_size];
    for (int i = 0; i < threads_size; i++)
    {
        int err = pthread_create(&threads[i], NULL, ThreadsHandle(), (void *)&queues);
        if (err != 0)
        {
            posix_error(err, "pthread_create error");
        }
    }
    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);

        //Critical section:
        if (getSize(waiting) + getSize(handled) >= queue_size) {
            //*****Overload Handling Here*****
        }

        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        enqueue(waiting, connfd);

    }
}
