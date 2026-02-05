#ifndef SEND_TRANSFER_H
#define SEND_TRANSFER_H

#include <stdint.h>

/* connect */
int connect_receiver(const char *ip);

/* send operations */
int send_file(int sock, const char *path);
int send_message(int sock, const char *msg);

/* folder operations */
int handle_send_folder_zip(int sock, const char *folder_path);
int handle_send_folder_filtered(int sock, const char *input);

#endif
