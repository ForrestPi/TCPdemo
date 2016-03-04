// NbtStat.cpp -- ��ȡ������ָ��������Ļ�����Ϣ

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
	string IP;			// IP��ַ
	string Name;		// ����
	string Mac;			// Mac��ַ
	string Workgroup;	// ������
};



// ͨ��NbtStat��ȡ�����������Ϣ�Ľṹ��
struct names
{
	unsigned char nb_name[16];	// ��ʾ���յ�������
	unsigned short name_flags;	// ��ʶ���ֵĺ���
};


// �����ȡNetBIOS��Ϣ��Socket��IP��ַ�б�
struct workstationNameThreadStruct
{
	SOCKET s;					// ָ�����ͺͽ���NetBIOS���ݰ���Socket
	std::map<unsigned long, CDevice*> *ips;	// ָ����ȡNetBIOS��Ϣ��IP��ַ�б�
};

// ��ʽ��ethernet�е��ֽ�Ϊ�ַ���
void GetEthernetAdapter(unsigned char *ethernet, char *macstr)
{
	sprintf(macstr, "%02x %02x %02x %02x %02x %02x",
		ethernet[0],	ethernet[1],		ethernet[2],
		ethernet[3],	ethernet[4],		ethernet[5]
	);
	return;
}


// ��ȡ�豸�����ƺ�MAC��ַ����Ϣ
/**
 * GetHostInfo()���������й���
 * �������ͺͽ���NetBIOS���ݰ���Socket, ��������ڱ��ص�ַ�Ķ˿�0�ϡ�
 * ��������NetBIOS��Ӧ�����̣߳�����unionStruct�а���Ҫ��ȡNetBIOS��Ϣ��IP��ַ��ͨ���õ�Socket��
 * ʹ��forѭ�����������ÿ��IP��ַ����NetBIOS���������������ݱ������ַ�����input�С�
 * ����WaitForSingleObject()�������ȴ������߳̽�����
 * �����ʱ������������߳�NetBiosRecvThreadProc()������
 * �ر��߳̾�����ͷ���Դ��
 **/
void GetHostInfo(std::map<unsigned long, CDevice*> &ips, int timeout)
{
	DWORD WINAPI NetBiosRecvThreadProc(void *param);

	const int defaultPort = 0;      // �趨Ĭ�ϵİ󶨶˿�
	SOCKET sock;					// ͨ���׽���
	struct sockaddr_in origen;		// ���ص�ַ
	WSADATA wsaData;				// Windows Sockets��������

	if(WSAStartup(MAKEWORD(2,1),&wsaData) != 0){// ��ʼ��Windows Sockets����
		return;
	}
	if(INVALID_SOCKET ==(sock = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP))){// ����TCP/IP�׽���
		return;
	}
	// ���ó�ʱʱ��
	if(SOCKET_ERROR ==setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))){
		closesocket(sock);
		WSACleanup();
		return;
	}
	// ���׽��ְ󶨵����ص�ַ�Ͷ˿�0
	memset(&origen, 0, sizeof(origen));
	origen.sin_family = AF_INET;
	origen.sin_addr.s_addr = htonl (INADDR_ANY);
	origen.sin_port = htons (defaultPort);
	if (bind (sock, (struct sockaddr *) &origen, sizeof(origen)) < 0) { // �׽�������
		closesocket(sock);
		WSACleanup();
		return;
	}

	// Ϊ���������߳�׼������
	workstationNameThreadStruct unionStruct;
	unionStruct.ips = &ips;
	unionStruct.s = sock;
	// �����̵߳ȴ�����NetBIOS��Ӧ��
	DWORD pid;
	HANDLE threadHandle = CreateThread(NULL, 0, NetBiosRecvThreadProc,(void *)&unionStruct, 0, &pid);

	// ������ips�е�ÿ��IP��ַ��137�˿ڷ���NetBIOS�������������input�ַ������У�
	std::map<unsigned long, CDevice*>::iterator itr;
	for(itr=ips.begin();itr != ips.end();itr++)
	{
		char input[]="\x80\x94\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x20\x43\x4b\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x00\x00\x21\x00\x01";
		struct sockaddr_in dest;
		// ����NetBios������Ϣ
		memset(&dest,0,sizeof(dest));
		dest.sin_addr.s_addr = itr->first;
		dest.sin_family = AF_INET;
		dest.sin_port = htons(137);
		sendto (sock, input, sizeof(input)-1, 0, (struct sockaddr *)&dest, sizeof (dest));
	}
	// �ȴ������߳�NetBiosRecvThreadProc����
	DWORD ret = WaitForSingleObject(threadHandle, timeout * 4);
	// �����ʱ������������߳�NetBiosRecvThreadProc
	if(ret == WAIT_TIMEOUT){
		TerminateThread(threadHandle, 0);
	}
	else{
		printf("thread success exit\n");
	}
	// �ر��߳̾��
	CloseHandle(threadHandle);

	// �ͷ���Դ
	closesocket(sock);
	WSACleanup();
}




