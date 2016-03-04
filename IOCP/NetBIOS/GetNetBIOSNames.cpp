// GetNetBIOSNames.cpp -- 获取LANA上所有NetBIOS名字

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <Nb30.h>

#pragma comment(lib, "netapi32.lib")

// Set LANANUM and LOCALNAME as appropriate for your system
#define LANANUM     0
#define LOCALNAME   "UNIQUENAME"
#define NBCheck(x)  if (NRC_GOODRET != x.ncb_retcode) { \
	                    printf("Line %d: Got 0x%x from NetBios()\n", \
                               __LINE__, x.ncb_retcode); \
                    }

void MakeNetbiosName (char *, LPCSTR);
BOOL NBReset (int, int, int);
BOOL NBAddName (int, LPCSTR);
BOOL NBListNames (int, LPCSTR);
BOOL NBAdapterStatus (int, PVOID, int, LPCSTR);

int main()
{
	// 初始化，清空本地名字表和会话表
	if (!NBReset (LANANUM, 20, 30)){
		return -1;
	}
	// 向本地名字表中添加UNIQUENAME
    if (!NBAddName (LANANUM, LOCALNAME)) {
		return -1;
	}
	// 列出本地名字表中的名字
    if (!NBListNames (LANANUM, LOCALNAME)) {
		return -1;
	}
    printf ("Succeeded.\n");
	system("pause");
	return 0;
}

/**
 * NCB结构体的定义:
 * typedef struct _NCB{
 *   UCHAR ncb_command;  // 指定命令编码以及表明NCB结构体是否被异步处理的标识
 *   UCHAR ncb_retcode;  // 指定命令的返回编码
 *   UCHAR ncb_lsn;      // 表示本地会话编号，在指定环境中此编号唯一标识一个会话
 *   UCHAR ncb_num;      // 指定本地网络名字编号。
 *   PUCHAR ncb_buffer;  // 指定消息缓冲区。（有发送信息、接收信息、接收请求状态信息三个缓冲区）
 *   WORD ncb_length;    // 指定消息缓冲区的大小，单位为字节。
 *   UCHAR ncb_callname[NCBNAMSZ]; // 指定远程端应用程序的名字
 *   UCHAR ncb_name[NCBNAMSZ];     // 指定应用程序可以识别的名字
 *   UCHAR ncb_rto;                // 指定会话执行接收操作的超时时间
 *   void (CALLBACK * ncb_post)(struct NCB);
 *   UCHAR ncb_lana_num;           // 指定LANA编号
 *   UCHAR ncb_cmd_cplt;           // 指定命令完成标识
 *   UCHAR ncb_reserve[X];         // 保留字段
 *   HANDLE ncb_event;             // 指向事件对象的句柄
 * }NCB, *PNCB;
 **/


// 清空本地名字和会话表
BOOL NBReset (int nLana, int nSessions, int nNames)
{
    NCB ncb;
    memset (&ncb, 0, sizeof (ncb));		// 清空ncb结构体
    ncb.ncb_command = NCBRESET;			// 执行NCBRESET命令，复位局域网网络适配器，清空指定LANA编号上定义的本地名字表和会话表。
    ncb.ncb_lsn = 0;					// 分配新的lana_num资源，Netbios()函数成功执行了NCBCALL命令后返回的编号
    ncb.ncb_lana_num = nLana;			// 设置lana_num资源，指定本地网络名字编号。
    ncb.ncb_callname[0] = nSessions;	// 设置最大会话数
    ncb.ncb_callname[2] = nNames;		// 设置最大名字数
    Netbios (&ncb);						// 执行NCBRESET命令
    NBCheck (ncb);						// 如果执行结果不正确，则输出ncb.ncb_retcode
	// 如果成功返回TRUE，否则返回FALSE
    return (NRC_GOODRET == ncb.ncb_retcode);
}


