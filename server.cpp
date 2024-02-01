#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <stdarg.h>

#define SERVER_PORT 5208
#define BUF_SIZE 1024
#define MAX_CLNT 256   

// Function prototypes
void handle_clnt(int clnt_sock);
void send_msg(const std::string &msg);
int output(const char *arg,...);
int error_output(const char *arg,...);
void error_handling(const std::string &message);

// Global variables
int clnt_cnt = 0;  // Counter for connected clients
std::mutex mtx;    // Mutex for thread-safe operations

// Map to store client sockets with their associated names
std::unordered_map<std::string, int> clnt_socks;

int main(int argc, const char **argv, const char **envp) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

    // Create a socket for the server
    serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_sock == -1) {
        error_handling("socket() failed!");
    }
    
    // Initialize server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERVER_PORT);

    // Bind the socket to the server address
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("bind() failed!");
    }

    // Start listening for incoming connections
    if (listen(serv_sock, MAX_CLNT) == -1) {
        error_handling("listen() error!");
    }

    printf("The server is running on port %d\n", SERVER_PORT);

    // Main server loop to accept incoming connections
    while(1) {   
        clnt_addr_size = sizeof(clnt_addr);

        // Accept a new client connection
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) {
            error_handling("accept() failed!");
        }

        // Increment the client count and spawn a new thread to handle the client
        mtx.lock();
        clnt_cnt++;
        mtx.unlock();

        std::thread th(handle_clnt, clnt_sock);
        th.detach();

        output("Connected client IP: %s \n", inet_ntoa(clnt_addr.sin_addr));
    }

    // Close the server socket (this will never be reached in the current implementation)
    close(serv_sock);
    return 0;
}

// Function to handle individual client connections
void handle_clnt(int clnt_sock) {
    char msg[BUF_SIZE];
    int flag = 0;

    char tell_name[13] = "#new client:";

    // Receive and process messages from the client
    while(recv(clnt_sock, msg, sizeof(msg), 0) != 0) {
        // Check if the message indicates a new client joining
        if (std::strlen(msg) > std::strlen(tell_name)) {
            char pre_name[13];
            std::strncpy(pre_name, msg, 12);
            pre_name[12] = '\0';
            if (std::strcmp(pre_name, tell_name) == 0) {
                char name[20];
                std::strcpy(name, msg + 12);

                // Check if the client name already exists
                if (clnt_socks.find(name) == clnt_socks.end()) {
                    output("The name of socket %d: %s\n", clnt_sock, name);
                    clnt_socks[name] = clnt_sock;
                } else {
                    // Inform the client that the name already exists and disconnect
                    std::string error_msg = std::string(name) + " exists already. Please quit and enter with another name!";
                    send(clnt_sock, error_msg.c_str(), error_msg.length() + 1, 0);
                    mtx.lock();
                    clnt_cnt--;
                    mtx.unlock();
                    flag = 1;
                }
            }
        }

        // If the message is not a new client notification, broadcast the message to all clients
        if (flag == 0)
            send_msg(std::string(msg));
    }

    // Handle client disconnection
    if (flag == 0) {
        std::string leave_msg;
        std::string name;
        mtx.lock();
        // Find the disconnected client's name and remove it from the map
        for (auto it = clnt_socks.begin(); it != clnt_socks.end(); ++it) {
            if (it->second == clnt_sock) {
                name = it->first;
                clnt_socks.erase(it->first);
            }
        }
        clnt_cnt--;
        mtx.unlock();

        // Notify other clients about the disconnection
        leave_msg = "Client " + name + " leaves the chat room";
        send_msg(leave_msg);
        output("Client %s leaves the chat room\n", name.c_str());
        close(clnt_sock);
    } else {
        // Close the socket for the new client with a duplicate name
        close(clnt_sock);
    }
}

// Function to broadcast messages to all connected clients
void send_msg(const std::string &msg) {
    mtx.lock();

    // Check if the message is a private message directed to a specific client
    std::string pre = "@";
    int first_space = msg.find_first_of(" ");
    if (msg.compare(first_space + 1, 1, pre) == 0) {
        int space = msg.find_first_of(" ", first_space + 1);
        std::string receive_name = msg.substr(first_space + 2, space - first_space - 2);
        std::string send_name = msg.substr(1, first_space - 2);

        // Check if the recipient client exists in the map
        if (clnt_socks.find(receive_name) == clnt_socks.end()) {
            // Inform the sender that the recipient does not exist
            std::string error_msg = "[error] There is no client named " + receive_name;
            send(clnt_socks[send_name], error_msg.c_str(), error_msg.length() + 1, 0);
        } else {
            // Send the private message to the recipient and the sender
            send(clnt_socks[receive_name], msg.c_str(), msg.length() + 1, 0);
            send(clnt_socks[send_name], msg.c_str(), msg.length() + 1, 0);
        }
    } else {
        // Broadcast the message to all connected clients
        for (auto it = clnt_socks.begin(); it != clnt_socks.end(); it++) {
            send(it->second, msg.c_str(), msg.length() + 1, 0);
        }
    }

    mtx.unlock();
}

// Function for formatted output to stdout
int output(const char *arg, ...) {
    int res;
    va_list ap;
    va_start(ap, arg);
    res = vfprintf(stdout, arg, ap);
    va_end(ap);
    return res;
}

// Function for formatted output to stderr
int error_output(const char *arg, ...) {
    int res;
    va_list ap;
    va_start(ap, arg);
    res = vfprintf(stderr, arg, ap);
    va_end(ap);
    return res;
}

// Function to handle errors and terminate the program
void error_handling(const std::string &message) {
    std::cerr << message << std::endl;
    exit(1);
}
