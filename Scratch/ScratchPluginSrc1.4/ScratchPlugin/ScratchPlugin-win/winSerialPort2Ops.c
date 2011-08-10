/*
	winSerialPort2Ops.c

	Support for SerialPort2 primitives under Windows.
	Based in part on sqWin32SerialPort.c by Andreas Raab.

	John Maloney, April 10, 2007.
*/

#include "scratchOps.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>

// helper functions
int digitValue(int);
int openPort(int portNum, int baudRate);

// port handles for open serial ports (0 means not open)
// Note: Squeak and COM port numbers are one-based; COM1 is serialPorts[0]
#define PORT_COUNT 32
static HANDLE serialPorts[PORT_COUNT] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0};

#define PRIM_FAILED -1

int SerialPortCount(void) { return PORT_COUNT; }

// Find the name of the given port number. Fill in portName if successful.
// Otherwise, make portName be the empty string.
void SerialPortName(int portIndex, char *portName, int maxPathSize) {
	*portName = '\0';	// result is empty string if portIndex is out of range not found
	if ((maxPathSize < 6) || (portIndex > PORT_COUNT)) return;
	sprintf(portName, "COM%d", portIndex);
}

int SerialPortOpenPortNamed(char *portName, int baudRate) {
	int portNum;
	int n = strlen(portName);
	if ((n < 4) || (n > 5)) return PRIM_FAILED;
	// assume portName is: COM<num>, where <num> is or two decimal digits
	if (digitValue(portName[3]) < 0) return PRIM_FAILED;
	portNum = digitValue(portName[3]);
	if (n == 5) {
		if (digitValue(portName[4]) < 0) return PRIM_FAILED;
		portNum = (10 * portNum) + digitValue(portName[4]);
	}
	return openPort(portNum, baudRate);
}

