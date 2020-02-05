#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

#include "simulator.h"


void main() {
  int                 clientSocket;  // client socket id
  struct sockaddr_in  clientAddress; // client address
  int                 status, bytesRcv;
  unsigned char       command = SHUTDOWN; // set to SHUTDOWN

  // Contact all the cell towers and ask them to shut down

  char                inStr[80];    // stores user input from keyboard
  char                buffer[80];   // stores sent and received data



  for (int i=0; i<NUM_TOWERS; i++) { // connect to and shut down each tower server
    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0) {
      printf("*** CLIENT ERROR: Could open socket.\n");
      exit(-1);
    }

    // Setup address
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    clientAddress.sin_port = htons((unsigned short) SERVER_PORT+i);

    // Connect to server
    status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
    if (status < 0) {
      printf("*** CLIENT ERROR: Could not connect.\n");
      exit(-1);
    }

    buffer[0] = command; // set first char as command

    send(clientSocket, buffer, sizeof(buffer), 0); // send command signal which is set to SHUTDOWN
    close(clientSocket);  // close socket
  }

  printf("CLIENT: Shutting down.\n");

}
