#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>

#define BUF_SIZE 1024
#define SERVER_PORT 5208 
#define IP "127.0.0.1"

// Function prototypes
void send_msg(int sock);
void recv_msg(int sock);
int output(const char *arg, ...);
int error_output(const char *arg, ...);
void error_handling(const std::string &message);

// Global variables
std::string name = "DEFAULT";
std::string msg;

int main(int argc, const char **argv, const char **envp) {
    int sock;
    struct sockaddr_in serv_addr;

    // Check command line arguments
    if (argc != 2) {
        error_output("Usage: %s <Name>\n", argv[0]);
        exit(1);
    }

    // Set the client's name from the command line argument
    name = "[" + std::string(argv[1]) + "]";

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        error_handling("socket() failed!");
    }

    // Initialize server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(SERVER_PORT);

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("connect() failed!");
    }

    // Inform the server about the new client
    std::string my_name = "#new client:" + std::string(argv[1]);
    send(sock, my_name.c_str(), my_name.length() + 1, 0);

    // Create threads for sending and receiving messages
    std::thread snd(send_msg, sock);
    std::thread rcv(recv_msg, sock);

    // Wait for threads to finish
    snd.join();
    rcv.join();

    // Close the socket
    close(sock);

    return 0;
}

// Function to send messages to the server
void send_msg(int sock) {
    while (1) {
        // Get user input
        getline(std::cin, msg);

        // Check for quitting command
        if (msg == "Quit" || msg == "quit") {
            close(sock);
            exit(0);
        }

        // Format message with sender's name and send to the server
        std::string name_msg = name + " " + msg;
        send(sock, name_msg.c_str(), name_msg.length() + 1, 0);
    }
}

// Function to receive messages from the server
void recv_msg(int sock) {
    char name_msg[BUF_SIZE + name.length() + 1];
    while (1) {
        // Receive messages from the server
        int str_len = recv(sock, name_msg, BUF_SIZE + name.length() + 1, 0);
        if (str_len == -1) {
            exit(-1);
        }

        // Print received message to the console
        std::cout << std::string(name_msg) << std::endl;
    }
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
