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
	int sockfd, numbytes;
	char buf[MAXDATA];
	struct addrinfo hint, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN], ip[15];
	long rdata;
	int i;

	struct msg{
		int a;
		int *p;
		int s;
		char *name;
	};

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

	if((rdata = recv(sockfd, buf, 27, 0)) == -1){
		perror("recv");
	}
	printf("rdata: %ld\n", rdata);
	if(rdata == 27){
		struct msg m1;
		uint32_t length = 0;
		memcpy(&m1.a, buf + length, sizeof(int));
		printf("m1.a : %d\n", m1.a);
		length += sizeof(int);
		memcpy(&m1.s, (buf + length), sizeof(int));
		length += sizeof(int);
		m1.p = (int *) malloc(m1.s);
		memcpy(m1.p, (buf + length), m1.s);
		length += m1.s;
		m1.name = (char *) malloc(strlen(buf+length) + 1);
		memcpy(m1.name, (buf + length), (strlen(buf+length) + 1));
		

		printf("received struct is:\n");
		printf("m1.a : %d\n", m1.a);
		printf("m1.p : ");
		for(i = 0; i < m1.s/sizeof(int); i++) printf(" %d,", m1.p[i]);
		printf("\n");
	    printf("m1.name : %s\n", m1.name);
	    free(m1.p);
	}

	#ifdef __WIN32
	  WSACleanup();
	#endif
	return 0;
}
