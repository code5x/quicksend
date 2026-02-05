#include "send_menu.h"
#include "send_transfer.h"

#include "../common/protocol.h"
#include "../common/net.h"

#include <stdio.h>
#include <stdlib.h>

void menu_loop(int sock) {
    while (1) {
        int choice;

        printf("\n1. Send single file\n");
        printf("2. Send multiple files\n");
        printf("3. Send folder (zip)\n");
        printf("4. Send folder files (filter)\n");
        printf("5. Send message\n");
        printf("Choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input\n");
            exit(1);
        }

        if (choice == 1) {
            char p[1024];
            printf("File path: ");
            scanf(" %[^\n]", p);
            send_file(sock, p);

        } else if (choice == 2) {
            int n;
            printf("File count: ");
            scanf("%d", &n);

            for (int i = 0; i < n; i++) {
                char p[1024];
                printf("File %d: ", i + 1);
                scanf(" %[^\n]", p);
                send_file(sock, p);
            }

        } else if (choice == 3) {
            char folder[1024];
            printf("Folder path: ");
            scanf(" %[^\n]", folder);
            handle_send_folder_zip(sock, folder);

        } else if (choice == 4) {
            char input[512];
            int c;
            while ((c = getchar()) != '\n' && c != EOF);

            printf("Enter folder path (you can also specify like /path/ *.txt): ");
            if (!fgets(input, sizeof(input), stdin))
                continue;

            input[strcspn(input, "\n")] = 0;
            handle_send_folder_filtered(sock, input);

        } else if (choice == 5) {
            char msg[2048];
            printf("Message: ");
            scanf(" %[^\n]", msg);
            send_message(sock, msg);

        } else {
            printf("Invalid input\n");
        }

        char more;
        printf("\nDo you want to send more? (y/n): ");
        scanf(" %c", &more);

        if (more == 'n' || more == 'N') {
            uint8_t e = MSG_EXIT;
            send_all(sock, &e, 1);
            break;
        }
    }
}
