#include "recv_server.h"
#include "recv_transfer.h"

#include "../common/protocol.h"
#include "../common/colors.h"
#include "../common/net.h"

#include <stdio.h>

void server_loop(int client) {
    while (1) {
        printf("\nWaiting for incoming data...\n");

        uint8_t type;
        if (recv_all(client, &type, 1) < 0)
            break;

        if (type == MSG_EXIT) {
            printf(CLR_RESET "Sender closed connection.\n");
            break;

        } else if (type == MSG_SINGLE_FILE) {
            handle_single_file(client);

        } else if (type == MSG_TEXT_MESSAGE) {
            handle_message(client);
        }
    }
}
