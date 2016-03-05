
// ClientDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

// CClientDlg �Ի���
class CClientDlg : public CDialogEx
{
// ����
public:
	CClientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CLIENT_DIALOG };

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
	CString m_IP;
	UINT m_port;
public:
//	CEdit m_info;
private:
	CEdit m_info;
	CEdit m_name;
	CListCtrl m_list;
private:
	SOCKET m_client;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton2();
private:
	void ReceiveData();
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

