#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>



// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should wait for a client to send a request, process it, and then
// close the client connection and wait for another client.  The requests that may be
// handles are SHUTDOWN, CONNECT and UPDATE.  A SHUTDOWN request causes the tower to
// go offline.   A CONNECT request contains 4 additional bytes which are the high and
// low bytes of the vehicle's X coordinate, followed by the high and low bytes of its
// Y coordinate.  If within range of this tower, the connection is accepted and a YES
// is returned, along with a char id for the vehicle and the tower id.   If UPDATE is
// received, the additional 4 byes for the (X,Y) coordinate are also received as well
// as the id of the vehicle.   Then YES is returned if the vehicle is still within
// the tower range, otherwise NO is returned.
void *handleIncomingRequests(void *ct) {
  CellTower       *tower = ct;

  int                 serverSocket, clientSocket;
  struct sockaddr_in  serverAddress, clientAddr;
  int                 status, addrSize, bytesRcv;
  unsigned char       buffer[30];
  unsigned char       response[30];

  short carX; // store current vehicle coordinates
  short carY;
  char inBound; // flag for if a car is within range of tower

  // Create the server socket
  serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serverSocket < 0) {
    //printf("*** SERVER ERROR: Could not open socket.\n");
    exit(-1);
  }

  // Setup the server address
  memset(&serverAddress, 0, sizeof(serverAddress)); // zeros the struct
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons((unsigned short) SERVER_PORT+(tower->id));

  // Bind the server socket
  status = bind(serverSocket,  (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (status < 0) {
    //printf("*** SERVER ERROR: Could not bind socket.\n");
    exit(-1);
  }

  // Set up the line-up to handle up to MAX_CONNECTIONS clients in line
  status = listen(serverSocket, MAX_CONNECTIONS);
  if (status < 0) {
    //printf("*** SERVER ERROR: Could not listen on socket.\n");
    exit(-1);
  }

  // Wait for clients now
  while (1) {
    addrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
    if (clientSocket < 0) {
      //printf("*** SERVER ERROR: Could not accept incoming client connection.\n");
      exit(-1);
    }
    //printf("SERVER: Received client connection.\n");

    // Get the message from the client
    bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);
    buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
    //printf("SERVER: Received client request: %s\n", buffer);

    if (buffer[0] == CONNECT || buffer[0] == UPDATE) {
      // check distance from car to cell tower
      carX = ((unsigned short)buffer[1]<<8) | (unsigned short)buffer[2];
      carY = ((unsigned short)buffer[3]<<8) | (unsigned short)buffer[4];

      inBound = (((carX-tower->x)*(carX-tower->x) + (carY-tower->y)*(carY-tower->y)) <= (tower->radius)*(tower->radius)); // compare distance with radius
      //printf("Tower: %d, inBound is %d\n",tower->id ,inBound);
      //printf("carX: %s carY: %s\n", carX, carY);

      // if connect, check if there's available spot
      if (buffer[0] == CONNECT) {
        // if not close enough, send no
        if (!inBound) {
          response[0] = NO;
        } else {
          // check if available spot in connectedVehicles
          for (int i=0; i<MAX_CONNECTIONS; i++) {
            response[0] = NO;
            if (tower->connectedVehicles[i].connected ==  0) {
              // set response
              response[0] = YES;
              response[1] = tower->id; // tower id
              response[2] = i; // index of vehicle connection storage
              // set connectedVehicles info
              tower->connectedVehicles[i].connected = 1;
              tower->connectedVehicles[i].x = carX;
              tower->connectedVehicles[i].y = carY;
              tower->numConnectedVehicles++;
              break;
            }
          }
        }

      } else if (buffer[0] == UPDATE) {
        response[0] = YES;
        // if not close enough, remove car from index
        if (!inBound) {
          response[0] = NO;
          tower->connectedVehicles[buffer[5]].connected = 0; //using buffer[5] as the index to make connect = 0
          tower->numConnectedVehicles--;
        } else { // update coordinates
          tower->connectedVehicles[buffer[5]].x = carX;
          tower->connectedVehicles[buffer[5]].y = carY;
        }

      }
      send(clientSocket, response, sizeof(response), 0); // send YES or NO
    }
    close(clientSocket); // Close this client's socket
    if (buffer[0] == SHUTDOWN) { // shut down tower connection
      //printf("SERVER: Closing client connection.\n");
      tower->online = 0;
      break;
    }

  }

  // close socket
  close(serverSocket);
  printf("TOWER %d SERVER: Shutting down.\n",(tower->id));



}
