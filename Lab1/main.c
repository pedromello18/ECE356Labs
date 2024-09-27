#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int server(uint16_t port);
int client(const char * addr, uint16_t port);

#define MAX_MSG_LENGTH (1300)
#define MAX_BACK_LOG (5)

int main(int argc, char ** argv)
{
	if (argc < 3 || (argv[1][0] == 'c' && argc < 4)) {
		printf("usage: myprog c <port> <address> or myprog s <port>\n");
		return 0;
	}

	uint32_t number = atoi(argv[2]);
	if (number < 1024 || number > 65535) {
		fprintf(stderr, "port number should be larger than 1023 and less than 65536\n");
		return 0;
	}

	uint16_t port = atoi(argv[2]);
	
	if (argv[1][0] == 'c') {
		return client(argv[3], port);
	} else if (argv[1][0] == 's') {
		return server(port);
	} else {
		fprintf(stderr, "unkonwn commend type %s\n", argv[1]);
		return 0;
	}
	return 0;
}

int client(const char * addr, uint16_t port)
{
	int sock;
	struct sockaddr_in server_addr;
	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];

	if ((sock = socket(AF_INET, SOCK_STREAM/* use tcp */, 0)) < 0) {
		perror("Create socket error:");
		return 1;
	}

	printf("Socket created\n");
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Connect error:");
		return 1;
	}

	printf("Connected to server %s:%d\n", addr, port);
	printf("Enter message: \n");
	while (fgets(msg, MAX_MSG_LENGTH, stdin)) {
		if (send(sock, msg, strnlen(msg, MAX_MSG_LENGTH), 0) < 0) {
			perror("Send error:");
			return 1;
		}
		int recv_len = 0;
		if ((recv_len = recv(sock, reply, MAX_MSG_LENGTH, 0)) < 0) {
			perror("Recv error:");
			return 1;
		}
		if (recv_len == 0){
			printf("Server disconnected, client quit.\n");
			break;
		}
		reply[recv_len] = '\0';
		printf("Server reply:\n%s", reply);
		printf("Enter message: \n");
	}
	if (send(sock, "", 0, 0) < 0) {
		perror("Send error:");
		return 1;
	}
	return 0;
}

int server(uint16_t port){
	struct sockaddr_in sin;
	char buf[MAX_MSG_LENGTH];
	int len;
	int s, new_s;

	bzero((char *)&sin, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("simplex-talk: socket");
		exit(1);
	}
	if((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0){
		perror("simplex-talk: bind");
		exit(1);
	}
	listen(s, MAX_BACK_LOG);
	printf("Server Listening...\n");

	while(1){
		if((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0){
			perror("simplex-talk: accept");
			exit(1);
		}
		printf("Client Connected...\n");
		while(len = recv(new_s, buf, sizeof(buf) - 1, 0)){
			fputs(buf, stdout);
			buf[len] = '\0';
			send(s, buf, len, 0);
		}
		close(new_s);
	}
	return 0;
}
