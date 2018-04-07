#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
/* In windows library for network program is different from linux */
//For Windows
#if defined(_WIN32)
	#include <winsock.h>
#else //for linux
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
#endif


#define PORT "9999"
#define BACKLOG 10 //for pending connections (multiple connections)
#define MAXDATA 1024 //maximum size of data to be received

/*
    Find out address family(IP address specificaly) IPv4 or IPv6
    sin_addr = IPv4
    sin6_addr = IPv6
*/
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
	/*
		hint : to specify which type of address we want
		servinfo : Linked list of address structure
		p : useable address structure. to be used with connect() and bind()
			it contains the address structure of the machine


		struct addrinfo {
	    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
	    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC (IPv4 or IPv6)
	    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM (TCP, UDP)
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
	struct sockaddr_storage their_address;
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN], buf[MAXDATA];
	long datalen;

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
	hint.ai_family = AF_UNSPEC; //as we don't know incoming request will be of IPv4 or IPv6
	hint.ai_socktype = SOCK_STREAM; 
	hint.ai_flags = AI_PASSIVE;

	//returns 0 on success otherwise errror code
	if((rv = getaddrinfo(NULL, PORT, &hint, &servinfo)) != 0){
	  printf("getaddrinfo: %s\n", gai_strerror(rv));
	  return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next){
		// Create serv socket according to p address structure
	  if((serv = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
	    perror("server: socket");
	    exit(1);
	  }

	  //socket created but has no actual address. assign address with bind()
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

	//listen on the created socket
	if(listen(serv, BACKLOG) == -1){
	  perror("listen");
	  exit(1);
	}

	printf("Server: waiting for connection..\n");

	sin_size = sizeof their_address; //their_address will contain clien's address

	for(;;){
	  /*
		accept() returns client socket
		serv : server socket to accept incomming connections on
		struct sockaddr * their_address : return parameter -> filled with the client address
		socklen_t * sin_size : return parameter -> filled with client address size
	  */
	  clnt = accept(serv, (struct sockaddr*)&their_address, &sin_size);
	  if(clnt == -1){
	    perror("accept");
	    continue;
	  }
	
	  /*
		after accepting the connect process it on new process
		so the server will keep listenning for new connections
		fork() returns twice. returns 0 for child process and parent process id for parent process
	  */
	  if((pid = fork()) == 0){ // will be processed on child process, i.e. pid = 0
	 
	 	// convert network address to presentation
	    inet_ntop(their_address.ss_family, get_in_addr((struct sockaddr*)&their_address), s, sizeof s);
	    printf("Server: got a connection from %s\n", s);
	    
	    while(1){
	      if((datalen = recv(clnt, buf, MAXDATA-1, 0)) == -1){
	        perror("server: recv");
	        break;
	      }
	      if(datalen){
	        buf[datalen] = '\0';
	        printf("> %s\n", buf);	  
	      }
	      else{
	        printf("Clinet disconnected..\n");
	        close(clnt);
	        break;
	      }
	    }
	  }
	  else continue; // parent process will continue listening for new connections
	}

	#ifdef __WIN32
	  WSACleanup();
	#endif
	return 0;
}