// Test.cpp : �������̨Ӧ�ó������ڵ㡣
//http://www.cppfans.org/1267.html
//[���翪��]IOCP��������
//http://blog.csdn.net/lostyears/article/details/7436802

#include "stdafx.h"
#include "iocp.h"
#include <iostream>

using namespace std;

#pragma comment( lib, "Ws2_32.lib" )

DWORD WINAPI ServerWorkThread( LPVOID CompletionPortID );

void main()
{

	SOCKET acceptSocket;
	HANDLE completionPort;
	LPPER_HANDLE_DATA pHandleData;
	LPPER_IO_OPERATION_DATA pIoData;
	DWORD recvBytes;
	DWORD flags;

	WSADATA wsaData;
	DWORD ret;
	if ( ret = WSAStartup( 0x0202, &wsaData ) != 0 )
	{
		std::cout << "WSAStartup failed. Error:" << ret << std::endl;
		return;
	}
	//������ɶ˿�
	completionPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
	if ( completionPort == NULL )
	{
		std::cout << "CreateIoCompletionPort failed. Error:" << GetLastError() << std::endl;
		return;
	}

	SYSTEM_INFO mySysInfo;
	GetSystemInfo( &mySysInfo );

	// ���� 2 * CPU���� + 1 �������߳�
	DWORD threadID;
	for ( DWORD i = 0; i < ( mySysInfo.dwNumberOfProcessors * 2 + 1 ); ++i )
	{
		HANDLE threadHandle;
		threadHandle = CreateThread( NULL, 0, ServerWorkThread, completionPort, 0, &threadID );
		if ( threadHandle == NULL )
		{
			std::cout << "CreateThread failed. Error:" << GetLastError() << std::endl;
			return;
		}

		CloseHandle( threadHandle );
	}

	// ����һ������socket
	SOCKET listenSocket = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if ( listenSocket == INVALID_SOCKET )
	{
		std::cout << " WSASocket( listenSocket ) failed. Error:" << GetLastError() << std::endl;
		return;
	}

	SOCKADDR_IN internetAddr;
	internetAddr.sin_family = AF_INET;
	internetAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	internetAddr.sin_port = htons( DefaultPort );

	// �󶨼����˿�
	if ( bind( listenSocket, (PSOCKADDR)&internetAddr, sizeof( internetAddr ) ) == SOCKET_ERROR )
	{
		std::cout << "Bind failed. Error:" << GetLastError() << std::endl;
		return;
	}

	if ( listen( listenSocket, SOMAXCONN) ==  SOCKET_ERROR )  //backlog���ȴ����Ӷ��е���󳤶ȡ�
	{
		std::cout << "listen failed. Error:" << GetLastError() << std::endl;
		return;
	}

	// ��ʼ��ѭ������������
	while ( 1 )
	{
		acceptSocket = WSAAccept( listenSocket, NULL, NULL, NULL, 0 );
		if ( acceptSocket == SOCKET_ERROR )
		{
			std::cout << "WSAAccept failed. Error:" << GetLastError() << std::endl;
			return;
		}
		//�ú����Ӷ��з���һ����Ŀ���ֽ���UINTuFlags, 
		// UINTuFlags ��������(��ʽ)  
		//DWORD dwBytes ������ֽ���
		//����һ���·�����ڴ����ľ��
		pHandleData = (LPPER_HANDLE_DATA)GlobalAlloc( GPTR, sizeof( PER_HANDLE_DATA ) );
		if ( pHandleData = NULL )
		{
			std::cout << "GlobalAlloc( HandleData ) failed. Error:" << GetLastError() << std::endl;
			return;
		}

		pHandleData->socket = acceptSocket;
		//��SOCKET�������ɶ˿���ϵ����
		if ( CreateIoCompletionPort( (HANDLE)acceptSocket, completionPort, (ULONG_PTR)pHandleData, 0 ) == NULL )
		{
			std::cout << "CreateIoCompletionPort failed. Error:" << GetLastError() << std::endl;
			return;
		}

		pIoData = ( LPPER_IO_OPERATION_DATA )GlobalAlloc( GPTR, sizeof( PER_IO_OPERATEION_DATA ) );
		if ( pIoData == NULL )
		{
			std::cout << "GlobalAlloc( IoData ) failed. Error:" << GetLastError() << std::endl;
			return;
		}

		ZeroMemory( &( pIoData->overlapped ), sizeof( pIoData->overlapped ) );
		pIoData->bytesSend = 0;
		pIoData->bytesRecv = 0;
		pIoData->databuff.len = DataBuffSize;
		pIoData->databuff.buf = pIoData->buffer;

		flags = 0;
		if ( WSARecv( acceptSocket, &(pIoData->databuff), 1, &recvBytes, &flags, &(pIoData->overlapped), NULL ) == SOCKET_ERROR )
		{
			if ( WSAGetLastError() != ERROR_IO_PENDING )
			{
				std::cout << "WSARecv() failed. Error:" << GetLastError() << std::endl;
				return;
			}
			else
			{
				std::cout << "WSARecv() io pending" << std::endl;
				return;
			}
		}
	}
}

