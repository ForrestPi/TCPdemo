
// ClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CClientDlg 对话框



CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientDlg::IDD, pParent)
	, m_IP(_T(""))
	, m_port(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_IP);
	DDX_Text(pDX, IDC_EDIT2, m_port);
	//  DDX_Control(pDX, IDC_EDIT3, m_info);
	DDX_Control(pDX, IDC_EDIT3, m_info);
	DDX_Control(pDX, IDC_EDIT4, m_name);
	DDX_Control(pDX, IDC_LIST1, m_list);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON2, &CClientDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CClientDlg 消息处理程序

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_client = socket(AF_INET,SOCK_STREAM,0);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlg::OnBnClickedOk()     //链接按钮
{
	// TODO: 在此添加控件通知处理程序代码
	//服务器端地址
	sockaddr_in serveraddr;
	UpdateData(TRUE);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(m_port);
	//m_IP转const char*cp;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,m_IP.GetBuffer(0),-1,NULL,0,NULL,FALSE)+1;
	char *psText;
	psText = new char[dwNum];
	if(!psText)
	{
		delete []psText;
	}
	WideCharToMultiByte (CP_OEMCP,NULL,m_IP.GetBuffer(0),-1,psText,dwNum,NULL,FALSE);
	serveraddr.sin_addr.S_un.S_addr = inet_addr(psText);
	delete []psText;
	if (connect(m_client,(sockaddr*)&serveraddr,sizeof(serveraddr))!=0)
	{
		MessageBox(L"连接失败");
		return;
	}
	else
	{
		MessageBox(L"连接成功");
	}
	WSAAsyncSelect(m_client,m_hWnd,1000,FD_READ);
	CString str,info;
	m_name.GetWindowTextW(str);

	info.Format(L"%s------->%s",str,L"进入聊天室");
	//cstring转char*
	dwNum = WideCharToMultiByte(CP_OEMCP,NULL,info.GetBuffer(0),-1,NULL,0,NULL,FALSE)+1;
	char *ch;
	ch = new char[dwNum];
	if(!ch)
	{
		delete []ch;
	}
	WideCharToMultiByte (CP_OEMCP,NULL,info.GetBuffer(0),-1,ch,dwNum,NULL,FALSE);
	int i = send(m_client,ch,dwNum,0);
	delete []ch;
	//CDialogEx::OnOK();
}


void CClientDlg::OnBnClickedButton2()     //发送按钮
{
	// TODO: 在此添加控件通知处理程序代码
	CString str,name,info;
	m_name.GetWindowTextW(name);
	m_info.GetWindowTextW(str);
	if (!name.IsEmpty()&&!str.IsEmpty())
	{
		info.Format(L"%s说：%s",name,str);
		//开始发送数据
		//CString转const char*
		DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,info.GetBuffer(0),-1,NULL,0,NULL,FALSE)+1;
		char *psText;
		psText = new char[dwNum];
		if(!psText)
		{
			delete []psText;
		}
		WideCharToMultiByte (CP_OEMCP,NULL,info.GetBuffer(0),-1,psText,dwNum,NULL,FALSE);
		int i = send(m_client,psText,info.GetLength(),0);
		delete []psText;
		int nRow = m_list.InsertItem(0, info);//插入行
		m_info.SetWindowTextW(L"");
	}

}

void CClientDlg::ReceiveData()
{
	char buffer[1024];
	//接收服务器端传来的数据
	int num = recv(m_client,buffer,1024,0);
	buffer[num] = 0;
	//char*转CString
	DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, buffer, -1, NULL, 0)+1;//获取bufer数组实际存储长度
	wchar_t *pwText;
	pwText = new wchar_t[dwNum];
	if(!pwText)
	{
		delete []pwText;
	}
	MultiByteToWideChar (CP_ACP, 0, buffer, -1, pwText, dwNum);
	m_list.InsertItem(1,pwText);
	delete []pwText;
}


BOOL CClientDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message==1000)
	{
		ReceiveData();
		return TRUE;
	}
	else
	{
		return CDialogEx::PreTranslateMessage(pMsg);
	}
	
}
