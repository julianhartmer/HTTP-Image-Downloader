/* 
 * Download images from http URL and output it to sdtout.
 * Does only work with http, not with https!
 * Compile with `make`.
 * Usage ./http HTTP_URL_TO_IMAGE
 */
#include "lib/tcp.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define FIRST_PART_RQ "GET "
#define SECOND_PART_RQ " HTTP/1.1\r\n"\
	"HOST: "
#define THIRD_PART_RQ " \r\n\r\n"
#define CONTENT_LENGTH "Content-Length: "
#define HEADER_END "\r\n\r\n"

#define BUFF_SIZE 300

/*
 * Create request message
 * @param msg message buffer
 * @param path URL String
 * @param path_len length of path
 * @param host host name
 * @param host_len length of host
 * @return length of message msg
 */
size_t create_request(char **msg, char *path, size_t path_len, char *host, size_t host_len) {
	size_t msg_len = strlen(FIRST_PART_RQ) + strlen(SECOND_PART_RQ) + path_len
		+ host_len + strlen(THIRD_PART_RQ);
	*msg = calloc(sizeof (char), msg_len+1);
	sprintf(*msg, "%s%s%s%s%s", FIRST_PART_RQ, path, SECOND_PART_RQ, host, THIRD_PART_RQ);
	return msg_len;
}

/*
 * Returns the index of the first header character
 * @param msg message buffer
 * @param len length of msg_len
 * @return offset to message header
 */
size_t find_header_end(char *msg, size_t len) {
	size_t i;
	for (i = 0; i < len-strlen(HEADER_END); i++) {
		if (0 == strncmp(&msg[i], HEADER_END, strlen(HEADER_END)))
			return i+strlen(HEADER_END);
	}
	return 0;
}

/*
 * Get the size of the content field in message.
 * @param msg message buffer
 * @param len length of msg_len
 * @return number of bytes of content field of the message
 */
size_t find_content_size(char *msg, size_t len) {
	size_t i, j;
	size_t content_size = 0;
	
	for (i = 0; i < len-strlen(CONTENT_LENGTH); i++) {
		if (0 == strncmp(&msg[i], CONTENT_LENGTH, strlen(CONTENT_LENGTH)))
			break;
	}
	if (i == len) {
		return 0;
	}
	for (j = i+strlen(CONTENT_LENGTH); msg[j] >= '0' && msg[j] <= '9' ; j++) {
		content_size = content_size * 10 + msg[j]-'0';
	}
	return content_size;

}

/*
 * Parse the host and object field from given URL string.
 * @param argv given URL string
 * @param obj target object from host
 * @param host hostname string
 * @return 1 on success, 0 on error
 */
int parse_address(char *argv, char **obj, char **host) {
	size_t http_len = strlen("http://");
	for (int i = http_len+1; i < strlen(argv); i++) {
		if ('/' == argv[i]) {
			*obj = calloc(sizeof (char), i-http_len+1);
			strncpy(*obj, &argv[http_len], i-http_len);
			*host = calloc(sizeof (char), strlen(argv)-i+1);
			strncpy(*host, &argv[i], strlen(argv)-i);
			return 1;
		}
	}
	return 0;
}

int main(int argc, char* argv[argc]) {
	int sockfd;
	char *host, *obj, *msg, buff[BUFF_SIZE], *resp;
	size_t resp_len, header_end, content_size, buff_len, msg_len;
	enum states {RECV_HEAD, RECV_BODY, FIN};
	enum states state = RECV_HEAD;

	if (argc != 2 || 0 == parse_address(argv[1], &host, &obj)) {
		fprintf(stderr, "usage: %s ADDRESS_TO_IMAGE\n", argv[0]);
		exit(1);
	}

	// send HTTP-Request
	msg_len = create_request(&msg, obj, strlen(obj), host, strlen(host));
	tcp_connect_to(host, "80", &sockfd);
	tcp_send(sockfd, msg, msg_len, 0);
	free(msg);

	resp_len = 0;
	resp = NULL;
	// recieve response
	while (FIN != state) {
		buff_len = tcp_recv(sockfd, buff, BUFF_SIZE, 0);
		resp = realloc(resp, resp_len+buff_len);
		memcpy(&resp[resp_len], buff, buff_len);
		resp_len += buff_len;

		switch (state) {
		case RECV_HEAD: 	// header not fully recieved
			if (0 != (header_end = find_header_end(resp, resp_len))) {
				state = RECV_BODY;
				content_size = find_content_size(resp, resp_len);
				// dont break, check img already recieved
			} else {
				break;
			}
		case RECV_BODY:		// img not fully recieved
			if (resp_len - header_end >= content_size) 
				state = FIN;
			break;
		default:break;
		}
	}

	for (size_t i = header_end; i < resp_len; i++) {
		printf("%c", resp[i]);
	}

	tcp_close(sockfd);
	free(resp);
	free(host);
	free(obj);
}
