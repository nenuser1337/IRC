#include "ServerPoll.hpp"

ServerPoll::ServerPoll() : serverSocket(-1) {
    serverSocket = createServerSocket();
    if (serverSocket != -1) {
        struct pollfd initialPollFd;
        initialPollFd.fd = serverSocket;
        initialPollFd.events = POLLIN;
        pollFds.push_back(initialPollFd);
    }
}

ServerPoll::~ServerPoll() {
    if (serverSocket != -1) {
        close(serverSocket);
    }
}

void ServerPoll::run() {
       std::cout << "---- SERVER ----\n\n";

    if (serverSocket == -1) {
        std::cerr << "[Server] Server socket initialization failed.\n";
        return;
    }

    std::cout << "---- SERVER ----\n\n";
    std::cout << "[Server] Listening on port " << port << "\n";

    while (true) {
        int status = poll(&pollFds[0], pollFds.size(), 2000); // 2 seconds timeout

        if (status < 0) {
            std::cerr << "[Server] Poll error: " << strerror(errno) << std::endl;
            break;
        } else if (status == 0) {
            std::cout << "[Server] Waiting..." << std::endl;
            continue;
        }

        for (size_t i = 0; i < pollFds.size(); ++i) {
            if (pollFds[i].revents & POLLIN) {
                if (pollFds[i].fd == serverSocket) {
                    std::cout << "[" << serverSocket << "] Ready for I/O operation" << std::endl;
                    acceptNewConnection();
                } else {
                    std::cout << "[" << pollFds[i].fd << "] Ready for I/O operation" << std::endl;
                    readDataFromSocket(i);
                }
            }
        }
    }
}


int ServerPoll::createServerSocket() {
    int socket_fd;
    struct sockaddr_in sa;

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any address
    sa.sin_port = htons(port);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        std::cerr << "[Server] Socket error: " << strerror(errno) << std::endl;
        return -1;
    }
     std::cout << "[Server] Created server socket fd: " << socket_fd << std::endl;
    if (bind(socket_fd, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa)) == -1) {
        std::cerr << "[Server] Bind error: " << strerror(errno) << std::endl;
        close(socket_fd);
        return -1;
    }
    std::cout << "[Server] Bound socket to localhost port " << port << std::endl;
    if (listen(socket_fd, 10) == -1) {
        std::cerr << "[Server] Listen error: " << strerror(errno) << std::endl;
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

void ServerPoll::acceptNewConnection() {
    int client_fd;
    char msg_to_send[1024]; // Assuming BUFSIZ is 1024 for this context

    // Accept a new connection
    client_fd = accept(serverSocket, NULL, NULL);
    if (client_fd == -1) {
        std::cerr << "[Server] Accept error: " << strerror(errno) << std::endl;
        return;
    }

    // Check if we need to resize the vector (this would be handled automatically in C++)
    if (pollFds.size() == pollFds.capacity()) {
        pollFds.reserve(pollFds.size() * 2); // Double the capacity
    }

    // Add the new file descriptor to the vector
    struct pollfd newPollFd = {client_fd, POLLIN, 0};
    pollFds.push_back(newPollFd);

    std::cout << "[Server] Accepted new connection on client socket " << client_fd << "." << std::endl;

    // Prepare the welcome message
    std::snprintf(msg_to_send, sizeof(msg_to_send), "Welcome. You are client fd [%d]\n", client_fd);

    // Send the welcome message to the client
    if (send(client_fd, msg_to_send, std::strlen(msg_to_send), 0) == -1) {
        std::cerr << "[Server] Send error to client " << client_fd << ": " << strerror(errno) << std::endl;
    }
}


void ServerPoll::readDataFromSocket(int index) {
    int sender_fd = pollFds[index].fd;
    char buffer[1024];

    int bytes_read = recv(sender_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        // Handle disconnection
        if (bytes_read == 0) {
            std::cout << "[" << sender_fd << "] Client socket closed connection." << std::endl;
        } else {
            std::cerr << "[Server] Recv error: " << strerror(errno) << std::endl;
        }
        close(sender_fd);
        delFromPollFds(index);
    } else {
        buffer[bytes_read] = '\0'; // Ensure null termination

        std::string message(buffer);

        // Check if the message signals file transfer
        if (isFileTransfer(message)) {
            // Extract file information and handle file transfer
            std::string file_name;
            long file_size;
            parseFileInfo(message, file_name, file_size);
            handleFileTransfer(sender_fd, file_name, file_size);
        } else {
            // Bot functionality
            if (isInsult(message)) {
                kickClient(sender_fd);
                return; // Client kicked, stop processing
            }

            if (isSpamming(sender_fd, message)) {
                warnClient(sender_fd);
                // Optionally reset the count after warning
                messageCounts[sender_fd] = 0;
            } else {
                // Normal message processing
                char msg_to_send[1024];
                int msg_length = snprintf(msg_to_send, sizeof(msg_to_send), "[%d] says: %s", sender_fd, buffer);
                if (msg_length < 0 || msg_length >= static_cast<int>(sizeof(msg_to_send))) {
                    std::cerr << "[Server] Error formatting outgoing message." << std::endl;
                    return;
                }

                // Broadcasting message to other clients
                for (size_t i = 0; i < pollFds.size(); ++i) {
                    int dest_fd = pollFds[i].fd;
                    if (dest_fd != serverSocket && dest_fd != sender_fd) {
                        if (send(dest_fd, msg_to_send, msg_length, 0) == -1) {
                            std::cerr << "[Server] Send error to client fd " << dest_fd << ": " << strerror(errno) << std::endl;
                        }
                    }
                }
            }
        }
    }
}




void ServerPoll::addToPollFds(int newFd) {
    struct pollfd newPollFd;
    newPollFd.fd = newFd;
    newPollFd.events = POLLIN;
    pollFds.push_back(newPollFd);
}

void ServerPoll::delFromPollFds(int index) {
    if (index >= 0 && index < static_cast<int>(pollFds.size())) {
        pollFds.erase(pollFds.begin() + index);
    }
}

// The main.cpp file would create an instance of ServerPoll and call its run method.
