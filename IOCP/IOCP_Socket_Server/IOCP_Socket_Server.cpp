// IOCP_Socket_Server.cpp : 定义控制台应用程序的入口点。
//http://blog.csdn.net/ggz631047367/article/details/45012993
// IOCP结合AcceptEx实例 

#include "stdafx.h"
// IOCP_TCPIP_Socket_Server.cpp

#include <WinSock2.h>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <mswsock.h>
#include <string>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")      // Socket编程需用的动态链接库
#pragma comment(lib, "Kernel32.lib")    // IOCP需要用到的动态链接库


#define SEND 0
#define RECV 1
#define ACCEPT 2


/**
* 结构体名称：PER_IO_DATA
* 结构体功能：重叠I/O需要用到的结构体，临时记录IO数据
**/
const int DataBuffSize = 2 * 1024;
typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[DataBuffSize];
	int BufferLen;
	int operationType;
	SOCKET client;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;

/**
* 结构体名称：PER_HANDLE_DATA
* 结构体存储：记录单个套接字的数据，包括了套接字的变量及套接字的对应的客户端的地址。
* 结构体作用：当服务器连接上客户端时，信息存储到该结构体中，知道客户端的地址以便于回访。
**/
typedef struct
{
	SOCKET socket;
	SOCKADDR_IN ClientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// 定义全局变量
const int DefaultPort = 6000;
vector < PER_HANDLE_DATA* > clientGroup;        // 记录客户端的向量组
int g_nThread = 0;//开启线程数量
HANDLE hThread[50];//线程句柄

SOCKET srvSocket = NULL;
DWORD dwBytes = 0;

HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);
DWORD WINAPI ServerSendThread(LPVOID IpParam);


LPFN_ACCEPTEX lpfnAcceptEx = NULL;//AcceptEx函数指针
GUID guidAcceptEx = WSAID_ACCEPTEX;
GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockAddrs = NULL;

