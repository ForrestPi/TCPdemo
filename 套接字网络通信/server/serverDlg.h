
// serverDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"

#define MAXNUM 10

// CserverDlg 对话框
class CserverDlg : public CDialogEx
{
// 构造
public:
	CserverDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
	SOCKET m_Clients[MAXNUM];    //客户端套接字
	int m_CurClient;     //当前链接的客户数量
public:
	afx_msg void OnBnClickedOk();
private:
	UINT m_port;
	void HandleData(void);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
