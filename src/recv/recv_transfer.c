#include "recv_transfer.h"

#include "../common/protocol.h"
#include "../common/colors.h"
#include "../common/net.h"
#include "../common/utils.h"
#include "../common/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>

/* ================= FILE EXIST ================= */

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* ================= UNIQUE PATH ================= */

static void make_unique_path(char *out,
                             size_t out_size,
                             const char *dir,
                             const char *filename) {

    char name[PATH_MAX];
    char ext[PATH_MAX] = "";

    char *dot = strrchr(filename, '.');
    if (dot) {
        size_t base_len = dot - filename;
        strncpy(name, filename, base_len);
        name[base_len] = 0;
        strncpy(ext, dot, sizeof(ext) - 1);
    } else {
        strncpy(name, filename, sizeof(name) - 1);
    }

    snprintf(out, out_size, "%s/%s%s", dir, name, ext);

    if (!file_exists(out))
        return;

    int i = 1;
    while (1) {
        snprintf(out, out_size, "%s/%s(%d)%s", dir, name, i, ext);
        if (!file_exists(out))
            return;
        i++;
    }
}

/* ================= RECEIVE FILE DATA ================= */

static int recv_file_data(int sock,
                          const char *save_path,
                          uint64_t filesize,
                          const char *log_name) {

    FILE *fp = fopen(save_path, "wb");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    char buf[BUF_SIZE];
    uint64_t received = 0;
    struct timeval start;
    gettimeofday(&start, NULL);

    while (received < filesize) {
        size_t to_read = BUF_SIZE;
        if (filesize - received < BUF_SIZE)
            to_read = filesize - received;

        ssize_t n = recv(sock, buf, to_read, 0);
        if (n <= 0) {
            fclose(fp);
            return -1;
        }

        fwrite(buf, 1, n, fp);
        received += n;

        /* progress */
        struct timeval now;
        gettimeofday(&now, NULL);

        double elapsed =
            (now.tv_sec - start.tv_sec) +
            (now.tv_usec - start.tv_usec) / 1000000.0;

        double speed = elapsed > 0 ? received / elapsed : 0;
        double eta = speed > 0 ? (filesize - received) / speed : 0;

        char rbuf[32], tbuf[32], sbuf[32];
        human_size(received, rbuf, sizeof(rbuf));
        human_size(filesize, tbuf, sizeof(tbuf));
        human_size(speed, sbuf, sizeof(sbuf));

        int percent = (int)((received * 100) / filesize);

        printf(CLR_PROGRESS
               "\rReceiving %3d%% | %s / %s | %s/s | ETA %02d:%02d",
               percent,
               rbuf,
               tbuf,
               sbuf,
               (int)(eta / 60),
               (int)((int)eta % 60));

        fflush(stdout);
    }

    fclose(fp);

    printf(CLR_DONE "\nFile received: %s\n" CLR_CONNECT, log_name);
    log_write_success("RECV", "file", filesize, log_name);

    return 0;
}

/* ================= HANDLE SINGLE FILE ================= */

int handle_single_file(int sock) {
    uint32_t name_len;

    if (recv_all(sock, &name_len, sizeof(name_len)) < 0)
        return -1;

    if (name_len >= PATH_MAX)
        return -1;

    char name[PATH_MAX];
    recv_all(sock, name, name_len);
    name[name_len] = 0;

    uint64_t size;
    recv_all(sock, &size, sizeof(size));

    char save_path[PATH_MAX];
    make_unique_path(save_path, sizeof(save_path), SAVE_DIR, name);

    printf("Receiving file: %s\n", name);
    return recv_file_data(sock, save_path, size, name);
}

/* ================= HANDLE MESSAGE ================= */

int handle_message(int sock) {
    uint32_t len;

    if (recv_all(sock, &len, sizeof(len)) < 0)
        return -1;

    if (len >= 4096)
        return -1;

    char msg[4096];
    recv_all(sock, msg, len);
    msg[len] = 0;

    printf(CLR_DONE
           "\nMessage received:\n---------------------------------------\n");
    printf("%s\n---------------------------------------\n" CLR_CONNECT, msg);

    log_write_success("RECV", "message", len, msg);
    return 0;
}
