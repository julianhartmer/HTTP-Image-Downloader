#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "network.h"

struct addr {
	struct sockaddr addr;
	socklen_t addrlen;
};

ADDR *allocate_addr() {
	return calloc(sizeof (struct addr), 1);
}
