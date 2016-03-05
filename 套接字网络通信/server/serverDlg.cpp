
// serverDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "server.h"
#include "serverDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CserverDlg �Ի���



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


// CserverDlg ��Ϣ�������

BOOL CserverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//�����׽���
	m_server = socket(AF_INET,SOCK_STREAM,0);
	//�������е��¼����������ڵ���Ϣ��
	WSAAsyncSelect(m_server,m_hWnd,2000,FD_WRITE|FD_READ|FD_ACCEPT);
	m_client = 0;
	m_serverIP = L"";
	for (int i=0;i<MAXNUM;i++)
	{
		m_Clients[i] = 0;
	}
	m_CurClient = 0;
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CserverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CserverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CserverDlg::OnBnClickedOk()  //���������ť
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//�������˵�ַ
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	m_IP.GetWindowText(m_serverIP);
	//���ñ�����ַ
	//CStringתconst char*
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
	//�󶨵�ַ
	if (bind(m_server,(sockaddr*)&serveraddr,sizeof(serveraddr)))
	{
		MessageBox(L"�󶨵�ַʧ��");
		return;
	}
	//��ʼ����
	listen(m_server,50);

	//CDialogEx::OnOK();
}


void CserverDlg::HandleData(void)
{
	sockaddr_in serveraddr;
	char buffer[1024];
	int len = sizeof(serveraddr);
	//���տͻ��˵�����
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
	if (num == -1)   //���տͻ��˵�����
	{
		if (m_CurClient<MAXNUM)
		{
			m_Clients[m_CurClient]=accept(m_server,(sockaddr*)&serveraddr,&len);
			m_CurClient += 1;
		}
		return;
	}
	//�����յ������ݷ��͸��ͻ���
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
	// TODO: �ڴ����ר�ô����/����û���
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
