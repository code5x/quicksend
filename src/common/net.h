#ifndef NET_H
#define NET_H

#include <stddef.h>

/* send all bytes */
int send_all(int sock, const void *buf, size_t len);

/* receive all bytes */
int recv_all(int sock, void *buf, size_t len);

#endif
