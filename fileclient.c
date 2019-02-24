#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
/* In windows library for network program is different from linux */
//For Windows
#if defined(_WIN32)
	#include <winsock.h>
#else //for linux
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
#endif

#define PORT "9999"
#define MAXDATA 1024

/*
    Find out address family(IP address specificaly) IPv4 or IPv6
    sin_addr = IPv4
    sin6_addr = IPv6
*/
void* get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
	  return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
	int sockfd; // socket file descripter for client
	char *buf;
	struct addrinfo hint, *servinfo, *p;
	/*
		struct addrinfo {
	    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
	    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
	    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
	    int              ai_protocol;  // use 0 for "any"
	    size_t           ai_addrlen;   // size of ai_addr in bytes
	    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
	    char            *ai_canonname; // full canonical hostname

	    struct addrinfo *ai_next;      // linked list, next node
		};

		struct sockaddr {
		    unsigned short    sa_family;    // address family, AF_xxx
		    char              sa_data[14];  // 14 bytes of protocol address
		};
	*/
	int rv;
	char s[INET6_ADDRSTRLEN], ip[15], dir[MAXDATA], filename[20];
	int opt, x;
	long rdata, datalen;
	FILE *fp;

	#ifdef _WIN32
	  WSADATA wsadata;
	  // The WSAStartup function initiates use of the Winsock DLL by a process
	  if (WSAStartup(MAKEWORD(1,1), &wsadata) == SOCKET_ERROR) {
	  printf("Error creating socket.");
	  return -1;
 	}
	#endif

 	/*
	Hint have some details specified about the serverinfo should be set up
	It is used in getaddrinfo()
 	*/
	memset(&hint, 0, sizeof hint);
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;

	printf("Enter the IP address of the server: ");
	scanf(" %s", ip);

	/*
	  getaddrinfo() returns list of available sockets
	  create server address structure based on server ip address and port and hint
	*/
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
		printf("1. List available files on the server\n");
		printf("2. Download a file\n");
		printf("3. Upload a file to the server\n");
		printf("4. Exit\n");
		scanf("%d", &opt);
		x = htonl(opt); //host to network converssion
		if(send(sockfd, &x, sizeof(int), 0) < 0){
			perror("client: send");
			continue;
		}
		switch(opt){
			case 1 :
				if((rdata = recv(sockfd, dir, MAXDATA-1, 0)) > 0){
				dir[rdata] = '\0';
					    printf("%s\n", dir);
				rdata = 0;
				}
				break;

			case 2 :
				printf("Enter the filename: ");
				scanf(" %s", filename);
				send(sockfd, filename, strlen(filename), 0);
				buf = (char*)malloc(3048);
				if((rdata = recv(sockfd, buf, 3048, 0)) > 0){
					fp = fopen(filename, "w");
					fprintf(fp, "%s", buf);
					free(buf);
					printf("File download complete\n\n");
					fclose(fp);
				}
				else{
					printf("Unable to download file..\n\n");
					continue;
				}
				break;

			case 3 :
			    printf("Enter filename to upload: ");
				scanf(" %s", filename);
				if((send(sockfd, filename, strlen(filename), 0)) < 0){
					perror("client: send filename");
					continue;
				}
				if((fp = fopen(filename, "r")) != NULL){
					fseek(fp, 0, SEEK_END);
					datalen = ftell(fp);
					fseek(fp, 0, SEEK_SET);
					buf = (char*)malloc(datalen + 1);
					fread(buf, datalen, 1, fp);
					fclose(fp);
					if((rdata = send(sockfd, buf, datalen, 0)) < 0){
						perror("client: send file");
						continue;
					}
					else{
						free(buf);
						printf("File upload successul\n\n");
					}
				}
				else{
					printf("failed to open and send file\n\n");
					continue;
				}
				break;

			case 4:
			  return 0;

			default :
			   printf("Not implemented\n");
			   break;
		}
	}

	#ifdef __WIN32
	  WSACleanup();
	#endif
	return 0;
}
