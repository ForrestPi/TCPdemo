
// ClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
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


// CClientDlg �Ի���



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


// CClientDlg ��Ϣ�������

BOOL CClientDlg::OnInitDialog()
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
	m_client = socket(AF_INET,SOCK_STREAM,0);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CClientDlg::OnPaint()
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
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlg::OnBnClickedOk()     //���Ӱ�ť
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//�������˵�ַ
	sockaddr_in serveraddr;
	UpdateData(TRUE);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(m_port);
	//m_IPתconst char*cp;
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
		MessageBox(L"����ʧ��");
		return;
	}
	else
	{
		MessageBox(L"���ӳɹ�");
	}
	WSAAsyncSelect(m_client,m_hWnd,1000,FD_READ);
	CString str,info;
	m_name.GetWindowTextW(str);

	info.Format(L"%s------->%s",str,L"����������");
	//cstringתchar*
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


void CClientDlg::OnBnClickedButton2()     //���Ͱ�ť
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString str,name,info;
	m_name.GetWindowTextW(name);
	m_info.GetWindowTextW(str);
	if (!name.IsEmpty()&&!str.IsEmpty())
	{
		info.Format(L"%s˵��%s",name,str);
		//��ʼ��������
		//CStringתconst char*
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
		int nRow = m_list.InsertItem(0, info);//������
		m_info.SetWindowTextW(L"");
	}

}

void CClientDlg::ReceiveData()
{
	char buffer[1024];
	//���շ������˴���������
	int num = recv(m_client,buffer,1024,0);
	buffer[num] = 0;
	//char*תCString
	DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, buffer, -1, NULL, 0)+1;//��ȡbufer����ʵ�ʴ洢����
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
	// TODO: �ڴ����ר�ô����/����û���
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