void SerialPortClose(int portNum) {
	HANDLE portPtr;
	
	if (!SerialPortIsOpen(portNum)) return;

	portPtr = serialPorts[portNum - 1];
	PurgeComm(portPtr, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	CloseHandle(portPtr);
	serialPorts[portNum - 1] = 0;
}

int SerialPortIsOpen(int portNum) {
	if ((portNum <= 0) || (portNum > PORT_COUNT)) return 0;
	return serialPorts[portNum - 1] != 0;
}

int SerialPortRead(int portNum, char *bufPtr, int bufSize) {
	HANDLE portPtr;
	DWORD bytesRead = 0;

	if (!SerialPortIsOpen(portNum)) return PRIM_FAILED;

	portPtr = serialPorts[portNum - 1];
	if (!ReadFile(portPtr, (void *) bufPtr, bufSize, &bytesRead, NULL)) {
		return PRIM_FAILED;
	}

	return bytesRead;
}

int SerialPortWrite(int portNum, char *bufPtr, int bufSize) {
	HANDLE portPtr;
	DWORD bytesWritten = 0;

	if (!SerialPortIsOpen(portNum)) return PRIM_FAILED;

	portPtr = serialPorts[portNum - 1];
	if (!WriteFile(portPtr, (void *) bufPtr, bufSize, &bytesWritten, NULL)) {
		return PRIM_FAILED;
	}

	return bytesWritten;
}

// Port options for Set/GetOption:
//	1. baud rate
//	2. data bits
//	3. stop bits
//	4. parity type
//	5. input flow control type
//	6. output flow control type
//	20-25: handshake line bits (DTR, RTS, CTS, DSR, CD, RD)

int SerialPortSetOption(int portNum, int optionNum, int newValue) {
	HANDLE portPtr;
	DCB dcb;

	if (!SerialPortIsOpen(portNum)) return PRIM_FAILED;
	portPtr = serialPorts[portNum - 1];

	// Get current state
	ZeroMemory(&dcb, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	if (!GetCommState(portPtr, &dcb)) return PRIM_FAILED;

	switch (optionNum) {
	case 1:	// baud rate
		dcb.BaudRate = newValue;
		break;
	case 2:	// # of data bits
		dcb.ByteSize = newValue;
		break;
	case 3:	// 1 or 2 stop bits
		dcb.StopBits = (newValue == 1) ? 0 : 2; // StopBits=0 means one stop bit
		break;
	case 4:	// parity
		dcb.Parity = newValue;
		break;
	case 5:	// input flow control
		dcb.fInX = FALSE;
		if (dcb.fRtsControl == RTS_CONTROL_HANDSHAKE) dcb.fRtsControl = RTS_CONTROL_ENABLE;
		if (newValue == 1) dcb.fInX = TRUE;							// enable xon/xoff input flow control
		if (newValue == 2) dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;	// enable hardware input flow control
		break;
	case 6:	// output flow control
		dcb.fOutX = FALSE;
		dcb.fOutxCtsFlow = FALSE;
		if (newValue == 1) dcb.fOutX = TRUE;		// enable xon/xoff output flow control
		if (newValue == 2) dcb.fOutxCtsFlow = TRUE;	// enable hardware output flow control
		break;
	case 20: // set DTR line state
		EscapeCommFunction(portPtr, (newValue == 0) ? CLRDTR: SETDTR);
		return 0;
	case 21: // set RTS line state
		EscapeCommFunction(portPtr, (newValue == 0) ? CLRRTS : SETRTS);
		return 0;
	default:
		return PRIM_FAILED;
	}
	if (!SetCommState(portPtr, &dcb)) return PRIM_FAILED;
	return 0;
}

int SerialPortGetOption(int portNum, int optionNum) {
	HANDLE portPtr;
	DCB dcb;
	DWORD handshake;

	if (!SerialPortIsOpen(portNum)) return PRIM_FAILED;
	portPtr = serialPorts[portNum - 1];

	// Get current state
	ZeroMemory(&dcb, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	if (!GetCommState(portPtr, &dcb)) return PRIM_FAILED;
	if (!GetCommModemStatus(portPtr, &handshake)) return PRIM_FAILED;

	switch (optionNum) {
	case 1:	// baud rate
		return dcb.BaudRate;
	case 2:	// # of data bits
		return dcb.ByteSize;
	case 3:	// 1 or 2 stop bits
		return (dcb.StopBits == 0) ? 1 : 2; // StopBits=0 means one stop bit
	case 4:	// parity
		return dcb.Parity;
	case 5:	// input flow control
		if (dcb.fRtsControl == RTS_CONTROL_HANDSHAKE) return 2;
		if (dcb.fInX) return 1;
		return 0;
	case 6:	// output flow control
		if (dcb.fOutxCtsFlow) return 2;
		if (dcb.fOutX) return 1;
		return 0;
	case 20: return PRIM_FAILED;	// DTR is output only
	case 21: return PRIM_FAILED;	// RTS is output only
	case 22: return (handshake & MS_CTS_ON) > 0;
	case 23: return (handshake & MS_DSR_ON) > 0;
	case 24: return (handshake & MS_RLSD_ON) > 0; // RLSD = carrier detect
	case 25: return (handshake & MS_RING_ON) > 0;
	}
	return PRIM_FAILED;
}

//****************************************************************************
//  helper functions
//*****************************************************************************

int digitValue(int ch) {
	if ((ch >= '0') && (ch <= '9')) return ch - '0';
	return -10;
}

int openPort(int portNum, int baudRate) {
	TCHAR name[40];
	HANDLE portPtr;
	COMMTIMEOUTS timeouts;
	DCB dcb;

	if ((portNum <= 0) || (portNum > PORT_COUNT)) return PRIM_FAILED;
	if (serialPorts[portNum - 1] != 0) return PRIM_FAILED;

	wsprintf(name, TEXT("\\\\.\\COM%d"), portNum);
	// MessageBox(NULL, name, "Debug: port name", 0);
	portPtr = CreateFile(
		name,
		GENERIC_READ | GENERIC_WRITE,
		0,				// comm devices must be opened with exclusive access
		NULL,			// no security attrs
		OPEN_EXISTING,	// comm devices must use OPEN_EXISTING
		0,				// no overlapped I/O
		NULL			// hTemplate must be NULL for comm devices
	);

	if (portPtr == INVALID_HANDLE_VALUE) return PRIM_FAILED;

	// purge the driver
	PurgeComm(portPtr, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	// Set input and output buffer sizes
	SetupComm(portPtr, 4096, 4096);

	// Set the timeouts so that reads do not block
	timeouts.ReadIntervalTimeout= 0xFFFFFFFF;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(portPtr, &timeouts);

	// Set default DCB settings
	ZeroMemory(&dcb, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	if (!GetCommState(portPtr, &dcb)) return PRIM_FAILED;

	// the basics
	dcb.BaudRate = baudRate;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;

	// no control flow by defaut
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;	// turn on DTR line
	dcb.fRtsControl = RTS_CONTROL_ENABLE;	// turn on RTS line
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fNull = FALSE;
	dcb.XonChar = 17;
	dcb.XoffChar = 19;

	if (!SetCommState(portPtr, &dcb)) {
		CloseHandle(portPtr);
		return PRIM_FAILED;
	}

	serialPorts[portNum - 1] = portPtr;
	return portNum;
}
