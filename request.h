#ifndef __REQUEST_H__
#include "segel.h"

void requestHandle(int fd, struct timeval arrival, struct timeval dispatch, threads_stats t_stats);

#endif
