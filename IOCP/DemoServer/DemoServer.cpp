// DemoServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <iostream>
#include <string>
#include "IOCPHeader.h"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")      // Socket������õĶ�̬���ӿ�

HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

DWORD WINAPI ServerThread( LPVOID lpParam );

int main( int argc, char *argv[] )
{
	//////////////////////////////////////////////////////////////////////////  
	WSADATA wsaData;
	//�����׽��ֶ�̬���ӿ⣺int WSAStartup(...);
	if( 0 != WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) )
	{
		printf( "Using %s (Status:%s)\n", wsaData.szDescription, wsaData.szSystemStatus );
		printf( "with API versions: %d.%d to %d.%d",
			LOBYTE( wsaData.wVersion), HIBYTE( wsaData.wVersion ),
			LOBYTE( wsaData.wHighVersion), HIBYTE( wsaData.wHighVersion) );

		return -1;
	}
	else
	{
		printf("Windows sockets 2.2 startup\n");
	}
	//////////////////////////////////////////////////////////////////////////


	int nPort = 6000;

	// ������ɶ˿ڶ���
	// ���������̴߳�����ɶ˿ڶ�����¼�
	HANDLE hIocp = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, 0, 0, 0 );
	//�����̣߳�HANDLE CreateThread(...);
	::CreateThread( NULL, 0, ServerThread, (LPVOID)hIocp, 0, 0 );

	// ���������׽��֣��󶨱��ض˿ڣ���ʼ����
	SOCKET sListen = ::socket( AF_INET, SOCK_STREAM, 0 );

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons( nPort );
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	::bind( sListen, (sockaddr *)&addr, sizeof( addr ) );
	::listen( sListen, 5 );

	printf( "iocp demo start......\n" );

	// ѭ��������������
	while ( TRUE )
	{
		// �ȴ�����δ������������
		SOCKADDR_IN saRemote;
		int nRemoteLen = sizeof( saRemote );
		SOCKET sRemote = ::accept( sListen, (sockaddr *)&saRemote, &nRemoteLen );

		// ���ܵ�������֮��Ϊ������һ��per_handle���ݣ��������ǹ�������ɶ˿ڶ���
		PPER_HANDLE_DATA pPerHandle = ( PPER_HANDLE_DATA )::GlobalAlloc( GPTR, sizeof( PPER_HANDLE_DATA ) );
		if( pPerHandle == NULL )
		{
			break;
		}

		pPerHandle->s = sRemote;
		memcpy( &pPerHandle->addr, &saRemote, nRemoteLen );

		::CreateIoCompletionPort( ( HANDLE)pPerHandle->s, hIocp, (DWORD)pPerHandle, 0 );

		// Ͷ��һ����������
		PPER_IO_DATA pIoData = ( PPER_IO_DATA )::GlobalAlloc( GPTR, sizeof( PPER_IO_DATA ) );
		if( pIoData == NULL )
		{
			break;
		}

		pIoData->nOperationType = OP_READ;
		WSABUF buf;
		buf.buf = pIoData->buf;
		buf.len = BUFFER_SIZE;

		DWORD dwRecv = 0;
		DWORD dwFlags = 0;

		::WSARecv( pPerHandle->s, &buf, 1, &dwRecv, &dwFlags, &pIoData->ol, NULL );

	}

	//////////////////////////////////////////////////////////////////////////
	WSACleanup();
	//////////////////////////////////////////////////////////////////////////

	return 0;
}

/******************************************************************
* �������ܣ�������ɶ˿ڶ����¼����߳�
* ���������
* ���������
* ����ֵ ��
*******************************************************************/
DWORD WINAPI ServerThread( LPVOID lpParam )
{
	HANDLE hIocp = ( HANDLE )lpParam;
	if( hIocp == NULL )
	{
		cout<<"hIocp Error"<<endl;
		return -1;
	}
	cout<<"ServerThread Start"<<endl;
	DWORD dwTrans = 0;
	PPER_HANDLE_DATA pPerHandle;
	PPER_IO_DATA     pPerIo;
	LPOVERLAPPED IpOverlapped;


	while( TRUE )
	{
		cout<<"Start"<<endl;
		// �ڹ���������ɶ˿ڵ������׽����ϵȴ�I/O���
		BOOL bRet = ::GetQueuedCompletionStatus( hIocp, &dwTrans, (PULONG_PTR)&pPerHandle, (LPOVERLAPPED*)&IpOverlapped, INFINITE );
		cout<<"Start 2"<<endl;

		if( !bRet )     // ��������
		{
			::closesocket( pPerHandle->s );
			::GlobalFree( pPerHandle );
			::GlobalFree( IpOverlapped );

			cout << "GetQueuedCompletionStatus error" << endl;
			continue;
		}
		
		pPerIo = (PPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, ol);

		// �׽��ֱ��Է��ر�
		if( dwTrans == 0 && ( pPerIo->nOperationType == OP_READ || pPerIo->nOperationType == OP_WRITE ) )
		{
			::closesocket( pPerHandle->s );
			::GlobalFree( pPerHandle );
			::GlobalFree( pPerIo );

			cout << "client closed" << endl;
			continue;
		}
		
		cout<<"Case Start"<<endl;
		switch ( pPerIo->nOperationType )
		{
		case OP_READ:       // ���һ����������
			{
				// ��ʼ���ݴ����������Կͻ��˵�����
				WaitForSingleObject(hMutex,INFINITE);

				//pPerIo->buf[dwTrans] = '\0';
				strcpy(pPerIo->buf,"Good News!"); /*�����鸳�ַ���*/ 
				pPerIo->buf[strlen(pPerIo->buf)+1] = 0;
				printf( "OP_READ:%s\n", pPerIo->buf );
				ReleaseMutex(hMutex);


				// Ϊ��һ���ص����ý�����I/O��������
				ZeroMemory(&(pPerIo->ol), sizeof(OVERLAPPED)); // ����ڴ�
				pPerIo->buf[0]=0;

				// ����Ͷ�ݽ��ܲ���
				WSABUF buf;
				buf.buf = pPerIo->buf;
				buf.len = BUFFER_SIZE;
				pPerIo->nOperationType = OP_READ;

				DWORD dwRecv = 0;
				DWORD dwFlags = 0;

				::WSARecv( pPerHandle->s, &buf, 1, &dwRecv, &dwFlags, &pPerIo->ol, NULL );

			}
			break;
		case OP_WRITE:
			cout<<"Case OP_WRITE"<<endl;
			break;
		case OP_ACCEPT:
			cout<<"Case OP_ACCEPT"<<endl;
			break;
		default:
			cout<<"Case Error"<<endl;
			break;

		}
		
	}

	return 0;
}