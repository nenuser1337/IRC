#include "ServerPoll.hpp"






bool ServerPoll::isInsult(const std::string& message) {
    // Check if the message contains the specific insult
    return message.find("retard") != std::string::npos;
}

bool ServerPoll::isSpamming(int sender_fd, const std::string& message) {
    // Check if the client is sending the same message repeatedly
    if (lastMessages[sender_fd] == message) {
        messageCounts[sender_fd]++;
    } else {
        lastMessages[sender_fd] = message;
        messageCounts[sender_fd] = 1;
    }

    return messageCounts[sender_fd] == 4;
}

void ServerPoll::kickClient(int sender_fd) {
    // Inform the client they are being kicked
    const char* kick_msg = "You have been kicked for misconduct.\n";
    send(sender_fd, kick_msg, strlen(kick_msg), 0);
    
    // Close the client socket and remove from the poll fds
    close(sender_fd);
    // You'll need to find the index for sender_fd to remove it correctly
    for (size_t i = 0; i < pollFds.size(); ++i) {
        if (pollFds[i].fd == sender_fd) {
            delFromPollFds(i);
            break;
        }
    }
    
    // Clear spam tracking for the client
    lastMessages.erase(sender_fd);
    messageCounts.erase(sender_fd);
}

void ServerPoll::warnClient(int sender_fd) {
    // Inform the client they are being warned for spamming
    const char* warn_msg = "Please do not spam. This is a warning.\n";
    send(sender_fd, warn_msg, strlen(warn_msg), 0);
    // Reset the count
    messageCounts[sender_fd] = 0;
}