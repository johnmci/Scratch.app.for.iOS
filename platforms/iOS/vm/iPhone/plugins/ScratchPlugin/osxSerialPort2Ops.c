/*
	unixSerialPort2Ops.c

	Support for SerialPort2 primitives under Unix, including OSX and Linux.
	Based in part on Apple's SerialPortSample.c.

	John Maloney, April 10, 2007
 
	Hacked for Scratch on iPad by John M McIntosh Jan 28th, 2010
 
*/

#include "scratchOps.h"

int SerialPortCount(void) {
	return 0;
}

// Find the name of the given port number. Fill in bsdPath if successful.
// Otherwise, make bsdPath be the empty string.
void SerialPortName(int portIndex, char *bsdPath, int maxPathSize) {
	*bsdPath = '\0';	// result is empty string if port not found
}

int SerialPortOpenPortNamed(char *portName, int baudRate) {
	return 0;
}

void SerialPortClose(int portNum) {
}

int SerialPortIsOpen(int portNum) {
	return 0;
}

int SerialPortRead(int portNum, char *bufPtr, int bufSize) {
	return 0;
}

int SerialPortWrite(int portNum, char *bufPtr, int bufSize) {
	return 0;
}

// Port options for SetOption/GetOption:
//	1. baud rate
//	2. data bits
//	3. stop bits
//	4. parity type
//	5. input flow control type
//	6. output flow control type
//	20-25: handshake line bits (DTR, RTS, CTS, DSR, CD, RD)

int SerialPortSetOption(int portNum, int optionNum, int newValue) {
		return 0;
}

int SerialPortGetOption(int portNum, int optionNum) {
	return 0;
}