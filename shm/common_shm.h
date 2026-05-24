#ifndef COMMON_SHM_H
#define COMMON_SHM_H

#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string>
#include "shm_data.h"

template<typename LockFunc, typename UnlockFunc>
void run_common_process(SharedData* data, const std::string& role,
                  LockFunc lock_fn, UnlockFunc unlock_fn) {
    std::cout << "Type a message and press Enter to send (or 'quit' to exit):" << std::endl;

    int last_counter = -1;
    bool should_quit = false;

    // Make stdin non-blocking so we can check for user input and messages at the same time
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    while (!should_quit) {
        // Check for user input
        char input[MAX_DATA_SIZE];
        if (fgets(input, sizeof(input), stdin) != nullptr) {
            std::string str(input);

            if (str.find("quit") != std::string::npos) {
                should_quit = true;
            } else if (str.length() > 1) {
                // Remove newline if present
                size_t len = str.length();
                if (str[len-1] == '\n') {
                    str[len-1] = '\0';
                }

                lock_fn();
                {
                    strcpy(data->message, str.c_str());
                    strcpy(data->sender, role.c_str());
                    data->message_counter++;
                    std::cout << role << ": Sent message #" << data->message_counter << std::endl;
                }
                unlock_fn();
            }
        }

        // Check for incoming messages
        lock_fn();
        {
            if (data->message_counter > last_counter) {
                std::cout << "[" << data->sender << "]: " << data->message << std::endl;
                last_counter = data->message_counter;
            }
        }
        unlock_fn();

        // Sleep 100ms
        usleep(100000);
    }
}

#endif
