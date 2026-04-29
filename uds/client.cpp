#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "message.hpp"

std::atomic<bool> running(true);

void message_receiver_thread_func(int sock) {
    while (running) {
        Message msg;
        ssize_t n = recv(sock, &msg, sizeof(msg), 0);

        if (n <= 0) {
            std::cout << "\nConnection closed by hub" << std::endl;
            running = false;
            break;
        }

        std::cout << "\n[Message received from " << msg.sender << "]: " << msg.data << std::endl;
    }
}

int run_client(const char* client_name, const char* other_client) {
    std::cout << client_name << " starting..." << std::endl;

    // Connect to hub
    int sock = -1;
    {
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("socket");
            return 1;
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, HUB_SOCKET);

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("connect");
            return 1;
        }
    }

    // Identify
    {
        if (send(sock, client_name, strlen(client_name), 0) < 0) {
            perror("send");
            return 1;
        }

        std::cout << "Connected to hub as " << client_name << std::endl;
        std::cout << "Type a message and press Enter to send to " << other_client << std::endl;
    }

    // Start receiver thread
    std::thread receiver(message_receiver_thread_func, sock);

    // Event loop
    while (running) {
        char buffer[MAX_MSG_SIZE];
        if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
            break;
        }

        // Remove newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (strlen(buffer) > 0) {
            Message msg;
            strcpy(msg.sender, client_name);
            strcpy(msg.recipient, other_client);
            strcpy(msg.data, buffer);

            if (send(sock, &msg, sizeof(msg), 0) < 0) {
                perror("send");
                break;
            }
        }
    }

    running = false;
    receiver.join();
    close(sock);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <client_name> <destination_client_name>" << std::endl;
        return 1;
    }

    return run_client(argv[1], argv[2]);
}