// ��ȡ����������Լ�MAC��ַ�ĺ����߳�
/**
 * NetBiosRecvThreadProc()�������й���
 * ��whileѭ���е���recvfrom()���������׽���sock�Ͻ�������.
 * �����ʱ������������Ρ�
 * NetBIOS��Ӧ����ǰ56λ������������״̬��Ϣ����57λ�������ֱ������ֵ�������
 * ���δ������ֱ���ÿ�������������һλ��0x00�����ʾ��ǰ���������ڱ��д����������߹����顣
 *�� name_flags�ֶο������ֵ�ǰ�����Ǽ���������ǹ����顣
 * ��NetBIOS��Ӧ���У����ֱ�����6���ֽ��Ǽ������MAC��ַ��
 *   ����GetEthernetAdapter()�������Խ���ת��Ϊ�ַ�����
 * ����ȡ���ļ���������������MAC��ַ���浽ipsӳ������е�CDevice�����С�
 **/

DWORD WINAPI NetBiosRecvThreadProc(void *param)
{
	char respuesta[1000];					// ������յ���NetBIOS
	unsigned int count=0;					// ������NetBIOS��Ӧ���ж�λ���������λ��
	unsigned char ethernet[6];			// ����MAC��ַ
	struct sockaddr_in from;				// ����NetBIOS��Ӧ��
	// ������Ҫ��ȡNetBIOS��Ϣ��IP��ַ���׽���
	workstationNameThreadStruct *unionStruct = (workstationNameThreadStruct *)param;
	SOCKET sock = unionStruct->s;	// ���ڽ���NetBIOS��Ӧ�����׽���
	// Ҫ��ȡNetBIOS��Ϣ��IP��ַ
	std::map<unsigned long, CDevice*> *ips = unionStruct->ips;
	int len = sizeof (sockaddr_in);		// ��ַ����
	// ������������
	struct names Names[20*sizeof(struct names)];

	int ipsCount = ips->size();			// Ҫ��ȡNetBIOS��Ϣ��IP��ַ����
	int timeoutRetry = 0;					// ��¼��ʱ���ԵĴ���
	while(true){
		count = 0;
		// ���׽���sock�Ͻ�����Ϣ
		int res= recvfrom (sock, respuesta, sizeof(respuesta), 0, (sockaddr *)&from, &len);
		// �����ʱ�������ԣ������Դ�������������
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

		// ��count��λ�����ֱ������ֵ���������NetBIOS��Ӧ���У�ǰ��56λ��������������״̬��Ϣ
		memcpy(&count, respuesta+56, 1);
		if(count > 20){		// �����20�����֣����������
			continue;
		}

		// �����ֱ����ڸ��Ƶ�Names������
		memcpy(Names,(respuesta+57), count*sizeof(struct names));

		// ���ո��ַ��滻�ɿ�
		for(unsigned int i = 0; i < count;i++) {
			for(unsigned int j = 0;j < 15;j++){
				if(Names[i].nb_name[j] == 0x20)
					Names[i].nb_name[j]=0;
			}
		}

		string mac;
		// ������ͻ�Ӧ���ĵ�ַ��ips�У�����ð�
		std::map<unsigned long, CDevice*>::iterator itr;
		if( (itr = ips->find(from.sin_addr.S_un.S_addr)) != ips -> end()){
			// ��ȡ����NetBIOS��Ӧ����IP��ַ
			in_addr inaddr;
			inaddr.S_un.S_addr = itr->first;
			itr->second->IP = inet_ntoa(inaddr);
			// �������ֱ��е���������
			for(int i=0;i<count;i++){
				// ������һλ��0x00�����ʾ��ǰ���ֱ���Ϊ�������������߹�����
				if(Names[i].nb_name[15] == 0x00){
					char buffers[17] = {0};
					memcpy(buffers, Names[i].nb_name, 16);
					// ʹ��name_flags�ֶ������ֵ�ǰ�����Ǽ���������ǹ�����
					if((Names[i].name_flags & 128) == 0) {
						itr->second->Name = buffers;
					}
					else{
						itr->second->Workgroup = buffers;
					}
				}
				// ���ֱ������MAC��ַ
				memcpy(ethernet,(respuesta+57+count*sizeof(struct names)),6);
				char mac[20] = {0};
				// ��ʽ��MAC��ַ
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
	// ��ips�����һ���豸
	CDevice dev1("10.5.1.5");
	unsigned long ip1 = inet_addr(dev1.IP.c_str());
	ips.insert(make_pair(ip1, &dev1));
	// ��ips����ӵ�2���豸
	CDevice dev2("10.10.10.1");
	unsigned long ip2 = inet_addr(dev2.IP.c_str());
	ips.insert(make_pair(ip2, &dev2));

	// ��ȡ�豸��Ϣ
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

