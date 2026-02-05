#include "log.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

#ifdef __ANDROID__
#define BASE_DIR "/sdcard/ftp_termux"
#else
#define BASE_DIR "/ftp_linux"
#endif

char SAVE_DIR[PATH_MAX];
static FILE *log_fp = NULL;

/* ================= LOGGING ================= */

void log_init(void) {
#ifdef __ANDROID__
    snprintf(SAVE_DIR, sizeof(SAVE_DIR), "%s", BASE_DIR);
#else
    const char *home = getenv("HOME");
    snprintf(SAVE_DIR, sizeof(SAVE_DIR), "%s%s", home, BASE_DIR);
#endif

    ensure_dir(SAVE_DIR);

    char log_path[PATH_MAX];
    snprintf(log_path, sizeof(log_path), "%s/quicksend.log", SAVE_DIR);

    struct stat st;
    int file_exists = (stat(log_path, &st) == 0);

    log_fp = fopen(log_path, "a");
    if (!log_fp) {
        perror("log open");
        exit(1);
    }

    if (!file_exists) {
        fprintf(log_fp,
                "================================================================================\n");
        fprintf(log_fp,
                "| MODE |  TYPE   |     SIZE      |      TIMESTAMP      |        CONTENT        |\n");
        fprintf(log_fp,
                "================================================================================\n");
        fflush(log_fp);
    }
}

void log_write_success(const char *mode,
                       const char *type,
                       double size_bytes,
                       const char *content) {
    if (!log_fp)
        return;

    char timebuf[64];
    char sizebuf[64];

    now_str(timebuf, sizeof(timebuf));
    human_size(size_bytes, sizebuf, sizeof(sizebuf));

    fprintf(log_fp,
            "| %-4s | %-7s | %-13s | %-19s | %-21s |\n",
            mode, type, sizebuf, timebuf, content);
    fprintf(log_fp,
            "--------------------------------------------------------------------------------\n");
    fflush(log_fp);
}

void log_close(void) {
    if (log_fp) {
        fclose(log_fp);
        log_fp = NULL;
    }
}
