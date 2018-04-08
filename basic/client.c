#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

#define ERROR -1
#define MAX_DATA 1024

int main(){
  int clientSocket;
  char buffer[MAX_DATA], ip[18];
  unsigned int prt;
  int data_len;
  struct sockaddr_in serverAddr;
  size_t addr_size;

  /* Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  
  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* set IP address */
  printf("Enter the IP address of the server: ");
  scanf(" %s", ip);
  serverAddr.sin_addr.s_addr = inet_addr(ip);
  /* Set port number, using htons function to use proper byte order */
  printf("Enter the port no: ");
  scanf("%d", &prt);
  serverAddr.sin_port = htons(prt);
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*---- Connect the socket to the server using the address struct ----*/
  addr_size = sizeof serverAddr;
  if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size) == ERROR){
    printf("Connect\n");
    exit(-1);
  }
  printf("Connected\n");
  while(1){
    // send message
    printf("> ");
    scanf(" %[^\n]", buffer);
    data_len = strlen(buffer);
    send(clientSocket, buffer, data_len, 0);
  }  

  return 0;
}
