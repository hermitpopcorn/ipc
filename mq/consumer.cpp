#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <atomic>
#include "mq.h"

static std::atomic<bool> running(true);

void sigint_handler(int signal) {
    std::cout << "\nReceived signal " << signal << std::endl;
    running = false;
}

int main(int, char**) {
    // Add Ctrl+C handler
    signal(SIGINT, sigint_handler);

    // Set MQ flags
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = sizeof(Message);
    attr.mq_curmsgs = 0; // Ignored but seems to be good practice to clear this first

    // Open MQ with read flags (create if not exists)
    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0666, &attr);
    if (mq == (mqd_t) -1) {
        perror("mq_open failed");
        return 1;
    }

    std::cout << "Waiting for messages..." << std::endl;

    // Read loop (polling)
    while (running) {
        Message msg;
        unsigned int priority;

        // Receive message (blocking, but timed to 1s timeout)
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 1;
        ssize_t bytes_read = mq_timedreceive(mq, (char *)&msg, sizeof(Message), &priority, &timeout);

        if (bytes_read == -1) {
            if (running && errno != ETIMEDOUT) {
                perror("mq_receive failed");
            }
        }

        if (bytes_read > 0) {
            std::cout << "Received message:" << std::endl;
            std::cout << "  - Sender ID: " << msg.sender_id << std::endl;
            std::cout << "  - Contents: " << msg.text << std::endl;
            std::cout << "  - Priority: " << priority << std::endl;
            std::cout << "---" << std::endl;
        }
    }

    // Close queue access
    if (mq_close(mq) == -1) {
        perror("mq_close failed");
        return 1;
    }

    return 0;
}
