
// serverDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "server.h"
#include "serverDlg.h"
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


// CserverDlg 对话框



CserverDlg::CserverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CserverDlg::IDD, pParent)
	, m_port(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CserverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_IP);
	DDX_Text(pDX, IDC_EDIT1, m_port);
}

BEGIN_MESSAGE_MAP(CserverDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CserverDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CserverDlg 消息处理程序

BOOL CserverDlg::OnInitDialog()
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
	//创建套接字
	m_server = socket(AF_INET,SOCK_STREAM,0);
	//将网络中的事件关联到窗口的消息中
	WSAAsyncSelect(m_server,m_hWnd,2000,FD_WRITE|FD_READ|FD_ACCEPT);
	m_client = 0;
	m_serverIP = L"";
	for (int i=0;i<MAXNUM;i++)
	{
		m_Clients[i] = 0;
	}
	m_CurClient = 0;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CserverDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CserverDlg::OnPaint()
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
HCURSOR CserverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CserverDlg::OnBnClickedOk()  //处理监听按钮
{
	// TODO: 在此添加控件通知处理程序代码
	//服务器端地址
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	m_IP.GetWindowText(m_serverIP);
	//设置本机地址
	//CString转const char*
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,m_serverIP.GetBuffer(0),-1,NULL,0,NULL,FALSE);
	char *psText;
	psText = new char[dwNum];
	if(!psText)
	{
		delete []psText;
	}
	WideCharToMultiByte (CP_OEMCP,NULL,m_serverIP.GetBuffer(0),-1,psText,dwNum,NULL,FALSE);
	serveraddr.sin_addr.S_un.S_addr = inet_addr(psText);
	delete []psText;
	UpdateData(TRUE);
	serveraddr.sin_port = htons(m_port);
	//绑定地址
	if (bind(m_server,(sockaddr*)&serveraddr,sizeof(serveraddr)))
	{
		MessageBox(L"绑定地址失败");
		return;
	}
	//开始监听
	listen(m_server,50);

	//CDialogEx::OnOK();
}


void CserverDlg::HandleData(void)
{
	sockaddr_in serveraddr;
	char buffer[1024];
	int len = sizeof(serveraddr);
	//接收客户端的数据
	int curlink = -1;
	int num = -1;
	for (int p=0;p<MAXNUM;p++)
	{
		num = recv(m_Clients[p],buffer,1024,0);
		if (num!=-1)
		{
			curlink = p;
			break;
		}
	}
	buffer[num]=0;
	if (num == -1)   //接收客户端的链接
	{
		if (m_CurClient<MAXNUM)
		{
			m_Clients[m_CurClient]=accept(m_server,(sockaddr*)&serveraddr,&len);
			m_CurClient += 1;
		}
		return;
	}
	//将接收到的数据发送给客户端
	for (int j=0;j<m_CurClient;j++)
	{
		if (j!=curlink)
		{
			send(m_Clients[j],buffer,num,0);
		}
	}
}


BOOL CserverDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message==2000)
	{
		HandleData();
		return TRUE;
	}
	else
	{
		return CDialogEx::PreTranslateMessage(pMsg);
	}	
}
