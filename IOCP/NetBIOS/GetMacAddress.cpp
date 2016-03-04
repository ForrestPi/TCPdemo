// GetMacAddress.cpp -- ��ȡ�����������ϵ�MAC��ַ

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Nb30.h>

#pragma comment(lib, "netapi32.lib")


/**
 * ADAPTER_STATUS�ṹ���а�����������������Ϣ
 * typedef struct _ADAPTER_STATUS{
 *   UCHAR adapter_address[6];  // ָ�������������ĵ�ַ
 *   UCHAR rev_major;           // ָ��������������汾��
 *   UCHAR reserved0;           // �����ֶΣ�ʼ��Ϊ��
 *   UCHAR adapter_type;        // ָ������������������
 *   UCHAR rev_minor;           // ָ����������ĸ��汾��
 *   WORD duration;             // ָ�������ʱ�����ڣ���λΪ����
 *   WORD frmr_recv;            // ָ�����յ���FRMR��֡�ܾ���֡����
 *   WORD frmr_xmit;            // ָ�����͵�FRMR֡����
 *   WORD iframe_recv_err;      // ָ�����յ��Ĵ���֡����
 *   WORD xmit_aborts;          // ָ�����ڴ���İ�����
 *   DWORD xmit_success;        // ָ���ɹ�����İ�����
 *   DWORD recv_success;        // ָ���ɹ����յİ�����
 *   DWORD iframe_xmit_err;     // ָ������Ĵ���֡����
 *   WORD recv_buf_unavail;     // ָ���������޷�ΪԶ�̼�����ṩ�������
 *   WORD tl_timeouts;          // ָ��DLC(Data Link Control, ������·���ƣ�T1��������ʱ�Ĵ���
 *   WORD ti_timeouts;          // ָ��ti�ǻ��ʱ����ʱ�Ĵ�����ti��ʱ�����ڼ��Ͽ�������
 *   DWORD reservedl;           // �����ֶΣ�ʼ��Ϊ0
 *   WORD free_ncbs;            // ָ����ǰ���е�������ƿ������
 *   WORD max_cfg_ncbs;         // ���������ƿ����ݰ��Ĵ�С
 *   WORD max_ncbs;             // ���������ƿ������
 *   WORD xmit_buf_unavail;     // �����õĴ�����Ļ�����
 *   WORD max_dgram_size;       // �������ֵ
 *   WORD pending_sess;         // ָ������Ự������
 *   WORD max_cfg_sess;         // ָ�����ݰ�������С����ֵ����Ϊ512�ֽ�
 *   WORD max_sess;             // �������
 *   WORD max_sess_pkt_size;    // ָ���Ự���ݰ�������С
 *   WORD name_count;           // ָ���������ֱ������ֵ�����
 * }ADAPTER_STATUS, *PADAPTER_STATUS;
 **/


// �ṹ��ASTAT���ڶ�������������״̬�����ֱ���Ϣ
typedef struct _ASTAT_
{
    ADAPTER_STATUS adapt;		// ����������״̬
    NAME_BUFFER NameBuff [30];	// ���ֱ���Ϣ
}ASTAT, * PASTAT;

ASTAT Adapter;

int main()
{
	NCB ncb;						// NCB�ṹ�壬��������ִ�е�NetBIOS����Ͳ���
    UCHAR uRetCode;					// ִ��Netbios()�����ķ���ֵ
    memset( &ncb, 0, sizeof(ncb) );	// ��ʼ��ncb�ṹ��
    ncb.ncb_command = NCBRESET;		// ����ִ��NCBRESET����λ����������
    ncb.ncb_lana_num = 0;			// ����LANA���

    uRetCode = Netbios( &ncb );		// ����Netbios()������ִ��NCBRESET����
	// ���ִ��NCBRESET����Ľ��
    printf( "The NCBRESET return code is: 0x%x \n", uRetCode );

    memset( &ncb, 0, sizeof(ncb) );	// ��ʼ��ncb
    ncb.ncb_command = NCBASTAT;		// ִ��NCBASTAT�����ȡ����������״̬
    ncb.ncb_lana_num = 0;			// ����LANA���
	// ����ִ��NCBASTAT����Ĳ���������ȡ�����������������ݱ��浽Adapter�ṹ����
    memcpy( &ncb.ncb_callname, "*               ", 16 );
    ncb.ncb_buffer = (UCHAR*) &Adapter;
    ncb.ncb_length = sizeof(Adapter);
    uRetCode = Netbios( &ncb );		// ����Netbios()������ִ��NCBASTAT����
    printf( "The NCBASTAT return code is: 0x%x \n", uRetCode );
    if ( uRetCode == 0 ) {			// ���MAC��ַ
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

