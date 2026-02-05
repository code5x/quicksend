#ifndef LOG_H
#define LOG_H

#include <stdio.h>

/* init */
void log_init(void);

/* write success entry */
void log_write_success(const char *mode,
                       const char *type,
                       double size_bytes,
                       const char *content);

/* close */
void log_close(void);

/* shared save dir */
extern char SAVE_DIR[];

#endif
