#ifndef __IOCP_H__
#define __IOCP_H__

#include <winsock2.h>
#include <windows.h>
#include <Mswsock.h>

#define DefaultPort 6000
#define DataBuffSize 8 * 1024

typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	CHAR buffer[ DataBuffSize ];
	DWORD bytesSend;
	DWORD bytesRecv;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA;

typedef struct
{
	SOCKET socket;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

#endif