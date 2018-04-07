#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <wait.h>
#if defined(_WIN32)
	#include <winsock.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
#endif


#define PORT "9999"
#define BACKLOG 10
#define MAXDATA 1048

void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
	  return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
	int serv, clnt, pid, i;
	struct addrinfo hint, *servinfo, *p;
	int rv;
	struct sockaddr_storage their_address;
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN], *buf;
	long rdata;

	//struct to be sent
	struct msg{
		int a;
		int *p;
		int s;
		char *name;
	};

	struct msg m1;
	m1.a = 5;
	m1.p = (int*)calloc(3, sizeof(int));
	for(i = 0; i < 3; i++) m1.p[i] = i + 1;
	m1.s = sizeof(int) * 3;
	m1.name = "Surjit";

	size_t size = sizeof(int) + m1.s + sizeof(int) + strlen(m1.name) + 1;
	buf = (char*)malloc(size);
    uint32_t data_length = 0;
	memcpy(buf, &m1.a, sizeof(int));
	data_length += sizeof(int);
	memcpy(buf + data_length, &m1.s, sizeof(int));
	data_length += sizeof(int);
	memcpy(buf + data_length, m1.p, m1.s);
	data_length += m1.s;
	memcpy(buf + data_length, m1.name, strlen(m1.name) + 1);
	data_length += (strlen(m1.name) + 1);
	
	printf("%lu == %u\n", size, data_length);	

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
	hint.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(NULL, PORT, &hint, &servinfo)) != 0){
	  printf("getaddrinfo: %s\n", gai_strerror(rv));
	  return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next){
	  if((serv = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
	    perror("server: socket");
	    exit(1);
	  }

	  if(bind(serv, p->ai_addr, p->ai_addrlen) == -1){
	    close(serv);
	    perror("server: bind");
	    exit(1);
	  }
	  break;
	}

	freeaddrinfo(servinfo);
	if(p == NULL){
	  fprintf(stderr, "server: failed to bind\n");
	  exit(1);
	}

	if(listen(serv, BACKLOG) == -1){
	  perror("listen");
	  exit(1);
	}

	printf("Server: waiting for connection..\n");

	sin_size = sizeof their_address;

	for(;;){
	  clnt = accept(serv, (struct sockaddr*)&their_address, &sin_size);
	  if(clnt == -1){
	    perror("accept");
	    continue;
	  }

	  if((pid = fork()) == 0){

	    inet_ntop(their_address.ss_family, get_in_addr((struct sockaddr*)&their_address), s, sizeof s);
	    printf("Server: got a connection from %s\n", s);

	    if((rdata = send(clnt, buf, size, 0)) == -1){
	    	perror("send");
	    	return -1;
	    }
	  }
	  waitpid(pid, NULL, 0);
	}

	#ifdef __WIN32
	  WSACleanup();
	#endif
	return 0;
}
