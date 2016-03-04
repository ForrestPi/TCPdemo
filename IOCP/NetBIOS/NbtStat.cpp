// NbtStat.cpp -- 获取网络中指定计算机的基本信息

#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <map>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;


class CDevice
{
public:
	CDevice(void){}
	CDevice(string ip):IP(ip), Name (""), Mac(""), Workgroup(""){}

public:
	~CDevice(void){}

public:
	string IP;			// IP地址
	string Name;		// 名称
	string Mac;			// Mac地址
	string Workgroup;	// 工作组
};



// 通过NbtStat获取计算机名字信息的结构体
struct names
{
	unsigned char nb_name[16];	// 表示接收到的名字
	unsigned short name_flags;	// 标识名字的含义
};


// 保存获取NetBIOS信息的Socket和IP地址列表
struct workstationNameThreadStruct
{
	SOCKET s;					// 指定发送和接收NetBIOS数据包的Socket
	std::map<unsigned long, CDevice*> *ips;	// 指定获取NetBIOS信息的IP地址列表
};

// 格式化ethernet中的字节为字符串
void GetEthernetAdapter(unsigned char *ethernet, char *macstr)
{
	sprintf(macstr, "%02x %02x %02x %02x %02x %02x",
		ethernet[0],	ethernet[1],		ethernet[2],
		ethernet[3],	ethernet[4],		ethernet[5]
	);
	return;
}


// 获取设备的名称和MAC地址等信息
/**
 * GetHostInfo()函数的运行过程
 * 创建发送和接收NetBIOS数据包的Socket, 并将其绑定在本地地址的端口0上。
 * 创建接收NetBIOS回应包的线程，参数unionStruct中包含要获取NetBIOS信息的IP地址和通信用的Socket。
 * 使用for循环语句依次向每个IP地址发送NetBIOS请求包，请求包数据保存在字符数组input中。
 * 调用WaitForSingleObject()函数，等待接收线程结束。
 * 如果超时，则结束接收线程NetBiosRecvThreadProc()函数。
 * 关闭线程句柄，释放资源。
 **/
void GetHostInfo(std::map<unsigned long, CDevice*> &ips, int timeout)
{
	DWORD WINAPI NetBiosRecvThreadProc(void *param);

	const int defaultPort = 0;      // 设定默认的绑定端口
	SOCKET sock;					// 通信套接字
	struct sockaddr_in origen;		// 本地地址
	WSADATA wsaData;				// Windows Sockets环境变量

	if(WSAStartup(MAKEWORD(2,1),&wsaData) != 0){// 初始化Windows Sockets环境
		return;
	}
	if(INVALID_SOCKET ==(sock = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP))){// 创建TCP/IP套接字
		return;
	}
	// 设置超时时间
	if(SOCKET_ERROR ==setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))){
		closesocket(sock);
		WSACleanup();
		return;
	}
	// 将套接字绑定到本地地址和端口0
	memset(&origen, 0, sizeof(origen));
	origen.sin_family = AF_INET;
	origen.sin_addr.s_addr = htonl (INADDR_ANY);
	origen.sin_port = htons (defaultPort);
	if (bind (sock, (struct sockaddr *) &origen, sizeof(origen)) < 0) { // 套接字连接
		closesocket(sock);
		WSACleanup();
		return;
	}

	// 为创建接收线程准备数据
	workstationNameThreadStruct unionStruct;
	unionStruct.ips = &ips;
	unionStruct.s = sock;
	// 启动线程等待接收NetBIOS回应包
	DWORD pid;
	HANDLE threadHandle = CreateThread(NULL, 0, NetBiosRecvThreadProc,(void *)&unionStruct, 0, &pid);

	// 依次向ips中的每个IP地址的137端口发送NetBIOS请求包（保存在input字符数组中）
	std::map<unsigned long, CDevice*>::iterator itr;
	for(itr=ips.begin();itr != ips.end();itr++)
	{
		char input[]="\x80\x94\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x20\x43\x4b\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x00\x00\x21\x00\x01";
		struct sockaddr_in dest;
		// 发送NetBios请求信息
		memset(&dest,0,sizeof(dest));
		dest.sin_addr.s_addr = itr->first;
		dest.sin_family = AF_INET;
		dest.sin_port = htons(137);
		sendto (sock, input, sizeof(input)-1, 0, (struct sockaddr *)&dest, sizeof (dest));
	}
	// 等待接收线程NetBiosRecvThreadProc结束
	DWORD ret = WaitForSingleObject(threadHandle, timeout * 4);
	// 如果超时，则结束接收线程NetBiosRecvThreadProc
	if(ret == WAIT_TIMEOUT){
		TerminateThread(threadHandle, 0);
	}
	else{
		printf("thread success exit\n");
	}
	// 关闭线程句柄
	CloseHandle(threadHandle);

	// 释放资源
	closesocket(sock);
	WSACleanup();
}