// 开始主函数
int main()
{
	// 加载socket动态链接库
	WORD wVersionRequested = MAKEWORD(2, 2); // 请求2.2版本的WinSock库
	WSADATA wsaData;    // 接收Windows Socket的结构信息
	DWORD err = WSAStartup(wVersionRequested, &wsaData);

	if (0 != err) { // 检查套接字库是否申请成功
		cerr << "Request Windows Socket Library Error!\n";
		system("pause");
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {// 检查是否申请了所需版本的套接字库
		WSACleanup();
		cerr << "Request Windows Socket Version 2.2 Error!\n";
		system("pause");
		return -1;
	}

	// 创建IOCP的内核对象
	/**
	* 需要用到的函数的原型：
	* HANDLE WINAPI CreateIoCompletionPort(
	*    __in   HANDLE FileHandle,      // 已经打开的文件句柄或者空句柄，一般是客户端的句柄
	*    __in   HANDLE ExistingCompletionPort,  // 已经存在的IOCP句柄
	*    __in   ULONG_PTR CompletionKey,    // 完成键，包含了指定I/O完成包的指定文件
	*    __in   DWORD NumberOfConcurrentThreads // 真正并发同时执行最大线程数，一般推介是CPU核心数*2
	* );
	**/
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort) {   // 创建IO内核对象失败
		cerr << "CreateIoCompletionPort failed. Error:" << GetLastError() << endl;
		system("pause");
		return -1;
	}

	// 创建IOCP线程--线程里面创建线程池

	// 确定处理器的核心数量
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);

	// 基于处理器的核心数量创建线程
	for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i) {
		// 创建服务器工作器线程，并将完成端口传递到该线程
		HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, completionPort, 0, NULL);//第一NULL代表默认安全选项，第一个0，代表线程占用资源大小，第二个0，代表线程创建后立即执行
		if (NULL == ThreadHandle) {
			cerr << "Create Thread Handle failed. Error:" << GetLastError() << endl;
			system("pause");
			return -1;
		}
		hThread[i] = ThreadHandle;
		++g_nThread;
	}

	// 建立流式套接字
	srvSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);


	// Associate SOCKET with IOCP  
	if (NULL == CreateIoCompletionPort((HANDLE)srvSocket, completionPort, NULL, 0))
	{
		cout << "CreateIoCompletionPort failed with error code: " << WSAGetLastError() << endl;
		if (INVALID_SOCKET != srvSocket)
		{
			closesocket(srvSocket);
			srvSocket = INVALID_SOCKET;
		}
		return -1;
	}


	// 绑定SOCKET到本机
	SOCKADDR_IN srvAddr;
	srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(DefaultPort);
	int bindResult = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
	if (SOCKET_ERROR == bindResult) {
		cerr << "Bind failed. Error:" << GetLastError() << endl;
		system("pause");
		return -1;
	}

	// 将SOCKET设置为监听模式
	int listenResult = listen(srvSocket, 10);
	if (SOCKET_ERROR == listenResult) {
		cerr << "Listen failed. Error: " << GetLastError() << endl;
		system("pause");
		return -1;
	}

	// 开始处理IO数据
	cout << "本服务器已准备就绪，正在等待客户端的接入...\n";

	//// 创建用于发送数据的线程
	HANDLE sendThread = CreateThread(NULL, 0, ServerSendThread, 0, 0, NULL);//第二个0，代表回掉函数参数为0

	int number = 0;

	//lpfnAcceptEx不需要加循环体，而accept则需要循环体
	//for (int i = 0; i < 10; ++i)//while(true)时候会不断循环，而for循环时，会停在最后一个循环，即使循环数目会变也会停在左后一个
	//{

		PER_HANDLE_DATA * PerHandleData = NULL;
		SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == acceptSocket)
		{
			cerr << "WSASocket failed with error code: %d/n" << WSAGetLastError() << endl;
			return FALSE;
		}

		// 开始在接受套接字上处理I/O使用重叠I/O机制
		// 在新建的套接字上投递一个或多个异步
		// WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务    
		// 单I/O操作数据(I/O重叠)
		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = ACCEPT;  // read
		PerIoData->client = acceptSocket;

		if (SOCKET_ERROR == WSAIoctl(srvSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &lpfnAcceptEx,
			sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL))
		{
			cerr << "WSAIoctl failed with error code: " << WSAGetLastError() << endl;
			if (INVALID_SOCKET != srvSocket)
			{
				closesocket(srvSocket);
				srvSocket = INVALID_SOCKET;
			}
			//goto EXIT_CODE;
			return -1;
		}

		if (SOCKET_ERROR == WSAIoctl(srvSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs,
			sizeof(GuidGetAcceptExSockAddrs), &lpfnGetAcceptExSockAddrs, sizeof(lpfnGetAcceptExSockAddrs),
			&dwBytes, NULL, NULL))
		{
			cerr << "WSAIoctl failed with error code: " << WSAGetLastError() << endl;
			if (INVALID_SOCKET != srvSocket)
			{
				closesocket(srvSocket);
				srvSocket = INVALID_SOCKET;
			}
			//goto EXIT_CODE;
			return -1;
		}

		if (FALSE == lpfnAcceptEx(srvSocket, PerIoData->client, PerIoData->databuff.buf, PerIoData->databuff.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &(PerIoData->overlapped)))
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				cerr << "lpfnAcceptEx failed with error code: " << WSAGetLastError() << endl;

				return FALSE;
			}
		}
		number++;
		cout<<"Circle Number:"<<number<<endl;
	//}//while/for


	//Sleep(1000 * 6);
	PostQueuedCompletionStatus(completionPort, 0, NULL, NULL);
	WaitForMultipleObjects(g_nThread, hThread, TRUE, INFINITE);

	WSACleanup();
	system("pause");
	return 0;
}

