// Client-reference.
#include "headerFunctions.h"

// Include files from Linux Machine.
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Define the RMP Segway IP Address, Port, and size of buffer.
#define RECEIVER "192.168.0.40"
#define PORT 8080
#define BUFSIZE 2048

// Global variables that store socket information.
// Actual socket.
int handle;
// Addresses of the socket and the receiver.
struct sockaddr_in address;
struct sockaddr_in remaddr;
// Length of the receiver.
socklen_t addrlen;

// Definitions needed for CRC16.
#define INITIAL_CRC (0)
#define CRC_ADJUSTMENT 0xA001
#define CRC_TABLE_SIZE 256

// Global variable used for CRC.
uint16_t crcTable[CRC_TABLE_SIZE];

// Functions that execute a Cyclic-Redundancy-Check (CRC).
// For more information on how the CRC is performed for the RMP Segway, please
// refer to the user's manual for an RMP Segway 400. It can also be found
// online at: http://rmp.segway.com/. An account is required to access the guide.
uint16_t computeCrcTableValue(uint16_t theByte);

void tkCrcInitialize() {
    uint16_t byte;
    for (byte = 0; byte < CRC_TABLE_SIZE; byte++) {
        crcTable[byte] = computeCrcTableValue(byte);
    }
}

uint16_t tkCrcCalculateCrc16(uint16_t oldCrc, uint8_t newByte) {
    uint16_t temp;
    uint16_t newCrc;
    temp = oldCrc ^ newByte;
    newCrc = (oldCrc >> 8) ^ crcTable[temp & 0x00FF];
    return (newCrc);
}

void tkCrcComputeByteBufferCrc(uint8_t *byteBuffer, uint32_t bytesInBuffer) {
    uint32_t count;
    uint32_t crcIndex = bytesInBuffer - 2;
    uint16_t newCrc = INITIAL_CRC;

    for (count = 0; count < crcIndex; count++) {
        newCrc = tkCrcCalculateCrc16(newCrc, byteBuffer[count]);
    }

    byteBuffer[crcIndex] = (uint8_t)((newCrc & 0xFF00) >> 8);
    byteBuffer[crcIndex + 1] = (uint8_t)(newCrc & 0x00FF);
}

uint16_t computeCrcTableValue(uint16_t theByte) {
    uint16_t j;
    uint16_t k;
    uint16_t tableValue;

    k = theByte;
    tableValue = 0;
    for (j = 0; j < 8; j++) {
        if (((tableValue ^ k) & 0x0001) == 0x0001) {
            tableValue = (tableValue >> 1) ^ CRC_ADJUSTMENT;
        } else {
            tableValue >>= 1;
        }
        k >>= 1;
    }
    return (tableValue);
}

// Returns an int from a double.
int floatToIntBits(double value) {
    union {
        float f;
        int i;
    } u;

    if (value != value) {
        return 0x7fc00000;
    } else {
        u.f = value;
        return u.i;
    }
}


// Functions to be used in the shared library. See header file for descriptions.
int initSegway() {
    handle = socket(AF_INET, SOCK_DGRAM, 0);
    if (handle < 0) {
        return -1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(0);
    if (bind(handle, (struct sockaddr*)&address, sizeof(address)) < 0) {
        return -2;
    }
    addrlen = sizeof(remaddr);
    remaddr.sin_family = AF_INET;
    remaddr.sin_addr.s_addr = inet_addr(RECEIVER);
    remaddr.sin_port = htons(PORT);
    uint8_t data[12];
    data[0] = 0x05;
    data[1] = 0x01;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x20;
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x00;
    data[9] = 0x05;
    tkCrcInitialize();
    tkCrcComputeByteBufferCrc(data, 12);
    int sendbytes = sendto(handle, (char*)data, sizeof(data), 0, (struct sockaddr*)&remaddr, addrlen);
    if (sendbytes < 0) {
        return -3;
    } else {
        return 1;
    }
}

void move(int direction) {
        uint8_t data[12];
        data[0] = 0x06;
        data[1] = 0x00;
        int iiv = (int) (0.75 * 32767);
        int iiy = (int) (0.05 * 32767);
        int ieee754int;
        switch(direction) {
        case 0:
            ieee754int = floatToIntBits(90.0);
            break;
        case 1:
            ieee754int = floatToIntBits(270.0);
            break;
        case 2:
            ieee754int = floatToIntBits(180.0);
            break;
        case 3:
            ieee754int = floatToIntBits(0.0);
            break;
        case 4:
        case 5:
            ieee754int = floatToIntBits(90.0);
            iiv = (int) (0.0 * 32767);
            if (direction == 5) {
                iiy = (int) (-0.5 * 32767);
            } else {
                iiy = (int) (0.5 * 32767);
            }
            break;
        }
        data[2] = ((iiv >> 8) & 0xFF);
        data[3] = (iiv & 0xFF);
        data[4] = ((iiy >> 8) & 0xFF);
        data[5] = (iiy & 0xFF);
        data[6] = ((ieee754int >> 24) & 0xFF);
        data[7] = ((ieee754int >> 16) & 0xFF);
        data[8] = ((ieee754int >> 8) & 0xFF);
        data[9] = (ieee754int & 0xFF);
        tkCrcInitialize();
        tkCrcComputeByteBufferCrc(data, 12);
        sendto(handle, (char*)data, sizeof(data), 0, (struct sockaddr*)&remaddr, addrlen);
}

void setOperationMode(int mode) {
        uint8_t data[12];
        data[0] = 0x05;
        data[1] = 0x01;
        data[2] = 0x00;
        data[3] = 0x00;
        data[4] = 0x00;
        data[5] = 0x20;
        data[6] = 0x00;
        data[7] = 0x00;
        data[8] = 0x00;
        data[9] = mode;
        tkCrcInitialize();
        tkCrcComputeByteBufferCrc(data, 12);
        sendto(handle, (char*)data, sizeof(data), 0, (struct sockaddr*)&remaddr, addrlen);
}

void customMove(double velCmd, double yawCmd, double angleCmd) {
        uint8_t data[12];
        int vel = (int) (velCmd * 32767);
        int yaw = (int) (yawCmd * 32767);
        int angle = floatToIntBits(angleCmd);
        data[0] = 0x06;
        data[1] = 0x00;
        data[2] = ((vel >> 8) & 0xFF);
        data[3] = (vel & 0xFF);
        data[4] = ((yaw >> 8) & 0xFF);
        data[5] = (yaw & 0xFF);
        data[6] = ((angle >> 24) & 0xFF);
        data[7] = ((angle >> 16) & 0xFF);
        data[8] = ((angle >> 8) & 0xFF);
        data[9] = (angle & 0xFF);
        tkCrcInitialize();
        tkCrcComputeByteBufferCrc(data, 12);
        sendto(handle, (char*)data, sizeof(data), 0, (struct sockaddr*)&remaddr, addrlen);
}
