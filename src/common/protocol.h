#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

/* ================= PROTOCOL ================= */

#define MSG_SINGLE_FILE      0x01
#define MSG_MULTI_FILE       0x02
#define MSG_FOLDER_ZIP       0x03
#define MSG_FOLDER_FILTER    0x04
#define MSG_TEXT_MESSAGE     0x05
#define MSG_EXIT             0x7F

#define PORT 8080
#define BUF_SIZE 8192

#endif
