#ifndef _IOCPHEADER_H_20080916_
#define _IOCPHEADER_H_20080916_

#include <WINSOCK2.H>
#include <windows.h>

#define BUFFER_SIZE 1024

/******************************************************************
* per_handle 数据
*******************************************************************/
typedef struct _PER_HANDLE_DATA
{
	SOCKET      s;      // 对应的套接字句柄
	sockaddr_in addr;   // 对方的地址

}PER_HANDLE_DATA, *PPER_HANDLE_DATA;

/******************************************************************
* per_io 数据
*******************************************************************/
typedef struct _PER_IO_DATA
{
	OVERLAPPED ol;                 // 重叠结构
	char        buf[BUFFER_SIZE];   // 数据缓冲区
	int         nOperationType;     // 操作类型

#define OP_READ   1
#define OP_WRITE 2
#define OP_ACCEPT 3

}PER_IO_DATA, *PPER_IO_DATA;

#endif