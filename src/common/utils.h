#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* directory */
void ensure_dir(const char *path);

/* time */
void now_str(char *buf, size_t sz);

/* size */
void human_size(double bytes, char *out, size_t sz);

#endif
