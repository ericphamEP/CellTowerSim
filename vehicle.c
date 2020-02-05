#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


// GPS Data for this client as well as the connected tower ID
short  x;
short  y;
short  direction;
char  connectionID;
char  connectedTowerID;
char  connectedTowerIndex;


int main(int argc, char * argv[]) {
  int                 clientSocket;  // client socket id
  struct sockaddr_in  clientAddress; // client address
  int                 status, bytesRcv;
  unsigned char       buffer[30];
  unsigned char       command;
  char turn;


  // Set up the random seed
  srand(time(NULL));

  // Get the starting coordinate and direction from the command line arguments
  x = atoi(argv[1]);
  y = atoi(argv[2]);
  direction = atoi(argv[3]);

  // To start, this vehicle is not connected to any cell towers
  connectionID = -1;
  connectedTowerID = -1;

  // Go into an infinite loop to keep sending updates to cell towers
  while(1) {
    usleep(50000);  // A delay to slow things down a little
    //usleep(500000);

    // move forward in current direction
    x += VEHICLE_SPEED*cos(direction*(M_PI/180)); // convert direction degrees to radians
    y += VEHICLE_SPEED*sin(direction*(M_PI/180));

    // change direction randomly
    turn = (char)(rand() % 3);
    //printf("Turn is now %c\n", turn);
    if (turn == 1) {
      direction += VEHICLE_TURN_ANGLE;
    } else if (turn == 2) {
      direction -= VEHICLE_TURN_ANGLE;
    }

    // keep direction in range of -180 to 180 degrees
    if (direction > 180) {
      direction -= 360;
    } else if (direction < -180) {
      direction += 360;
    }



    if (connectionID == -1) { // if not connected to a tower, CONNECT to all towers (and check if left the city, then break and close)

      // check if left the city, and if so, break
      if (x < 0 || x > CITY_WIDTH || y < 0 || y > CITY_HEIGHT) {
        printf("Vehicle left the city.\n");
        break;
      }


      command = CONNECT;
      for (int i=0; i<NUM_TOWERS; i++) { // check each tower
        // Create socket
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket < 0) {
          //printf("*** Vehicle ERROR: Couldn't open socket.\n");
          break;
        }

        // Setup address
        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddress.sin_family = AF_INET;
        clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
        clientAddress.sin_port = htons((unsigned short) SERVER_PORT+i);

        // Connect to server
        status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
        if (status < 0) {
          //printf("*** Vehicle ERROR: Could not connect to server.\n");
          continue;
        }

        // set buffer to contain command and x and y coordinates
        buffer[0] = command;
        // x coord
        buffer[1] = x>>8;
        buffer[2] = (x & 0b0000000011111111);
        // y coord
        buffer[3] = y>>8;
        buffer[4] = (y & 0b0000000011111111);

        send(clientSocket, buffer, sizeof(buffer), 0); // 3rd param is length of buffer

        // Get response from server, should be YES or NO
        bytesRcv = recv(clientSocket, buffer, 80, 0);
        buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
        //printf("Vehicle: Got back response \"%s\" from server.\n", buffer);

        close(clientSocket);

        if (buffer[0] == YES) { // connect to tower
          connectionID = 1;
          connectedTowerID = buffer[1]; // tower ID
          connectedTowerIndex = buffer[2]; // index of vehicle
          break;
        }
      }
    }
    if (connectionID == 1) { // if connected, update
      command = UPDATE;

      // send UPDATE, if recieve a NO it means it's out of tower boundary

      // Create socket
      clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (clientSocket < 0) {
        //printf("*** Vehicle ERROR: Couldn't open socket.\n");
        break;
      }

      // Setup address
      memset(&clientAddress, 0, sizeof(clientAddress));
      clientAddress.sin_family = AF_INET;
      clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
      clientAddress.sin_port = htons((unsigned short) SERVER_PORT+connectedTowerID);

      // Connect to server
      status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
      if (status < 0) {
        //printf("*** Vehicle ERROR: Could not connect to server.\n");
        connectionID = -1;
        continue;
      }

      // set buffer to contain command and x and y coordinates
      buffer[0] = command;
      // x coord
      buffer[1] = x>>8;
      buffer[2] = (x & 0b0000000011111111);
      // y coord
      buffer[3] = y>>8;
      buffer[4] = (y & 0b0000000011111111);
      // index of vehicle
      buffer[5] = connectedTowerIndex;

      send(clientSocket, buffer, sizeof(buffer), 0); // send UPDATE signal

      // Get response from server, should be YES or NO
      bytesRcv = recv(clientSocket, buffer, 80, 0);
      buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
      //printf("Vehicle: Got back response \"%s\" from server.\n", buffer);

      close(clientSocket);

      if (buffer[0] == NO) { // disconnect from tower
        connectionID = -1;
      }



    }



  }
  printf("Vehicle: Shutting down.\n");
}
