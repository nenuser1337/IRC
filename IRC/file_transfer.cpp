#include "ServerPoll.hpp"




void ServerPoll::handleFileTransfer(int client_fd, const std::string& file_name, long file_size) {
    char buffer[BUFSIZ];
    std::ofstream file(file_name, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << file_name << std::endl;
        return;
    }

    long bytes_received = 0;
    while (bytes_received < file_size) {
        int bytes_read = recv(client_fd, buffer, BUFSIZ, 0);
        if (bytes_read <= 0) {
            // Handle closing or error
            break;
        }
        file.write(buffer, bytes_read);
        bytes_received += bytes_read;
    }
}
bool ServerPoll::isFileTransfer(const std::string& message) {
    return message.rfind("FILE:", 0) == 0; // checks if "FILE:" is at the start
}

void ServerPoll::parseFileInfo(const std::string& header, std::string& file_name, long& file_size) {
    std::stringstream ss(header);
    std::string prefix;
    std::getline(ss, prefix, ':');  // Read the prefix "FILE"
    std::getline(ss, file_name, ':'); // Read the filename
    ss >> file_size; // Read the filesize
}