// 获取计算机名称以及MAC地址的函数线程
/**
 * NetBiosRecvThreadProc()函数运行过程
 * 在while循环中调用recvfrom()函数，在套接字sock上接收数据.
 * 如果超时，则多重试两次。
 * NetBIOS回应包的前56位是网络适配器状态信息，第57位保存名字表中名字的数量。
 * 依次处理名字表中每个名字项，如果最后一位是0x00，则表示当前名字项用于保有存计算机名或者工作组。
 *　 name_flags字段可以区分当前名字是计算机名还是工作组。
 * 在NetBIOS回应包中，包字表后面的6个字节是计算机的MAC地址。
 *   调用GetEthernetAdapter()函数可以将其转换为字符串。
 * 将获取到的计算机名、工作组和MAC地址保存到ips映射表项中的CDevice对象中。
 **/

DWORD WINAPI NetBiosRecvThreadProc(void *param)
{
	char respuesta[1000];					// 保存接收到的NetBIOS
	unsigned int count=0;					// 用于在NetBIOS回应包中定位名字数组的位置
	unsigned char ethernet[6];			// 保存MAC地址
	struct sockaddr_in from;				// 发送NetBIOS回应包
	// 参数是要获取NetBIOS信息的IP地址和套接字
	workstationNameThreadStruct *unionStruct = (workstationNameThreadStruct *)param;
	SOCKET sock = unionStruct->s;	// 用于接收NetBIOS回应包的套接字
	// 要获取NetBIOS信息的IP地址
	std::map<unsigned long, CDevice*> *ips = unionStruct->ips;
	int len = sizeof (sockaddr_in);		// 地址长度
	// 定义名字数组
	struct names Names[20*sizeof(struct names)];

	int ipsCount = ips->size();			// 要获取NetBIOS信息的IP地址数量
	int timeoutRetry = 0;					// 记录超时重试的次数
	while(true){
		count = 0;
		// 在套接字sock上接收消息
		int res= recvfrom (sock, respuesta, sizeof(respuesta), 0, (sockaddr *)&from, &len);
		// 如果超时，则重试，但重试次数不超过两次
		if(res == SOCKET_ERROR)	{
			if(GetLastError() == 10060)	{
				timeoutRetry++;
				if(timeoutRetry == 2)
					break;
			}
			continue;
		}
		if(res <= 121){
			continue;
		}

		// 将count定位到名字表中名字的数量。在NetBIOS回应包中，前面56位是网络适配器的状态信息
		memcpy(&count, respuesta+56, 1);
		if(count > 20){		// 最多有20个名字，超出则错误
			continue;
		}

		// 将名字表内在复制到Names数组中
		memcpy(Names,(respuesta+57), count*sizeof(struct names));

		// 将空格字符替换成空
		for(unsigned int i = 0; i < count;i++) {
			for(unsigned int j = 0;j < 15;j++){
				if(Names[i].nb_name[j] == 0x20)
					Names[i].nb_name[j]=0;
			}
		}

		string mac;
		// 如果发送回应包的地址在ips中，则处理该包
		std::map<unsigned long, CDevice*>::iterator itr;
		if( (itr = ips->find(from.sin_addr.S_un.S_addr)) != ips -> end()){
			// 获取发送NetBIOS回应包的IP地址
			in_addr inaddr;
			inaddr.S_un.S_addr = itr->first;
			itr->second->IP = inet_ntoa(inaddr);
			// 处理名字表中的所有名字
			for(int i=0;i<count;i++){
				// 如果最后一位是0x00，则表示当前名字表项为保存计算机名或者工作组
				if(Names[i].nb_name[15] == 0x00){
					char buffers[17] = {0};
					memcpy(buffers, Names[i].nb_name, 16);
					// 使用name_flags字段来区分当前名字是计算机名还是工作组
					if((Names[i].name_flags & 128) == 0) {
						itr->second->Name = buffers;
					}
					else{
						itr->second->Workgroup = buffers;
					}
				}
				// 名字表后面是MAC地址
				memcpy(ethernet,(respuesta+57+count*sizeof(struct names)),6);
				char mac[20] = {0};
				// 格式化MAC地址
				GetEthernetAdapter(ethernet,mac);
				itr->second->Mac = mac;
			}
		}
	}

	return 0;
}



int main()
{
	std::map<unsigned long, CDevice*> ips;
	// 向ips中添加一个设备
	CDevice dev1("10.5.1.5");
	unsigned long ip1 = inet_addr(dev1.IP.c_str());
	ips.insert(make_pair(ip1, &dev1));
	// 向ips中添加第2个设备
	CDevice dev2("10.10.10.1");
	unsigned long ip2 = inet_addr(dev2.IP.c_str());
	ips.insert(make_pair(ip2, &dev2));

	// 获取设备信息
	GetHostInfo(ips, 2000);
	std::map<unsigned long, CDevice*>::iterator itr;
	for(itr = ips.begin(); itr != ips.end(); itr++)
	{
		printf("\nIP: %s;  \nName: %s;  \nMac: %s;  \nWorkgroup:  %s\n\n",
			itr->second->IP.c_str(), itr->second->Name.c_str(), itr->second->Mac.c_str(), itr->second->Workgroup.c_str());
	}
	system("pause");
	return 0;
}

