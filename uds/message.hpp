#pragma once

#define HUB_SOCKET "/tmp/uds_hub.sock"
#define MAX_MSG_SIZE 1024

struct Message {
    char sender[64];
    char recipient[64];
    char data[MAX_MSG_SIZE];
};
