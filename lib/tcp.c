#define _POSIX_C_SOURCE 200112L
#include "tcp.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BACKLOG 10

#define ERROR_HOST_PORT_MISSING -1

struct addr {
	struct sockaddr addr;
	socklen_t addrlen;
};

size_t tcp_send(int sockfd, void *buf, size_t len, int flags) {
	return send(sockfd, buf, len, flags);
}

size_t tcp_recv(int sockfd, void *buf, size_t len, int flags) {
	return recv(sockfd, buf, len, flags);
}

int tcp_listen_on_port(char *port, int *sockfd) {
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int status = getaddrinfo(NULL, port, &hints, &servinfo);
	if (0 != status) {
		return status;
	}

	// iterate through list
	for (p = servinfo; p != NULL; p = p->ai_next) {
		*sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol);
		if (-1 == *sockfd) 
			continue;

		// allow rebinding
		int yes = 1;
		if (-1 == setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
					sizeof (int))) {

			perror("setsockopt");
			continue;
		}

		// bind socket
		if (-1 == bind(*sockfd, p->ai_addr, p->ai_addrlen)) {
			close(*sockfd);
			continue;
		}
		break;
	}
	
	freeaddrinfo(servinfo);

	if (p == NULL) {
		return -2;
	}

	if (-1 == listen(*sockfd, BACKLOG)) {
		return -3;
	}

	return 1;
}

int tcp_accept_connection(int sockfd, ADDR *addr_p) {
	struct addr *addr = (struct addr *) addr_p;
	addr->addrlen = sizeof addr->addr;
	return accept(sockfd, &addr->addr, &(addr->addrlen));
}

int tcp_connect_to(char *hostname, char *port, int *sockfd) {
	if (NULL == hostname || NULL == port) {
		return ERROR_HOST_PORT_MISSING;
	}

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int status;
	if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		return status;
	}

	// iterate through servinfo linked list, try all connections
	for (p = servinfo; p != NULL; p = p->ai_next) {
		*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (-1 == *sockfd) {
			continue;
		}

		if (-1 == connect(*sockfd, p->ai_addr, p->ai_addrlen)) {
			close(*sockfd);
			continue;
		}

		break;
	}
	
	freeaddrinfo(servinfo);
	return !!p;
}

int tcp_close(int sockfd) {
	return close(sockfd);
}

const char* tcp_print_error(int error_code) {
	if (ERROR_HOST_PORT_MISSING == error_code) {
		return "Port or Hostadress missing!";
	}
	return gai_strerror(error_code);
}
