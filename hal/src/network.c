#include "hal/network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int logging_enabled = 1;

void Network_enable_logging(int enable) {
    logging_enabled = enable;
}

// --- CLIENT IMPLEMENTATION ---

int Network_connect(const char* ip_address, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        if (logging_enabled) perror("[NET] Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        if (logging_enabled) perror("[NET] Invalid address/ Address not supported");
        return -1;
    }

    if (logging_enabled) printf("[NET] Connecting to %s:%d...\n", ip_address, port);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        if (logging_enabled) perror("[NET] Connection Failed");
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

// --- SERVER IMPLEMENTATION ---

int Network_start_server(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // 1. Create Socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        if (logging_enabled) perror("[NET] Socket failed");
        return -1;
    }

    // 2. Set Socket Options (Reuse Address/Port to prevent 'Address already in use')
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        if (logging_enabled) perror("[NET] setsockopt failed");
        close(server_fd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on ALL network interfaces
    address.sin_port = htons(port);

    // 3. Bind to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        if (logging_enabled) perror("[NET] Bind failed");
        close(server_fd);
        return -1;
    }

    // 4. Start Listening (Queue up to 3 connections)
    if (listen(server_fd, 3) < 0) {
        if (logging_enabled) perror("[NET] Listen failed");
        close(server_fd);
        return -1;
    }
    
    
    if (logging_enabled) printf("[NET] Server listening on port %d...\n", port);
    return server_fd;
}

int Network_accept_client(int server_fd) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    if (logging_enabled) printf("[NET] Waiting for connection...\n");
    int client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    
    if (client_fd < 0) {
        if (logging_enabled) perror("[NET] Accept failed");
    } else {
        if (logging_enabled) printf("[NET] Client connected!\n");
    }
    return client_fd;
}

// --- SHARED IMPLEMENTATION ---

void Network_send(int sockfd, const char* message) {
    if (sockfd < 0) return;
    // Send the message (strlen)
    send(sockfd, message, strlen(message), 0);
}

int Network_recv(int sockfd, char* buffer, int max_len) {
    if (sockfd < 0) return -1;
    
    // Clear buffer first for safety
    memset(buffer, 0, max_len);
    
    // Read bytes
    int bytes_read = recv(sockfd, buffer, max_len - 1, 0);
    return bytes_read;
}

void Network_close(int sockfd) {
    if (sockfd >= 0) {
        close(sockfd);
        if (logging_enabled) printf("[NET] Connection closed.\n");
    }
}