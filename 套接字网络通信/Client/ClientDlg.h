
// ClientDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

// CClientDlg 对话框
class CClientDlg : public CDialogEx
{
// 构造
public:
	CClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CLIENT_DIALOG };

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

