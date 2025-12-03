/**
 * @file network.h
 * @brief Simple Network Abstraction Layer for TCP Client/Server.
 */

#ifndef NETWORK_H
#define NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Connect to a remote server.
 *
 * @param ip_address The IP address of the server (e.g., "192.168.1.10").
 * @param port The port number to connect to.
 * @return The socket file descriptor on success, or -1 on failure.
 */
int Network_connect(const char *ip_address, int port);

/**
 * @brief Start a TCP server on a specific port.
 *
 * @param port The port number to listen on.
 * @return The server socket file descriptor on success, or -1 on failure.
 */
int Network_start_server(int port);

/**
 * @brief Accept an incoming client connection.
 *
 * This function blocks until a client connects.
 *
 * @param server_fd The server socket file descriptor returned by
 * Network_start_server.
 * @return The client socket file descriptor on success, or -1 on failure.
 */
int Network_accept_client(int server_fd);

/**
 * @brief Send a message over a socket.
 *
 * @param sockfd The socket file descriptor.
 * @param message The null-terminated string to send.
 */
void Network_send(int sockfd, const char *message);

/**
 * @brief Receive data from a socket.
 *
 * @param sockfd The socket file descriptor.
 * @param buffer The buffer to store the received data.
 * @param max_len The maximum number of bytes to read.
 * @return The number of bytes read, or <= 0 on failure/disconnect.
 */
int Network_recv(int sockfd, char *buffer, int max_len);

/**
 * @brief Close a socket connection.
 *
 * @param sockfd The socket file descriptor to close.
 */
void Network_close(int sockfd);

/**
 * @brief Enable or disable internal logging.
 *
 * @param enable 1 to enable logging to stdout, 0 to disable.
 */
void Network_enable_logging(int enable);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_H