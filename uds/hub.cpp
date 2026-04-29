#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <map>
#include <vector>
#include <poll.h>
#include "message.hpp"

struct Client {
    int fd;
    char name[64];
    std::vector<Message> inbox;
};

int main() {
    std::cout << "Hub starting..." << std::endl;

    // Remove old socket file
    unlink(HUB_SOCKET);

    // Create server socket
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // Bind to socket
    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, HUB_SOCKET);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    // Listen for connections
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        return 1;
    }

    std::cout << "Hub listening on " << HUB_SOCKET << std::endl;

    std::map<std::string, Client> clients;
    std::vector<struct pollfd> poll_fds;

    // Add server socket to poll list
    struct pollfd server_pfd;
    server_pfd.fd = server_fd;
    server_pfd.events = POLLIN;
    poll_fds.push_back(server_pfd);

    while (true) {
        // Poll for activity
        int ret = poll(poll_fds.data(), poll_fds.size(), -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        // Check for new connections
        if (poll_fds[0].revents & POLLIN) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) {
                perror("accept");
                continue;
            }

            // Receive client name
            char name[64];
            ssize_t n = recv(client_fd, name, sizeof(name) - 1, 0);
            if (n <= 0) {
                close(client_fd);
                continue;
            }
            name[n] = '\0';

            std::cout << "Client connected: " << name << std::endl;

            Client new_client;
            new_client.fd = client_fd;
            strcpy(new_client.name, name);
            clients[name] = new_client;

            // Add to poll list
            struct pollfd pfd;
            pfd.fd = client_fd;
            pfd.events = POLLIN;
            poll_fds.push_back(pfd);
        }

        // Check for messages from clients
        for (size_t i = 1; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                Message msg;
                ssize_t n = recv(poll_fds[i].fd, &msg, sizeof(msg), 0);

                if (n <= 0) {
                    // Client disconnected
                    std::cout << "Client disconnected" << std::endl;
                    close(poll_fds[i].fd);

                    // Remove from clients map and poll list
                    for (auto it = clients.begin(); it != clients.end(); ++it) {
                        if (it->second.fd == poll_fds[i].fd) {
                            clients.erase(it);
                            break;
                        }
                    }
                    poll_fds.erase(poll_fds.begin() + i);
                    --i;
                } else {
                    // Forward message to recipient
                    if (clients.find(msg.recipient) != clients.end()) {
                        int recipient_fd = clients[msg.recipient].fd;
                        if (send(recipient_fd, &msg, sizeof(msg), 0) < 0) {
                            perror("send to recipient");
                        } else {
                            std::cout << "Forwarded message from " << msg.sender
                                      << " to " << msg.recipient << std::endl;
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    unlink(HUB_SOCKET);
    return 0;
}
