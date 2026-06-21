#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <cstdlib>
#include "mq.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <sender_id> <message> [priority]" << std::endl;
        std::cerr << "Example: " << argv[0] << " 1 \"Hello World\" 5" << std::endl;
        return 1;
    }

    int sender_id = std::atoi(argv[1]);
    const char *msg_text = argv[2];
    unsigned int priority = (argc > 3) ? std::atoi(argv[3]) : 0;

    // Set MQ flags
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = sizeof(Message);
    attr.mq_curmsgs = 0; // Ignored but seems to be good practice to clear this first

    // Open MQ with write flags (create if not exists)
    // Using O_NONBLOCK will cause the producer to fail instead of waiting for the queue to free up
    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        return 1;
    }

    // Create message
    Message msg;
    msg.sender_id = sender_id;
    strncpy(msg.text, msg_text, MAX_MSG_SIZE - 1);
    msg.text[MAX_MSG_SIZE - 1] = '\0';

    // Send message
    if (mq_send(mq, (const char *)&msg, sizeof(Message), priority) == -1) {
        perror("mq_send failed");
        mq_close(mq);
        return 1;
    }

    std::cout << "Message sent" << std::endl;

    // Close queue access
    if (mq_close(mq) == -1) {
        perror("mq_close failed");
        return 1;
    }

    return 0;
}
