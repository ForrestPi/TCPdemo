// DemoServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <string>
#include "IOCPHeader.h"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")      // Socket编程需用的动态链接库

HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

DWORD WINAPI ServerThread( LPVOID lpParam );

int main( int argc, char *argv[] )
{
	//////////////////////////////////////////////////////////////////////////  
	WSADATA wsaData;
	//链接套接字动态链接库：int WSAStartup(...);
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

	// 创建完成端口对象
	// 创建工作线程处理完成端口对象的事件
	HANDLE hIocp = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, 0, 0, 0 );
	//创建线程：HANDLE CreateThread(...);
	::CreateThread( NULL, 0, ServerThread, (LPVOID)hIocp, 0, 0 );

	// 创建监听套接字，绑定本地端口，开始监听
	SOCKET sListen = ::socket( AF_INET, SOCK_STREAM, 0 );

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons( nPort );
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	::bind( sListen, (sockaddr *)&addr, sizeof( addr ) );
	::listen( sListen, 5 );

	printf( "iocp demo start......\n" );

	// 循环处理到来的请求
	while ( TRUE )
	{
		// 等待接受未决的连接请求
		SOCKADDR_IN saRemote;
		int nRemoteLen = sizeof( saRemote );
		SOCKET sRemote = ::accept( sListen, (sockaddr *)&saRemote, &nRemoteLen );

		// 接受到新连接之后，为它创建一个per_handle数据，并将他们关联到完成端口对象
		PPER_HANDLE_DATA pPerHandle = ( PPER_HANDLE_DATA )::GlobalAlloc( GPTR, sizeof( PPER_HANDLE_DATA ) );
		if( pPerHandle == NULL )
		{
			break;
		}

		pPerHandle->s = sRemote;
		memcpy( &pPerHandle->addr, &saRemote, nRemoteLen );

		::CreateIoCompletionPort( ( HANDLE)pPerHandle->s, hIocp, (DWORD)pPerHandle, 0 );

		// 投递一个接受请求
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
* 函数介绍：处理完成端口对象事件的线程
* 输入参数：
* 输出参数：
* 返回值 ：
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
		// 在关联到此完成端口的所有套接字上等待I/O完成
		BOOL bRet = ::GetQueuedCompletionStatus( hIocp, &dwTrans, (PULONG_PTR)&pPerHandle, (LPOVERLAPPED*)&IpOverlapped, INFINITE );
		cout<<"Start 2"<<endl;

		if( !bRet )     // 发生错误
		{
			::closesocket( pPerHandle->s );
			::GlobalFree( pPerHandle );
			::GlobalFree( IpOverlapped );

			cout << "GetQueuedCompletionStatus error" << endl;
			continue;
		}
		
		pPerIo = (PPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, ol);

		// 套接字被对方关闭
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
		case OP_READ:       // 完成一个接收请求
			{
				// 开始数据处理，接收来自客户端的数据
				WaitForSingleObject(hMutex,INFINITE);

				//pPerIo->buf[dwTrans] = '\0';
				strcpy(pPerIo->buf,"Good News!"); /*给数组赋字符串*/ 
				pPerIo->buf[strlen(pPerIo->buf)+1] = 0;
				printf( "OP_READ:%s\n", pPerIo->buf );
				ReleaseMutex(hMutex);


				// 为下一个重叠调用建立单I/O操作数据
				ZeroMemory(&(pPerIo->ol), sizeof(OVERLAPPED)); // 清空内存
				pPerIo->buf[0]=0;

				// 继续投递接受操作
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