// 向本地名字表中添加名字
BOOL NBAddName (int nLana, LPCSTR szName)
{
    NCB ncb;
    memset (&ncb, 0, sizeof (ncb));		// 清空ncb结构体
    ncb.ncb_command = NCBADDNAME;		// 执行NCBDDNAME命令，向本地名字表中添加一个唯一的名字
    ncb.ncb_lana_num = nLana;			// 设置lana_num资源，指定本地网络名字编号。
	MakeNetbiosName ((char*) ncb.ncb_name, szName); // 将szName赋值到ncb.ncb_name中
    Netbios (&ncb);						// 执行NCBRESET命令
    NBCheck (ncb);						// 如果执行结果不正确，则输出ncb.ncb_retcode
	// 如果成功返回TRUE，否则返回FALSE
    return (NRC_GOODRET == ncb.ncb_retcode);
}


// Build a name of length NCBNAMSZ, padding with spaces.
// 将szSrc中的名字赋值到achDest中，名字的长度为NCBNAMESZ
// 如果不足，则使用空格补齐
void MakeNetbiosName (char *achDest, LPCSTR szSrc)
{
    int cchSrc = strlen ((char*)szSrc);	// 取名字的长度
    if (cchSrc > NCBNAMSZ){
        cchSrc = NCBNAMSZ;
	}
    memset (achDest, ' ', NCBNAMSZ);
    memcpy (achDest, szSrc, cchSrc);
}

// 列出指定LANA上所有的名字
BOOL NBListNames (int nLana, LPCSTR szName)
{
    int cbBuffer;					// 获取数据的缓冲区
    ADAPTER_STATUS *pStatus;		// 保存网络适配器的信息
    NAME_BUFFER *pNames;			// 保存本地名字信息
    HANDLE hHeap;					// 当前调用进程的堆句柄

    hHeap = GetProcessHeap();		// 当前调用进程的堆句柄
    cbBuffer = sizeof (ADAPTER_STATUS) + 255 * sizeof (NAME_BUFFER);// 分配可能的最大缓冲区空间
	pStatus = (ADAPTER_STATUS *) HeapAlloc (hHeap, 0, cbBuffer);// 为pStatus分配空间
    if (NULL == pStatus){
        return FALSE;
	}
	// 获取本地网络适配器信息，结果保存到pStatus中
    if (!NBAdapterStatus (nLana, (PVOID) pStatus, cbBuffer, szName)){
        HeapFree (hHeap, 0, pStatus);
        return FALSE;
    }
    // 列出跟在ADAPTER_STATUS结构体后面的名字信息
    pNames = (NAME_BUFFER *) (pStatus + 1);
    for (int i = 0; i < pStatus->name_count; i++){
        printf ("\t%.*s\n", NCBNAMSZ, pNames[i].name);
	}

    HeapFree (hHeap, 0, pStatus);// 释放分配的堆空间

    return TRUE;
}

// 获取指定LANA的网络适配器信息
// nLana, LANA编号
// pBuffer, 获取到的网络适配器缓冲区
// cbBuffer, 缓冲区长度
// szName, 主机名字
BOOL NBAdapterStatus (int nLana, PVOID pBuffer, int cbBuffer,  LPCSTR szName)
{
    NCB ncb;
    memset (&ncb, 0, sizeof (ncb));		// 清空ncb结构体
    ncb.ncb_command = NCBASTAT;			// 设置执行NCBASTAT命令，获取本地或远程网络适配器的状态
    ncb.ncb_lana_num = nLana;			// 设置LANA编号

    ncb.ncb_buffer = (PUCHAR) pBuffer;	// 将获取到的数据保存到参数pBuffer中
    ncb.ncb_length = cbBuffer;			// 设置缓冲区长度

    MakeNetbiosName ((char*) ncb.ncb_callname, szName);// 设置参数ncb.ncb_callname
    Netbios (&ncb);						// 执行NetBIOS命令
    NBCheck (ncb);						// 如果执行不成功，则输出返回值
	// 如果成功返回TRUE，否则返回FALSE
    return (NRC_GOODRET == ncb.ncb_retcode);
}
