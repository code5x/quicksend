#ifndef RECV_TRANSFER_H
#define RECV_TRANSFER_H

#include <stdint.h>

/* handle incoming data */
int handle_single_file(int sock);
int handle_message(int sock);

#endif
