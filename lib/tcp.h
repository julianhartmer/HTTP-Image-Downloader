#include <stdint.h>
#include <stddef.h>

#include "network.h"

// sends len-bytes from buf to socketfd. Returns amount of bytes sent
size_t tcp_send(int sockfd, void *buf, size_t len, int flags);

// read data from sockfd. Returns amount of bytes read.
size_t tcp_recv(int sockfd, void *buf, size_t len, int flags);

// creates and connects socket to host. Returns 0 on success
int tcp_connect_to(char *hostname, char *port, int *sockfd);

// closes socketfiledescriptor
int tcp_close(int sockfd);

// prints errorcodes to string
const char* tcp_print_error(int error_code);

int tcp_accept_connection(int sockfd, ADDR *addr);

// creates and bind socket to listen on specified port;
int tcp_listen_on_port(char *port, int *sockfd);
