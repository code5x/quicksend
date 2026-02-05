#include "send_args.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void invalid_args(void) {
    printf("Invalid arguments.\n");
    print_send_help();
    exit(1);
}

void print_send_help(void) {
    printf("\nUsage:\n");
    printf(" send [options]\n\n");

    printf("Options:\n");
    printf(" -i, --ip <ip> | auto     Receiver IP address\n");
    printf(" -f, --file <file...>     Send file(s)\n");
    printf(" -z, --zip <folder>       Send folder as zip\n");
    printf(" -d, --directory <path>   Send all files in folder\n");
    printf("                          You can use filter like /path/ *.ext\n");
    printf(" -m, --message <text>     Send text message\n");
    printf(" -h, --help               Show this help\n\n");

    printf("Examples:\n");
    printf(" send -i 192.168.1.5 -f /path/to/document.pdf\n");
    printf(" send -f /path/to/video.mp4\n");
    printf(" send -i auto -f /path/to/file1.ext /path/to/file2.ext\n");
    printf(" send -z /path/to/folder\n");
    printf(" send -i auto -d \"/path/to/folder/ *.ext\"\n");
    printf(" send -i 192.168.1.5 -m \"Hello\"\n");
    printf(" send\n\n");
}

void parse_send_args(int argc, char *argv[], send_args_t *args) {
    memset(args, 0, sizeof(*args));

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_send_help();
            exit(0);
        } else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--ip")) {
            if (++i >= argc)
                invalid_args();

            if (!strcmp(argv[i], "auto")) {
                args->auto_ip = 1;
            } else {
                strncpy(args->ip, argv[i], sizeof(args->ip) - 1);
            }
        } else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")) {
            if (i + 1 >= argc)
                invalid_args();

            while (i + 1 < argc && argv[i + 1][0] != '-') {
                args->files[args->file_count++] = argv[++i];
            }
        } else if (!strcmp(argv[i], "-z") || !strcmp(argv[i], "--zip")) {
            if (++i >= argc)
                invalid_args();
            args->zip_folder = argv[i];
        } else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--directory")) {
            if (++i >= argc)
                invalid_args();
            args->dir_input = argv[i];
        } else if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--message")) {
            if (++i >= argc)
                invalid_args();
            args->message = argv[i];
        } else {
            invalid_args();
        }
    }
}
