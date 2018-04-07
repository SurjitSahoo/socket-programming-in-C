#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#if defined(_WIN32)
	#include <winsock.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
#endif

#define PORT "9999"
#define MAXDATA 1024

void* get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
	  return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
	int sockfd, numbytes, datalen;
	char buf[MAXDATA];
	struct addrinfo hint, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN], ip[15];

	#ifdef _WIN32
	  WSADATA wsadata;
	  if (WSAStartup(MAKEWORD(1,1), &wsadata) == SOCKET_ERROR) {
	  printf("Error creating socket.");
	  return -1;
 	}
	#endif

	memset(&hint, 0, sizeof hint);
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;

	printf("Enter the IP address of the server: ");
	scanf(" %s", ip);

	if((rv = getaddrinfo(ip, PORT, &hint, &servinfo)) != 0){
	  fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	  return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next){
	  if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
	    perror("Client: socket");
	    continue;
	  }

	  if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
	    perror("client: connect");
	    continue;
	  }
	  break;
	}

	if(p == NULL){
	  fprintf(stderr, "Client: failed to connect..\n");
	  return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
	printf("client: Connecting to %s\n", s);
	freeaddrinfo(servinfo);

	while(1){
	  printf("> ");
	  scanf(" %[^\n]", buf);
	  datalen = strlen(buf);
	  if((numbytes = send(sockfd, buf, datalen, 0)) == -1){
	    perror("client: send");
	    exit(1);
	  }
	}

	#ifdef __WIN32
	  WSACleanup();
	#endif
	return 0;
}
