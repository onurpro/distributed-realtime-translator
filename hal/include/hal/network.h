#ifndef NETWORK_H
#define NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

// --- CLIENT FUNCTIONS (For Board 4) ---

// Connect to a specific IP and Port.
// Returns a socket file descriptor (positive int) or -1 on error.
int Network_connect(const char* ip_address, int port);

// --- SERVER FUNCTIONS (For Boards 1, 2, 3) ---

// Start listening on a specific port.
// Returns a server socket FD or -1 on error.
int Network_start_server(int port);

// Block until a client connects.
// Returns the client socket FD or -1 on error.
int Network_accept_client(int server_fd);

// --- SHARED FUNCTIONS ---

// Send a string message over a socket.
void Network_send(int sockfd, const char* message);

// Receive data into a buffer.
// Returns number of bytes read.
int Network_recv(int sockfd, char* buffer, int max_len);

// Close the connection.
void Network_close(int sockfd);

#ifdef __cplusplus
}
#endif

#endif