#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
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
	int serv, clnt, pid;
	struct addrinfo hint, *servinfo, *p;
	int rv, opt, x, rdata;
	struct sockaddr_storage their_address;
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN], *buf, filename[20], directory[MAXDATA];
	long datalen;
	DIR *dr;
	struct dirent *de;
	FILE *fp;

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

	    while(1){
	      if((rdata = recv(clnt, &x, sizeof(int), 0)) > 0){
  	        opt = ntohl(x);
	        switch(opt){
	      	  case 1 :
	      	    if((dr = opendir(".")) != NULL){
	      	  	  while((de = readdir(dr)) != NULL){
	      	  	    sprintf(filename, "%s\n", de->d_name);
									strcat(directory, filename);
	      	  	  }
					      rdata = send(clnt, directory, strlen(directory), 0);
	      	    }
							else send(clnt, "unable to open directory\n", 25, 0);
	      	    break;

				    case 2 :
						  if((rdata = recv(clnt, filename, 20, 0)) > 0){
								filename[rdata] = '\0';
								if((fp = fopen(filename, "r")) != NULL){
									fseek(fp, 0, SEEK_END);
									datalen = ftell(fp);
									buf = (char*)malloc(datalen + 1);
									fseek(fp, 0, SEEK_SET);
									fread(buf,datalen, 1, fp);
									fclose(fp);
									if((rdata = send(clnt, buf, datalen, 0)) < 0){
										perror("Server: send file");
										continue;
									}
									free(buf);
								}
							}
							break;

						case 3 :
						  if((rdata = recv(clnt, filename, 20, 0)) > 0){
								filename[rdata] = '\0';
								buf = (char*)malloc(3048);
								if((rdata = recv(clnt, buf, 3048, 0)) > 0){
									buf[rdata] = '\0';
									fp = fopen(filename, "w");
									fprintf(fp, "%s", buf);
									fclose(fp);
								}
								else{
									perror("server: recv file");
									continue;
								}
							}
							else{
								perror("server: recv filename to upload");
								continue;
							}
							break;
	        }


          }
	    }
	  }
	}

	#ifdef __WIN32
	  WSACleanup();
	#endif
	return 0;
}
