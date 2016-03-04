// GetNetBIOSNames.cpp -- ��ȡLANA������NetBIOS����

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
	// ��ʼ������ձ������ֱ�ͻỰ��
	if (!NBReset (LANANUM, 20, 30)){
		return -1;
	}
	// �򱾵����ֱ������UNIQUENAME
    if (!NBAddName (LANANUM, LOCALNAME)) {
		return -1;
	}
	// �г��������ֱ��е�����
    if (!NBListNames (LANANUM, LOCALNAME)) {
		return -1;
	}
    printf ("Succeeded.\n");
	system("pause");
	return 0;
}

/**
 * NCB�ṹ��Ķ���:
 * typedef struct _NCB{
 *   UCHAR ncb_command;  // ָ����������Լ�����NCB�ṹ���Ƿ��첽����ı�ʶ
 *   UCHAR ncb_retcode;  // ָ������ķ��ر���
 *   UCHAR ncb_lsn;      // ��ʾ���ػỰ��ţ���ָ�������д˱��Ψһ��ʶһ���Ự
 *   UCHAR ncb_num;      // ָ�������������ֱ�š�
 *   PUCHAR ncb_buffer;  // ָ����Ϣ�����������з�����Ϣ��������Ϣ����������״̬��Ϣ������������
 *   WORD ncb_length;    // ָ����Ϣ�������Ĵ�С����λΪ�ֽڡ�
 *   UCHAR ncb_callname[NCBNAMSZ]; // ָ��Զ�̶�Ӧ�ó��������
 *   UCHAR ncb_name[NCBNAMSZ];     // ָ��Ӧ�ó������ʶ�������
 *   UCHAR ncb_rto;                // ָ���Ựִ�н��ղ����ĳ�ʱʱ��
 *   void (CALLBACK * ncb_post)(struct NCB);
 *   UCHAR ncb_lana_num;           // ָ��LANA���
 *   UCHAR ncb_cmd_cplt;           // ָ��������ɱ�ʶ
 *   UCHAR ncb_reserve[X];         // �����ֶ�
 *   HANDLE ncb_event;             // ָ���¼�����ľ��
 * }NCB, *PNCB;
 **/


// ��ձ������ֺͻỰ��
BOOL NBReset (int nLana, int nSessions, int nNames)
{
    NCB ncb;
    memset (&ncb, 0, sizeof (ncb));		// ���ncb�ṹ��
    ncb.ncb_command = NCBRESET;			// ִ��NCBRESET�����λ���������������������ָ��LANA����϶���ı������ֱ�ͻỰ��
    ncb.ncb_lsn = 0;					// �����µ�lana_num��Դ��Netbios()�����ɹ�ִ����NCBCALL����󷵻صı��
    ncb.ncb_lana_num = nLana;			// ����lana_num��Դ��ָ�������������ֱ�š�
    ncb.ncb_callname[0] = nSessions;	// �������Ự��
    ncb.ncb_callname[2] = nNames;		// �������������
    Netbios (&ncb);						// ִ��NCBRESET����
    NBCheck (ncb);						// ���ִ�н������ȷ�������ncb.ncb_retcode
	// ����ɹ�����TRUE�����򷵻�FALSE
    return (NRC_GOODRET == ncb.ncb_retcode);
}


// �򱾵����ֱ����������
BOOL NBAddName (int nLana, LPCSTR szName)
{
    NCB ncb;
    memset (&ncb, 0, sizeof (ncb));		// ���ncb�ṹ��
    ncb.ncb_command = NCBADDNAME;		// ִ��NCBDDNAME����򱾵����ֱ������һ��Ψһ������
    ncb.ncb_lana_num = nLana;			// ����lana_num��Դ��ָ�������������ֱ�š�
	MakeNetbiosName ((char*) ncb.ncb_name, szName); // ��szName��ֵ��ncb.ncb_name��
    Netbios (&ncb);						// ִ��NCBRESET����
    NBCheck (ncb);						// ���ִ�н������ȷ�������ncb.ncb_retcode
	// ����ɹ�����TRUE�����򷵻�FALSE
    return (NRC_GOODRET == ncb.ncb_retcode);
}


// Build a name of length NCBNAMSZ, padding with spaces.
// ��szSrc�е����ָ�ֵ��achDest�У����ֵĳ���ΪNCBNAMESZ
// ������㣬��ʹ�ÿո���
void MakeNetbiosName (char *achDest, LPCSTR szSrc)
{
    int cchSrc = strlen ((char*)szSrc);	// ȡ���ֵĳ���
    if (cchSrc > NCBNAMSZ){
        cchSrc = NCBNAMSZ;
	}
    memset (achDest, ' ', NCBNAMSZ);
    memcpy (achDest, szSrc, cchSrc);
}

// �г�ָ��LANA�����е�����
BOOL NBListNames (int nLana, LPCSTR szName)
{
    int cbBuffer;					// ��ȡ���ݵĻ�����
    ADAPTER_STATUS *pStatus;		// ������������������Ϣ
    NAME_BUFFER *pNames;			// ���汾��������Ϣ
    HANDLE hHeap;					// ��ǰ���ý��̵ĶѾ��

    hHeap = GetProcessHeap();		// ��ǰ���ý��̵ĶѾ��
    cbBuffer = sizeof (ADAPTER_STATUS) + 255 * sizeof (NAME_BUFFER);// ������ܵ���󻺳����ռ�
	pStatus = (ADAPTER_STATUS *) HeapAlloc (hHeap, 0, cbBuffer);// ΪpStatus����ռ�
    if (NULL == pStatus){
        return FALSE;
	}
	// ��ȡ����������������Ϣ��������浽pStatus��
    if (!NBAdapterStatus (nLana, (PVOID) pStatus, cbBuffer, szName)){
        HeapFree (hHeap, 0, pStatus);
        return FALSE;
    }
    // �г�����ADAPTER_STATUS�ṹ������������Ϣ
    pNames = (NAME_BUFFER *) (pStatus + 1);
    for (int i = 0; i < pStatus->name_count; i++){
        printf ("\t%.*s\n", NCBNAMSZ, pNames[i].name);
	}

    HeapFree (hHeap, 0, pStatus);// �ͷŷ���Ķѿռ�

    return TRUE;
}

// ��ȡָ��LANA��������������Ϣ
// nLana, LANA���
// pBuffer, ��ȡ��������������������
// cbBuffer, ����������
// szName, ��������
BOOL NBAdapterStatus (int nLana, PVOID pBuffer, int cbBuffer,  LPCSTR szName)
{
    NCB ncb;
    memset (&ncb, 0, sizeof (ncb));		// ���ncb�ṹ��
    ncb.ncb_command = NCBASTAT;			// ����ִ��NCBASTAT�����ȡ���ػ�Զ��������������״̬
    ncb.ncb_lana_num = nLana;			// ����LANA���

    ncb.ncb_buffer = (PUCHAR) pBuffer;	// ����ȡ�������ݱ��浽����pBuffer��
    ncb.ncb_length = cbBuffer;			// ���û���������

    MakeNetbiosName ((char*) ncb.ncb_callname, szName);// ���ò���ncb.ncb_callname
    Netbios (&ncb);						// ִ��NetBIOS����
    NBCheck (ncb);						// ���ִ�в��ɹ������������ֵ
	// ����ɹ�����TRUE�����򷵻�FALSE
    return (NRC_GOODRET == ncb.ncb_retcode);
}
