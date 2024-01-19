#ifndef SERVERPOLL_HPP
#define SERVERPOLL_HPP

#include <iostream>
#include <vector>
#include <cstring>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

class ServerPoll {
public:
    ServerPoll();
    ~ServerPoll();
    void run();
    void readDataFromSocket(int index);
     // Bot-related functions
    bool isInsult(const std::string& message);
    bool isSpamming(int sender_fd, const std::string& message);
    void kickClient(int sender_fd);
    void warnClient(int sender_fd);
     void startFileTransfer(int sender_fd, const std::string& filename);
    void receiveFileData(int sender_fd, const char* data, size_t length);
    void endFileTransfer(int sender_fd);
    // File transfer methods
    void handleFileTransfer(int client_fd, const std::string& file_name, long file_size);
    bool isFileTransfer(const std::string& message);
    void parseFileInfo(const std::string& header, std::string& file_name, long& file_size);

private:
    int createServerSocket();
    void acceptNewConnection();
    void addToPollFds(int new_fd);
    void delFromPollFds(int index);
    
  

    static const int port = 4242;
    int serverSocket;
    std::vector<struct pollfd> pollFds;
    std::map<int, std::string> lastMessages;
    std::map<int, int> messageCounts;
    //  std::map<int, std::ofstream> fileStreams;
};

#endif // SERVERPOLL_HPP
