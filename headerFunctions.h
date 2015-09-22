// Initializes the Segway and the socket that is used to communicate with the Segway.
// The Segway is ready to receive move commands after a successful call.
// Returns an int that indicates the success/failure of the function.
// Return #'s: 1: Success!, -1: Failed to initialize WinSock, -2: Failed to create socket, -3: Failed to bind socket, -4: Failed to send packet.
int initSegway();

// Moves the Segway in a given direction passed as an int.
// Defined here: 0: FORWARD, 1: BACKWARD, 2: LEFT, 3: RIGHT, 4: RCW, 5: RCCW, where RCW and RCCW are rotations that are either CW or CCW.
void move(int);

// Sets the operational mode of the Segway passed as an int.
// Defined here: 1: Disable, 2: Powerdown, 3: DTZ, 4: Standby, 5: Tractor.
void setOperationalMode(int);

// Allows for custom movements.
// velCmd: Scaled velocity of the RMP, can be -1.0 to 1.0. Default set to 0.75.
// yawCmd: Scaled angular velocity of the RMP, can be -1.0 to 1.0. Default for rotation set to 0.5.
// angleCmd: Direction of the RMP in degrees. The directions are arranged like a coordinate grid: Right->0.0, Forward->90.0, Left->180.0, Backward->270.0.
void customMove(double, double, double);
