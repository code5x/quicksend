#include "send_args.h"
#include "send_menu.h"
#include "send_transfer.h"

#include "../common/protocol.h"
#include "../common/colors.h"
#include "../common/log.h"
#include "../common/net.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <pthread.h>

#define MAX_IP_LEN 64
#define MAX_THREADS 50

/* ================= SHARED STATE ================= */

static char found_ip[MAX_IP_LEN] = {0};
static int peer_found = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* ================= TASK ================= */

typedef struct {
    char ip[MAX_IP_LEN];
} PingTask;

/* ================= PING WORKER ================= */

static void *ping_worker(void *arg) {
    PingTask *task = (PingTask *)arg;

    pthread_mutex_lock(&lock);
    if (peer_found) {
        pthread_mutex_unlock(&lock);
        free(task);
        return NULL;
    }
    pthread_mutex_unlock(&lock);

    char cmd[128];
    snprintf(cmd, sizeof(cmd),
             "ping -c 1 -W 1 %s > /dev/null 2>&1", task->ip);

    if (system(cmd) == 0) {
        pthread_mutex_lock(&lock);
        if (!peer_found) {
            strncpy(found_ip, task->ip, MAX_IP_LEN - 1);
            peer_found = 1;
        }
        pthread_mutex_unlock(&lock);
    }

    free(task);
    return NULL;
}

/* ================= DETECT PEER IP ================= */

static int detect_peer_ip(char *ip) {
    struct ifaddrs *ifap = NULL, *ifa = NULL;
    char local_ip[MAX_IP_LEN] = {0};

    peer_found = 0;
    found_ip[0] = '\0';

    if (getifaddrs(&ifap) != 0)
        return -1;

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET &&
            strcmp(ifa->ifa_name, "lo") != 0) {

            struct sockaddr_in *sa =
                (struct sockaddr_in *)ifa->ifa_addr;

            const char *addr = inet_ntoa(sa->sin_addr);
            if (strncmp(addr, "127.", 4) != 0) {
                strncpy(local_ip, addr, MAX_IP_LEN - 1);
                break;
            }
        }
    }

    freeifaddrs(ifap);

    if (!local_ip[0])
        return -1;

    char *dot = strrchr(local_ip, '.');
    if (!dot)
        return -1;

    *dot = '\0';
    char subnet[32];
    snprintf(subnet, sizeof(subnet), "%s", local_ip);
    *dot = '.';

    printf("Scanning subnet %s.1 - 254\n", subnet);

    pthread_t threads[MAX_THREADS];
    int tcount = 0;

    for (int i = 1; i <= 254 && !peer_found; i++) {
        char target[MAX_IP_LEN];
        snprintf(target, sizeof(target), "%s.%d", subnet, i);

        if (!strcmp(target, local_ip))
            continue;

        PingTask *task = malloc(sizeof(PingTask));
        if (!task)
            continue;

        strncpy(task->ip, target, MAX_IP_LEN - 1);

        pthread_create(&threads[tcount++], NULL, ping_worker, task);

        if (tcount >= MAX_THREADS) {
            for (int j = 0; j < tcount; j++)
                pthread_join(threads[j], NULL);
            tcount = 0;
        }
    }

    for (int j = 0; j < tcount; j++)
        pthread_join(threads[j], NULL);

    if (peer_found) {
        strncpy(ip, found_ip, MAX_IP_LEN - 1);
        return 0;
    }

    return -1;
}

/* ================= IP CHOICE ================= */

static void choose_ip(char *ip) {
    int choice;

    printf("Select IP method:\n");
    printf("1. Auto detect IP\n");
    printf("2. Manually enter IP\n");
    printf("Choose: ");
    scanf("%d", &choice);

    if (choice == 1) {
        if (detect_peer_ip(ip) != 0) {
            printf("Auto detect failed.\n");
            printf("Enter receiver IP: ");
            scanf("%63s", ip);
        } else {
            printf("Detected receiver IP: %s\n", ip);
        }
    } else if (choice == 2) {
        printf("Enter receiver IP: ");
        scanf("%63s", ip);
    } else {
        printf("Invalid choice.\n");
        exit(1);
    }
}

/* ================= MAIN ================= */

int main(int argc, char *argv[]) {
    send_args_t args;
    parse_send_args(argc, argv, &args);

    log_init();

    printf("Turn on Wifi and connect to receiver.\n");
    sleep(2);

    if (args.auto_ip) {
        if (detect_peer_ip(args.ip) != 0) {
            printf("Auto detect failed.\n");
            printf("Enter receiver IP: ");
            scanf("%63s", args.ip);
        } else {
            printf("Detected receiver IP: %s\n", args.ip);
        }
    } else if (!args.ip[0]) {
        choose_ip(args.ip);
    }

    int sock = connect_receiver(args.ip);
    if (sock < 0) {
        printf("Connection failed.\n");
        return 1;
    }

    if (args.file_count > 0) {
        for (int i = 0; i < args.file_count; i++)
            send_file(sock, args.files[i]);
    }

    if (args.dir_input)
        handle_send_folder_filtered(sock, args.dir_input);

    if (args.zip_folder)
        handle_send_folder_zip(sock, args.zip_folder);

    if (args.message)
        send_message(sock, args.message);

    if (args.file_count == 0 &&
        !args.message &&
        !args.zip_folder &&
        !args.dir_input) {

        menu_loop(sock);
    } else {
        char more;
        printf("\nDo you want to send more? (y/n): ");
        scanf(" %c", &more);

        if (more == 'y' || more == 'Y')
            menu_loop(sock);
        else {
            uint8_t e = MSG_EXIT;
            send_all(sock, &e, 1);
        }
    }

    close(sock);
    log_close();
    printf(CLR_RESET "Connection closed\n\n");
    return 0;
}
