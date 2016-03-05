
// serverDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"

#define MAXNUM 10

// CserverDlg �Ի���
class CserverDlg : public CDialogEx
{
// ����
public:
	CserverDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CIPAddressCtrl m_IP;
	CString m_serverIP;
private:
	SOCKET m_server,m_client;
	SOCKET m_Clients[MAXNUM];    //�ͻ����׽���
	int m_CurClient;     //��ǰ���ӵĿͻ�����
public:
	afx_msg void OnBnClickedOk();
private:
	UINT m_port;
	void HandleData(void);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
