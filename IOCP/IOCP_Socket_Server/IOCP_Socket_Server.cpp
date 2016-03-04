// IOCP_Socket_Server.cpp : �������̨Ӧ�ó������ڵ㡣
//http://blog.csdn.net/ggz631047367/article/details/45012993
// IOCP���AcceptExʵ�� 

#include "stdafx.h"
// IOCP_TCPIP_Socket_Server.cpp

#include <WinSock2.h>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <mswsock.h>
#include <string>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")      // Socket������õĶ�̬���ӿ�
#pragma comment(lib, "Kernel32.lib")    // IOCP��Ҫ�õ��Ķ�̬���ӿ�


#define SEND 0
#define RECV 1
#define ACCEPT 2


/**
* �ṹ�����ƣ�PER_IO_DATA
* �ṹ�幦�ܣ��ص�I/O��Ҫ�õ��Ľṹ�壬��ʱ��¼IO����
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
* �ṹ�����ƣ�PER_HANDLE_DATA
* �ṹ��洢����¼�����׽��ֵ����ݣ��������׽��ֵı������׽��ֵĶ�Ӧ�Ŀͻ��˵ĵ�ַ��
* �ṹ�����ã��������������Ͽͻ���ʱ����Ϣ�洢���ýṹ���У�֪���ͻ��˵ĵ�ַ�Ա��ڻطá�
**/
typedef struct
{
	SOCKET socket;
	SOCKADDR_IN ClientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// ����ȫ�ֱ���
const int DefaultPort = 6000;
vector < PER_HANDLE_DATA* > clientGroup;        // ��¼�ͻ��˵�������
int g_nThread = 0;//�����߳�����
HANDLE hThread[50];//�߳̾��

SOCKET srvSocket = NULL;
DWORD dwBytes = 0;

HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);
DWORD WINAPI ServerSendThread(LPVOID IpParam);


LPFN_ACCEPTEX lpfnAcceptEx = NULL;//AcceptEx����ָ��
GUID guidAcceptEx = WSAID_ACCEPTEX;
GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockAddrs = NULL;

// ��ʼ������
int main()
{
	// ����socket��̬���ӿ�
	WORD wVersionRequested = MAKEWORD(2, 2); // ����2.2�汾��WinSock��
	WSADATA wsaData;    // ����Windows Socket�Ľṹ��Ϣ
	DWORD err = WSAStartup(wVersionRequested, &wsaData);

	if (0 != err) { // ����׽��ֿ��Ƿ�����ɹ�
		cerr << "Request Windows Socket Library Error!\n";
		system("pause");
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {// ����Ƿ�����������汾���׽��ֿ�
		WSACleanup();
		cerr << "Request Windows Socket Version 2.2 Error!\n";
		system("pause");
		return -1;
	}

	// ����IOCP���ں˶���
	/**
	* ��Ҫ�õ��ĺ�����ԭ�ͣ�
	* HANDLE WINAPI CreateIoCompletionPort(
	*    __in   HANDLE FileHandle,      // �Ѿ��򿪵��ļ�������߿վ����һ���ǿͻ��˵ľ��
	*    __in   HANDLE ExistingCompletionPort,  // �Ѿ����ڵ�IOCP���
	*    __in   ULONG_PTR CompletionKey,    // ��ɼ���������ָ��I/O��ɰ���ָ���ļ�
	*    __in   DWORD NumberOfConcurrentThreads // ��������ͬʱִ������߳�����һ���ƽ���CPU������*2
	* );
	**/
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort) {   // ����IO�ں˶���ʧ��
		cerr << "CreateIoCompletionPort failed. Error:" << GetLastError() << endl;
		system("pause");
		return -1;
	}

	// ����IOCP�߳�--�߳����洴���̳߳�

	// ȷ���������ĺ�������
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);

	// ���ڴ������ĺ������������߳�
	for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i) {
		// �����������������̣߳�������ɶ˿ڴ��ݵ����߳�
		HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, completionPort, 0, NULL);//��һNULL����Ĭ�ϰ�ȫѡ���һ��0�������߳�ռ����Դ��С���ڶ���0�������̴߳���������ִ��
		if (NULL == ThreadHandle) {
			cerr << "Create Thread Handle failed. Error:" << GetLastError() << endl;
			system("pause");
			return -1;
		}
		hThread[i] = ThreadHandle;
		++g_nThread;
	}

	// ������ʽ�׽���
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


	// ��SOCKET������
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

	// ��SOCKET����Ϊ����ģʽ
	int listenResult = listen(srvSocket, 10);
	if (SOCKET_ERROR == listenResult) {
		cerr << "Listen failed. Error: " << GetLastError() << endl;
		system("pause");
		return -1;
	}

	// ��ʼ����IO����
	cout << "����������׼�����������ڵȴ��ͻ��˵Ľ���...\n";

	//// �������ڷ������ݵ��߳�
	HANDLE sendThread = CreateThread(NULL, 0, ServerSendThread, 0, 0, NULL);//�ڶ���0������ص���������Ϊ0

	int number = 0;

	//lpfnAcceptEx����Ҫ��ѭ���壬��accept����Ҫѭ����
	//for (int i = 0; i < 10; ++i)//while(true)ʱ��᲻��ѭ������forѭ��ʱ����ͣ�����һ��ѭ������ʹѭ����Ŀ���Ҳ��ͣ�����һ��
	//{

		PER_HANDLE_DATA * PerHandleData = NULL;
		SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == acceptSocket)
		{
			cerr << "WSASocket failed with error code: %d/n" << WSAGetLastError() << endl;
			return FALSE;
		}

		// ��ʼ�ڽ����׽����ϴ���I/Oʹ���ص�I/O����
		// ���½����׽�����Ͷ��һ�������첽
		// WSARecv��WSASend������ЩI/O������ɺ󣬹������̻߳�ΪI/O�����ṩ����    
		// ��I/O��������(I/O�ص�)
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

