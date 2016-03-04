// GetMacAddress.cpp -- 获取网络适配器上的MAC地址

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Nb30.h>

#pragma comment(lib, "netapi32.lib")


/**
 * ADAPTER_STATUS结构体中包含网络适配器的信息
 * typedef struct _ADAPTER_STATUS{
 *   UCHAR adapter_address[6];  // 指定网络适配器的地址
 *   UCHAR rev_major;           // 指定发布软件的主版本号
 *   UCHAR reserved0;           // 保留字段，始终为零
 *   UCHAR adapter_type;        // 指定网络适配器的类型
 *   UCHAR rev_minor;           // 指定发布软件的副版本号
 *   WORD duration;             // 指定报告的时间周期，单位为分钟
 *   WORD frmr_recv;            // 指定接收到的FRMR（帧拒绝）帧数量
 *   WORD frmr_xmit;            // 指定传送的FRMR帧数量
 *   WORD iframe_recv_err;      // 指定接收到的错误帧数量
 *   WORD xmit_aborts;          // 指定终于传输的包数量
 *   DWORD xmit_success;        // 指定成功传输的包数量
 *   DWORD recv_success;        // 指定成功接收的包数量
 *   DWORD iframe_xmit_err;     // 指定传输的错误帧数量
 *   WORD recv_buf_unavail;     // 指定缓冲区无法为远程计算机提供服务次数
 *   WORD tl_timeouts;          // 指定DLC(Data Link Control, 数据链路控制）T1计数器超时的次数
 *   WORD ti_timeouts;          // 指定ti非活动计时器超时的次数。ti计时器用于检测断开的连接
 *   DWORD reservedl;           // 保留字段，始终为0
 *   WORD free_ncbs;            // 指定当前空闲的网络控制块的数量
 *   WORD max_cfg_ncbs;         // 最大网络控制块数据包的大小
 *   WORD max_ncbs;             // 最大网络控制块的数量
 *   WORD xmit_buf_unavail;     // 不可用的传输包的缓冲区
 *   WORD max_dgram_size;       // 包的最大值
 *   WORD pending_sess;         // 指定挂起会话的数量
 *   WORD max_cfg_sess;         // 指定数据包的最大大小，该值至少为512字节
 *   WORD max_sess;             // 最大数量
 *   WORD max_sess_pkt_size;    // 指定会话数据包的最大大小
 *   WORD name_count;           // 指定本地名字表中名字的数量
 * }ADAPTER_STATUS, *PADAPTER_STATUS;
 **/


// 结构体ASTAT用于定义网络适配器状态和名字表信息
typedef struct _ASTAT_
{
    ADAPTER_STATUS adapt;		// 网络适配器状态
    NAME_BUFFER NameBuff [30];	// 名字表信息
}ASTAT, * PASTAT;

ASTAT Adapter;

int main()
{
	NCB ncb;						// NCB结构体，用于设置执行的NetBIOS命令和参数
    UCHAR uRetCode;					// 执行Netbios()函数的返回值
    memset( &ncb, 0, sizeof(ncb) );	// 初始化ncb结构体
    ncb.ncb_command = NCBRESET;		// 设置执行NCBRESET，复位网络适配器
    ncb.ncb_lana_num = 0;			// 设置LANA编号

    uRetCode = Netbios( &ncb );		// 调用Netbios()函数，执行NCBRESET命令
	// 输出执行NCBRESET命令的结果
    printf( "The NCBRESET return code is: 0x%x \n", uRetCode );

    memset( &ncb, 0, sizeof(ncb) );	// 初始化ncb
    ncb.ncb_command = NCBASTAT;		// 执行NCBASTAT命令，获取网络适配器状态
    ncb.ncb_lana_num = 0;			// 设置LANA编号
	// 设置执行NCBASTAT命令的参数，将获取到的网络适配器数据保存到Adapter结构体中
    memcpy( &ncb.ncb_callname, "*               ", 16 );
    ncb.ncb_buffer = (UCHAR*) &Adapter;
    ncb.ncb_length = sizeof(Adapter);
    uRetCode = Netbios( &ncb );		// 调用Netbios()函数，执行NCBASTAT命令
    printf( "The NCBASTAT return code is: 0x%x \n", uRetCode );
    if ( uRetCode == 0 ) {			// 输出MAC地址
        printf( "The Ethernet Number is: %02x-%02x-%02x-%02x-%02x-%02x\n",
                Adapter.adapt.adapter_address[0],
                Adapter.adapt.adapter_address[1],
                Adapter.adapt.adapter_address[2],
                Adapter.adapt.adapter_address[3],
                Adapter.adapt.adapter_address[4],
                Adapter.adapt.adapter_address[5] );
    }
	system("pause");
	return 0;
}

