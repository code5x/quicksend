#include "utils.h"

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

/* ================= DIRECTORY ================= */

void ensure_dir(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        mkdir(path, 0755);
    }
}

/* ================= TIME ================= */

void now_str(char *buf, size_t sz) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, sz, "%d-%m-%Y %H:%M:%S", tm);
}

/* ================= SIZE UNIT ================= */

void human_size(double bytes, char *out, size_t sz) {
    const char *unit[] = {"Bytes", "KB", "MB", "GB"};
    int i = 0;
    while (bytes >= 1024 && i < 3) {
        bytes /= 1024;
        i++;
    }
    snprintf(out, sz, "%.2f %s", bytes, unit[i]);
}
