#ifndef SEND_ARGS_H
#define SEND_ARGS_H

#include <stddef.h>

/* parsed arguments */
typedef struct {
    int auto_ip;
    char ip[64];

    char *files[64];
    int file_count;

    char *message;
    char *zip_folder;
    char *dir_input;
} send_args_t;

/* parse command line */
void parse_send_args(int argc, char *argv[], send_args_t *args);

/* print help */
void print_send_help(void);

#endif