DWORD WINAPI ServerWorkThread( LPVOID CompletionPortID )
{
	HANDLE complationPort = (HANDLE)CompletionPortID;
	DWORD bytesTransferred;
	LPPER_HANDLE_DATA pHandleData = NULL;
	LPPER_IO_OPERATION_DATA pIoData = NULL;
	DWORD sendBytes = 0;
	DWORD recvBytes = 0;
	DWORD flags;

	while ( 1 )
	{
		if ( GetQueuedCompletionStatus( complationPort, &bytesTransferred, (PULONG_PTR)&pHandleData, (LPOVERLAPPED *)&pIoData, INFINITE ) == 0 )
		{
			std::cout << "GetQueuedCompletionStatus failed. Error:" << GetLastError() << std::endl;
			return 0;
		}

		// ��������Ƿ��Ѿ���������
		if ( bytesTransferred == 0 )
		{
			std::cout << " Start closing socket..." << std::endl;
			if ( CloseHandle( (HANDLE)pHandleData->socket ) == SOCKET_ERROR )
			{
				std::cout << "Close socket failed. Error:" << GetLastError() << std::endl;
				return 0;
			}

			GlobalFree( pHandleData );
			GlobalFree( pIoData );
			continue;
		}

		// ���ܵ����Ƿ�������
		if ( pIoData->bytesRecv == 0 )
		{
			pIoData->bytesRecv = bytesTransferred;
			pIoData->bytesSend = 0;
		}
		else
		{
			pIoData->bytesSend += bytesTransferred;
		}

		// ����û�з��꣬��������
		if ( pIoData->bytesRecv > pIoData->bytesSend )
		{
			ZeroMemory( &(pIoData->overlapped), sizeof( OVERLAPPED ) );
			pIoData->databuff.buf = pIoData->buffer + pIoData->bytesSend;
			pIoData->databuff.len = pIoData->bytesRecv - pIoData->bytesSend;

			// �������ݳ�ȥ
			if ( WSASend( pHandleData->socket, &(pIoData->databuff), 1, &sendBytes, 0, &(pIoData->overlapped), NULL ) == SOCKET_ERROR )
			{
				if ( WSAGetLastError() != ERROR_IO_PENDING )
				{
					std::cout << "WSASend() failed. Error:" << GetLastError() << std::endl;
					return 0;
				}
				else
				{
					std::cout << "WSASend() failed. io pending. Error:" << GetLastError() << std::endl;
					return 0;
				}
			}

			std::cout << "Send " << pIoData->buffer << std::endl;
		}
		else
		{
			pIoData->bytesRecv = 0;
			flags = 0;

			ZeroMemory( &(pIoData->overlapped), sizeof( OVERLAPPED ) );
			pIoData->databuff.len = DataBuffSize;
			pIoData->databuff.buf = pIoData->buffer;

			if ( WSARecv( pHandleData->socket, &(pIoData->databuff), 1, &recvBytes, &flags, &(pIoData->overlapped), NULL ) == SOCKET_ERROR )
			{
				if ( WSAGetLastError() != ERROR_IO_PENDING )
				{
					std::cout << "WSARecv() failed. Error:" << GetLastError() << std::endl;
					return 0;
				}
				else
				{
					std::cout << "WSARecv() io pending" << std::endl;
					return 0;
				}
			}
		}
	}
}
