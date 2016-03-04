// Client.cpp : 定义控制台应用程序的入口点。
//http://www.cppblog.com/niewenlong/archive/2007/08/17/30224.html
//IOCP的例子 

#include "stdafx.h"

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")		// Socket编程需用的动态链接库

//---------------------------------------------------------------------------

SOCKET sockClient;
struct sockaddr_in addrServer;
char buf[24];
int n = 0;
int Init();

int main(int argc, char* argv[])
{
	if(Init() != 0)
		goto theend;

	sockClient = socket(AF_INET,SOCK_STREAM,0);
	if(sockClient == INVALID_SOCKET)
	{
		cout<<"socket 失败"<<endl;
		WSACleanup();
		goto theend;
	}
	memset(&addrServer,0,sizeof(sockaddr_in));
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	addrServer.sin_port = htons(6000);
	cout<<"连接服务器..."<<endl;
	if(connect(sockClient,(const struct sockaddr *)&addrServer,sizeof(sockaddr)) != 0)
	{
		cout<<"connect 失败"<<endl;
		WSACleanup();
		goto theend;
	}
	cout<<"开始发送测试包"<<endl;
	memset(buf,0,24);
	while(true)
	{
		sprintf(buf,"第%d个包", n);
		cout<<"发送："<<buf<<endl;
		if(send(sockClient,buf,strlen(buf),0) <= 0)
		{
			cout<<"send失败,可能连接断开"<<endl;
			//break;
			goto theend;
		}
		memset(buf,0,24);

		//接收服务端应答
		if(recv(sockClient,buf,24,0) <= 0)
		{
			cout<<"recv失败,可能连接断开"<<endl;
			//break;
			goto theend;
		}
		cout<<"服务器应答："<<buf<<endl;
		memset(buf,0,24);

		Sleep(200);
		n++;
	}

theend:
	WSACleanup();
	getchar();
	return 0;
}
//---------------------------------------------------------------------------
int Init()
{
	WSAData wsaData;
	if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
	{
		cout<<"WSAStartup失败"<<endl;
		return -1;
	}

	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		cout<<"SOCKET版本不对"<<endl;
		WSACleanup();
		return -1;
	}
	return 0;
}
