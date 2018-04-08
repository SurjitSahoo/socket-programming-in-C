#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#define ERROR -1
#define MAX_DATA 1024

int main(){
  int welcomeSocket, newSocket;
  char buffer[MAX_DATA];
  int data_len;
  struct sockaddr_in serverAddr;
  int addr_size = sizeof(struct sockaddr_in);

  if((welcomeSocket = socket(PF_INET, SOCK_STREAM, 0)) == ERROR){
    printf("Socket: ");
    exit(-1);
  }
  /* Configure settings of the server address struct */
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(9999);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /* Bind the server address struct to the socket */
  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  /* Listen on the socket, with 5 max connection requests queued */
  if(listen(welcomeSocket,5) == 0)
    printf("Listening\n");
  else
    printf("Error\n");

  while(1){
    /* Accept call for the incoming connection */
    if((newSocket = accept(welcomeSocket, (struct sockaddr *)&newSocket, &addr_size)) == ERROR){
      perror("accept");
      exit(-1);
    }
    data_len = 1;
    while(data_len){
      /* receive msg and print */
      data_len = recv(newSocket, buffer, MAX_DATA, 0);
      if(data_len){
        buffer[data_len] = '\0';
        printf("%s\n", buffer);
      }
    }
  }
  printf("Client Disconnected\n");
  close(newSocket);

  return 0;
}
