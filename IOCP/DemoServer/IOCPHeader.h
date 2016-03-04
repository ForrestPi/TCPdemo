#ifndef _IOCPHEADER_H_20080916_
#define _IOCPHEADER_H_20080916_

#include <WINSOCK2.H>
#include <windows.h>

#define BUFFER_SIZE 1024

/******************************************************************
* per_handle ����
*******************************************************************/
typedef struct _PER_HANDLE_DATA
{
	SOCKET      s;      // ��Ӧ���׽��־��
	sockaddr_in addr;   // �Է��ĵ�ַ

}PER_HANDLE_DATA, *PPER_HANDLE_DATA;

/******************************************************************
* per_io ����
*******************************************************************/
typedef struct _PER_IO_DATA
{
	OVERLAPPED ol;                 // �ص��ṹ
	char        buf[BUFFER_SIZE];   // ���ݻ�����
	int         nOperationType;     // ��������

#define OP_READ   1
#define OP_WRITE 2
#define OP_ACCEPT 3

}PER_IO_DATA, *PPER_IO_DATA;

#endif