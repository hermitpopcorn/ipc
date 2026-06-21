#pragma once

#define QUEUE_NAME "/ipc_demo_queue"
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256

struct Message {
    int sender_id;
    char text[MAX_MSG_SIZE];
};

