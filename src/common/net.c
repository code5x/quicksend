#include "net.h"

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/* ================= SEND ALL ================= */

int send_all(int sock, const void *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(sock, (const char *)buf + sent, len - sent, 0);
        if (n <= 0) {
            printf("Error: Sending failed\n");
            return -1;
        }
        sent += n;
    }
    return 0;
}

/* ================= RECV ALL ================= */

int recv_all(int sock, void *buf, size_t len) {
    size_t recvd = 0;
    while (recvd < len) {
        ssize_t n = recv(sock, (char *)buf + recvd, len - recvd, 0);
        if (n <= 0) {
            printf("Error: Receiving failed\n");
            return -1;
        }
        recvd += n;
    }
    return 0;
}
