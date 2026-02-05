#include "recv_server.h"

#include "../common/protocol.h"
#include "../common/colors.h"
#include "../common/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>

/* ================= LOCAL IP ================= */

static int get_local_ip(char *ip) {
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        return -1;

    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET &&
            strcmp(ifa->ifa_name, "lo") != 0) {

            struct sockaddr_in *sa =
                (struct sockaddr_in *)ifa->ifa_addr;

            strcpy(ip, inet_ntoa(sa->sin_addr));
            freeifaddrs(ifaddr);
            return 0;
        }
    }

    freeifaddrs(ifaddr);
    return -1;
}

/* ================= MAIN ================= */

int main(int argc, char *argv[]) {
    (void)argv;
    if (argc > 1) {
        printf("Usage: recv\n");
        return 1;
    }

    log_init();

    char ip[64];
    if (get_local_ip(ip) != 0) {
        printf("Could not detect local IP.\n");
        return 1;
    }

    printf("Your IP address is: %s\n", ip);
    printf("Turn on Wifi and connect to sender.\n");
    sleep(1);

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server);
        return 1;
    }

    if (listen(server, 1) < 0) {
        perror("listen");
        close(server);
        return 1;
    }

    printf("Waiting for sender...\n");

    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);

    int client = accept(server, (struct sockaddr *)&cli, &len);
    if (client < 0) {
        perror("accept");
        close(server);
        return 1;
    }

    printf(CLR_CONNECT "Sender connected\n");

    server_loop(client);

    close(client);
    close(server);
    log_close();

    printf(CLR_RESET "Connection closed\n\n");
    return 0;
}
