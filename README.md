# CellTowerSim
A simulation for cell towers tracking vehicles.

Eric Pham
A past school assignment

------------------------

Source Files:
- cellTower.c
- display.c
- generator.c
- makefile
- simulator.c
- simulator.h
- stop.c
- vehicle.c

Instructions:
- To compile, navigate to directory with the source files in terminal and type:
	make all
- To run simulator, type:
	./simulator &
- To run vehicle generator, type:
	./generator &
- To stop generator, type fg, and then Ctrl+C
- To stop simulator, type:
	./stop
- To clear object and executable files, type:
	make clean

Request buffer info:

Each char in a buffer is used to represent a seperate signal.

All commands/main responses are stored in the char index 0 of each buffer/response (SHUTDOWN, CONNECT, UPDATE, YES, NO)

Vehicle requests:
- For requests sent from vehicle, x-coordinates are stored in char indexes 1 and 2, and y-coordinates are stored in char indexes 3 and 4
- If a vehicle is connected, char at index 5 will also send the index it is located in the tower's vehicle connections (connectedVehicles)

Tower responses:
- If responding to a CONNECT with char at index 0 being YES, chat at index 1 will represent the tower ID and at index 2 will represent the index of where the vehicle will be stored