// 开始服务工作线程函数
DWORD WINAPI ServerWorkThread(LPVOID IpParam)
{
	HANDLE CompletionPort = (HANDLE)IpParam;
	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	DWORD RecvBytes = 0;
	DWORD Flags = 0;
	BOOL bRet = false;

	while (true) 
	{
		bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);
		if (bRet == 0) 
		{
			if (WAIT_TIMEOUT == GetLastError())
			{
				continue;
			}
			// Error
			cout << "GetQueuedCompletionStatus failed with error:" << GetLastError() << endl;
			continue;
		}
		PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);
		//这个宏的作用是：根据一个结构体实例中的成员的地址，取到整个结构体实例的地址
		//PER_IO_DATA的成员overlapped的地址为&IpOverlapped，结果就可以获得PER_IO_DATA的地址

		if (NULL == PerIoData)
		{
			// Exit thread  
			break;
		}

		// 检查在套接字上是否有错误发生
		if (0 == BytesTransferred && (PerIoData->operationType == RECV || PerIoData->operationType == SEND))
		{
			closesocket(PerHandleData->socket);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);
			continue;
		}

		switch (PerIoData->operationType)
		{
		case ACCEPT:
			{
				SOCKADDR_IN* remote = NULL;
				SOCKADDR_IN* local = NULL;
				int remoteLen = sizeof(SOCKADDR_IN);
				int localLen = sizeof(SOCKADDR_IN);
				lpfnGetAcceptExSockAddrs(PerIoData->databuff.buf, PerIoData->databuff.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
					sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&local, &localLen, (LPSOCKADDR*)&remote, &remoteLen);
				//使用GetAcceptExSockaddrs函数 获得具体的各个地址参数.

				if (setsockopt(PerIoData->client, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
					(char*)&(PerHandleData->socket), sizeof(PerHandleData->socket)) == SOCKET_ERROR)
					cout << "setsockopt ERROR..." << endl;
				// 创建用来和套接字关联的单句柄数据信息结构
				PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));  // 在堆中为这个PerHandleData申请指定大小的内存
				PerHandleData->socket = PerIoData->client;

				//memcpy(&(perHandleData->clientAddr),raddr,sizeof(raddr));
				//将新的客户套接字与完成端口连接
				CreateIoCompletionPort((HANDLE)PerHandleData->socket,
					CompletionPort, (ULONG_PTR)PerHandleData, 0);

				PerHandleData->ClientAddr = *remote;
				clientGroup.push_back(PerHandleData);		// 将单个客户端数据指针放到客户端组中

				memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
				PerIoData->operationType = RECV;        //将状态设置成接收
				//设置WSABUF结构
				PerIoData->databuff.buf = PerIoData->buffer;
				PerIoData->databuff.len = PerIoData->BufferLen = 1024;

				cout << "wait for data arrive(Accept)..." << endl;
				Flags = 0;
				if (WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1,
					&RecvBytes, &Flags, &(PerIoData->overlapped), NULL) == SOCKET_ERROR)
					if (WSAGetLastError() == WSA_IO_PENDING)
						cout << "WSARecv Pending..." << endl;

				continue;
			}
			break;
		case RECV:
			// 开始数据处理，接收来自客户端的数据
			//WaitForSingleObject(hMutex, INFINITE);
			cout << "A Client says: " << PerIoData->databuff.buf << endl;
			//ReleaseMutex(hMutex);

			// 为下一个重叠调用建立单I/O操作数据
			ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存
			PerIoData->databuff.len = 1024;
			PerIoData->databuff.buf = PerIoData->buffer;//buf是个指针，这一过程会清空buffer的内容
			PerIoData->operationType = RECV;    // read
			WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);

			continue;
			break;
		default:
			break;
		}
	}

	return 0;
}


// 发送信息的线程执行函数
DWORD WINAPI ServerSendThread(LPVOID IpParam)
{
	while (1) {
		if (clientGroup.empty())
		{
		Sleep(5000);
		continue;
		}

		string talk;
		getline(cin, talk);
		//int len;
		// for (len = 0; talk[len] != '\0'; ++len) {
		//    // 找出这个字符组的长度
		//}
		//talk[len] = '\n';
		//talk[++len] = '\0';
		//printf("I Say:");
		cout<<"I Say: "<< talk<<" size: "<<clientGroup.size()<<endl;
		WaitForSingleObject(hMutex, INFINITE);
		for (unsigned i = 0; i < clientGroup.size(); ++i) {
			send(clientGroup[i]->socket, talk.c_str(), 200, 0); // 发送信息
		}
		ReleaseMutex(hMutex);
	}
	return 0;
}