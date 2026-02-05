#include "send_transfer.h"

#include "../common/protocol.h"
#include "../common/colors.h"
#include "../common/net.h"
#include "../common/utils.h"
#include "../common/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <limits.h>

/* temp file for folder filter */
static char TEMP_FOLDER_FILE[PATH_MAX];

/* ================= CONNECT ================= */

int connect_receiver(const char *ip) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return -1;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }

    printf(CLR_CONNECT "Receiver connected: %s:%d\n", ip, PORT);
    return sock;
}

/* ================= FILE SEND ================= */

int send_file(int sock, const char *path) {
    struct stat st;

    if (stat(path, &st) < 0) {
        printf("Invalid file path\n");
        return -1;
    }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    uint8_t type = MSG_SINGLE_FILE;
    uint32_t name_len = strlen(name);

    if (send_all(sock, &type, 1) < 0) goto fail;
    send_all(sock, &name_len, sizeof(name_len));
    send_all(sock, name, name_len);
    send_all(sock, &st.st_size, sizeof(uint64_t));

    char buf[BUF_SIZE];
    uint64_t sent = 0;
    struct timeval start;
    gettimeofday(&start, NULL);

    while (!feof(fp)) {
        size_t n = fread(buf, 1, sizeof(buf), fp);
        if (n == 0) break;
        if (send_all(sock, buf, n) < 0) goto fail;
        sent += n;

        /* progress */
        struct timeval now;
        gettimeofday(&now, NULL);

        double elapsed =
            (now.tv_sec - start.tv_sec) +
            (now.tv_usec - start.tv_usec) / 1000000.0;

        double speed = elapsed > 0 ? sent / elapsed : 0;
        double eta = speed > 0 ? (st.st_size - sent) / speed : 0;

        char sbuf[32], tbuf[32], spbuf[32];
        human_size(sent, sbuf, sizeof(sbuf));
        human_size(st.st_size, tbuf, sizeof(tbuf));
        human_size(speed, spbuf, sizeof(spbuf));

        int percent = (int)((sent * 100) / st.st_size);

        printf(CLR_PROGRESS
               "\rSending %3d%% | %s / %s | %s/s | ETA %02d:%02d",
               percent,
               sbuf,
               tbuf,
               spbuf,
               (int)(eta / 60),
               (int)((int)eta % 60));

        fflush(stdout);
    }

    printf(CLR_DONE "\nSend file: %s\n" CLR_CONNECT, name);
    log_write_success("SEND", "file", st.st_size, name);

    fclose(fp);
    return 0;

fail:
    fclose(fp);
    return -1;
}

/* ================= MESSAGE SEND ================= */

int send_message(int sock, const char *msg) {
    uint8_t type = MSG_TEXT_MESSAGE;
    uint32_t len = strlen(msg);

    if (send_all(sock, &type, 1) < 0)
        return -1;

    send_all(sock, &len, sizeof(len));
    send_all(sock, msg, len);

    printf(CLR_DONE "\nSend message: %s\n" CLR_CONNECT, msg);
    log_write_success("SEND", "message", len, msg);
    return 0;
}

/* ================= ZIP ================= */

static int create_zip(const char *folder, char *zip_out) {
    struct stat st;
    if (stat(folder, &st) != 0 || !S_ISDIR(st.st_mode)) {
        printf("Folder does not exist: %s\n", folder);
        return -1;
    }

    char timebuf[32];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(timebuf, sizeof(timebuf), "%Y%m%d_%H%M%S", tm);

    const char *base = strrchr(folder, '/');
    base = base ? base + 1 : folder;

    snprintf(zip_out, PATH_MAX, "%s_%s.zip", base, timebuf);

    char cmd[PATH_MAX * 2];
    snprintf(cmd, sizeof(cmd),
             "zip -r \"%s\" \"%s\"",
             zip_out, folder);

    return system(cmd);
}

int handle_send_folder_zip(int sock, const char *folder_path) {
    char zip[PATH_MAX];

    if (create_zip(folder_path, zip) != 0) {
        printf("Failed to create zip\n");
        return -1;
    }

    if (send_file(sock, zip) != 0) {
        remove(zip);
        return -1;
    }

    remove(zip);
    return 0;
}

/* ================= FOLDER FILTER ================= */

int handle_send_folder_filtered(int sock, const char *input) {
    char folder_path[512];
    char extension[32] = {0};
    int file_count = 0;

    snprintf(TEMP_FOLDER_FILE,
             sizeof(TEMP_FOLDER_FILE),
             "%s/.send_folder_list.txt",
             SAVE_DIR);

    char *last_slash = strrchr(input, '/');
    if (last_slash && strstr(last_slash, "*.")) {
        char *dot = strchr(last_slash, '.');
        if (dot)
            strcpy(extension, dot + 1);

        size_t len = last_slash - input;
        strncpy(folder_path, input, len);
        folder_path[len] = '\0';
    } else {
        strncpy(folder_path, input, sizeof(folder_path) - 1);
    }

    DIR *dir = opendir(folder_path);
    if (!dir) {
        perror("opendir failed");
        return -1;
    }

    FILE *out = fopen(TEMP_FOLDER_FILE, "w");
    if (!out) {
        perror("fopen TEMP_FOLDER_FILE failed");
        closedir(dir);
        return -1;
    }

    struct dirent *dp;
    struct stat st;
    char full[PATH_MAX];

    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;

        snprintf(full, sizeof(full), "%s/%s", folder_path, dp->d_name);
        if (stat(full, &st) != 0)
            continue;

        if (S_ISREG(st.st_mode)) {
            char *dot = strrchr(dp->d_name, '.');
            if (extension[0]) {
                if (dot && strcmp(dot + 1, extension) == 0) {
                    fprintf(out, "%s\n", full);
                    file_count++;
                }
            } else {
                fprintf(out, "%s\n", full);
                file_count++;
            }
        }
    }

    closedir(dir);
    fclose(out);

    if (file_count == 0) {
        fprintf(stderr, "No matching files found in folder\n");
        remove(TEMP_FOLDER_FILE);
        return -1;
    }

    FILE *in = fopen(TEMP_FOLDER_FILE, "r");
    if (!in)
        return -1;

    char line[PATH_MAX];
    while (fgets(line, sizeof(line), in)) {
        line[strcspn(line, "\n")] = 0;
        send_file(sock, line);
    }

    fclose(in);
    remove(TEMP_FOLDER_FILE);
    return 0;
}