// ��ʼ�������̺߳���
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
		//�����������ǣ�����һ���ṹ��ʵ���еĳ�Ա�ĵ�ַ��ȡ�������ṹ��ʵ���ĵ�ַ
		//PER_IO_DATA�ĳ�Աoverlapped�ĵ�ַΪ&IpOverlapped������Ϳ��Ի��PER_IO_DATA�ĵ�ַ

		if (NULL == PerIoData)
		{
			// Exit thread  
			break;
		}

		// ������׽������Ƿ��д�����
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
				//ʹ��GetAcceptExSockaddrs���� ��þ���ĸ�����ַ����.

				if (setsockopt(PerIoData->client, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
					(char*)&(PerHandleData->socket), sizeof(PerHandleData->socket)) == SOCKET_ERROR)
					cout << "setsockopt ERROR..." << endl;
				// �����������׽��ֹ����ĵ����������Ϣ�ṹ
				PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));  // �ڶ���Ϊ���PerHandleData����ָ����С���ڴ�
				PerHandleData->socket = PerIoData->client;

				//memcpy(&(perHandleData->clientAddr),raddr,sizeof(raddr));
				//���µĿͻ��׽�������ɶ˿�����
				CreateIoCompletionPort((HANDLE)PerHandleData->socket,
					CompletionPort, (ULONG_PTR)PerHandleData, 0);

				PerHandleData->ClientAddr = *remote;
				clientGroup.push_back(PerHandleData);		// �������ͻ�������ָ��ŵ��ͻ�������

				memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
				PerIoData->operationType = RECV;        //��״̬���óɽ���
				//����WSABUF�ṹ
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
			// ��ʼ���ݴ����������Կͻ��˵�����
			//WaitForSingleObject(hMutex, INFINITE);
			cout << "A Client says: " << PerIoData->databuff.buf << endl;
			//ReleaseMutex(hMutex);

			// Ϊ��һ���ص����ý�����I/O��������
			ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
			PerIoData->databuff.len = 1024;
			PerIoData->databuff.buf = PerIoData->buffer;//buf�Ǹ�ָ�룬��һ���̻����buffer������
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


// ������Ϣ���߳�ִ�к���
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
		//    // �ҳ�����ַ���ĳ���
		//}
		//talk[len] = '\n';
		//talk[++len] = '\0';
		//printf("I Say:");
		cout<<"I Say: "<< talk<<" size: "<<clientGroup.size()<<endl;
		WaitForSingleObject(hMutex, INFINITE);
		for (unsigned i = 0; i < clientGroup.size(); ++i) {
			send(clientGroup[i]->socket, talk.c_str(), 200, 0); // ������Ϣ
		}
		ReleaseMutex(hMutex);
	}
	return 0;
}