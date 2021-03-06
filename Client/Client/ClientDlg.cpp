
// ClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"
#include "MoveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
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
	: CDialog(IDD_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	InitializeCriticalSection(&g_cs);//初始化临界区
}

CClientDlg::~CClientDlg()
{
	DeleteCriticalSection(&g_cs);//删除临界区
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, ip_address);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, &CClientDlg::OnBnClickedConnect)
	ON_WM_SIZE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CClientDlg 消息处理程序

BOOL CClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	GetClientRect(&m_rect); //获取对话框的大小
	const char * strIP = "192.168.3.71";
	DWORD dwIP;
	inet_pton(AF_INET, strIP, &dwIP);
	ip_address.SetAddress(ntohl(dwIP));
	SetDlgItemTextW(IDC_EDIT1, _T("8888"));


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
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
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
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlg::OnBnClickedConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!con_flag)
	{	
		if (!ConnectSocket())
		{
			MessageBox(_T("Socket连接出错!"));
			return;
		}
		
		if (!SendMsg("hello"))
		{
			MessageBox(_T("发送消息出错!"));
			return;
		}
		plotThread = ::AfxBeginThread(PlotImageThread, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);//绘图线程
		if (plotThread == NULL)
		{
			MessageBox(_T("线程创建失败!"));
			return;
		}
		heartThread = ::AfxBeginThread(HeartBeatThread, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);//心跳线程
		if (heartThread == NULL)
		{
			MessageBox(_T("线程创建失败!"));
			return;
		}
		con_flag = TRUE;
		GetDlgItem(IDC_CONNECT)->SetWindowTextW(_T("断开连接"));	
	}
	else
	{
		::shutdown(ClientSocket, SD_SEND);
		WaitForSingleObject(plotThread->m_hThread, INFINITE);//等待绘图线程结束

		EnterCriticalSection(&g_cs);//防止心跳线程不知道已关闭socket
		::closesocket(ClientSocket);
		ClientSocket = NULL;
		LeaveCriticalSection(&g_cs);
		GetDlgItem(IDC_CONNECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_CONNECT)->SetWindowTextW(_T("正在断开，请稍等"));
		WaitForSingleObject(heartThread->m_hThread, INFINITE);//等待心跳线程结束
		
		con_flag = FALSE;
		GetDlgItem(IDC_CONNECT)->SetWindowTextW(_T("连接"));
		GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);
		Invalidate();
	}

}


void CClientDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	Invalidate(); 
	MoveDlg::OnSize(this, m_rect, nType, cx, cy);
}

BOOL CClientDlg::ConnectSocket()
{
	/* Create Socket */
	ClientSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET)
	{
		MessageBox(_T("Socket创建失败!"));
		return FALSE;
	}
	struct sockaddr_in ClientAddr;
	ClientAddr.sin_family = AF_INET;
	BYTE f1, f2, f3, f4;
	ip_address.GetAddress(f1, f2, f3, f4);
	char ip[20];
	sprintf_s(ip, 20, "%d.%d.%d.%d", f1, f2, f3, f4);
	inet_pton(AF_INET, ip, &ClientAddr.sin_addr);
	CString port;
	GetDlgItem(IDC_EDIT1)->GetWindowTextW(port);
	ClientAddr.sin_port = htons(_ttoi(port));
	memset(ClientAddr.sin_zero, 0x00, 8);
	/* connect socket */
	if (::connect(ClientSocket, (struct sockaddr*)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)
	{
		MessageBox(_T("请求连接出错！"));
		return FALSE;
	}

	return TRUE;
}

BOOL CClientDlg::SendMsg(const char * msg)
{
	HEAD head;
	char buf[10];
	head.type = OTHER;
	head.length = (int)strlen(msg);
	memset(buf, 0, 10);
	memcpy(buf, &head, sizeof(head));
	//MessageBox(_T("测试1!"));
	EnterCriticalSection(&g_cs);//锁住连续发两个包，防止插入心跳包，对端接收混乱
	//MessageBox(_T("测试2!"));
	if (::send(ClientSocket, buf, sizeof(head), 0) == SOCKET_ERROR)
	{
		return FALSE;
	}
	if (::send(ClientSocket, msg, head.length, 0) == SOCKET_ERROR)
	{
		return FALSE;
	}
	LeaveCriticalSection(&g_cs);
	return TRUE;
}

BOOL CClientDlg::ReceiveImg()
{
	int iResult = 0;
	int lenReaded = 0;
	int lenLeaved = 0;
	img_data.length = 0;
	iResult = Recv((char*)&img_data.length, sizeof(int));
	if (iResult <= 0) 
	{
		return FALSE;
	}

	memset(img_data.RecvBuffer, 0, BUFFER_SIZE);
	lenLeaved = img_data.length;

	while (lenReaded < img_data.length)
	{
		iResult = Recv(img_data.RecvBuffer + lenReaded, lenLeaved);
		if (iResult <= 0)
		{
			return FALSE;
		}
		lenReaded += iResult;
		lenLeaved = img_data.length - lenReaded;
	}
	return TRUE;
}

int CClientDlg::Recv(char * recv_buf, int len)
{
	if (ClientSocket == INVALID_SOCKET || recv_buf == NULL)
	{
		return -1;
	}
	return ::recv(ClientSocket, recv_buf, len, 0);
}

BOOL CClientDlg::DrawImage()
{
	if (img_data.length <= 0)
	{
		return FALSE;
	}
	HGLOBAL hImageData = ::GlobalAlloc(GMEM_MOVEABLE, img_data.length);//创建全局内存对象
	if (hImageData == NULL)
	{
		return FALSE;
	}
	BYTE* pImgData = (BYTE*)::GlobalLock(hImageData);
	if (pImgData == NULL)
	{
		::GlobalFree(hImageData);
		return FALSE;
	}
	::memcpy(pImgData, img_data.RecvBuffer, img_data.length);//进行内存拷贝
	::GlobalUnlock(hImageData);
	IStream *pStmImg = NULL;
	if (::CreateStreamOnHGlobal(hImageData, FALSE, &pStmImg) != S_OK)//创建数据流
	{
		pStmImg->Release();
		::GlobalFree(hImageData);
		return FALSE;
	}
	Image *image = Image::FromStream(pStmImg);//通过流获得图像对象
	pStmImg->Release();

	CWnd* pwnd = GetDlgItem(IDC_DISPLAY);//IDC_DISPLAY 为图像控件的 ID 
	CDC* dc = pwnd->GetDC(); //获取图像控件的设备上下文 
	CRect rect;
	pwnd->GetClientRect(&rect); //获取客户区域的信息 
	Graphics graph(dc->GetSafeHdc());
	graph.DrawImage(image, 0, 0, rect.Width(), rect.Height()); //在指定图像控件的区域中绘制图像 
	ReleaseDC(dc); //释放资源 
	::GlobalFree(hImageData);
	return TRUE;
}

UINT CClientDlg::PlotImageThread(LPVOID param)//绘图线程
{
	CClientDlg *myDlg = (CClientDlg*)param;
	myDlg->Invalidate();
	while (1)
	{
		if (!myDlg->ReceiveImg())
		{
			break;
		}
		if (!myDlg->DrawImage())
		{
			break;
		}
	}
	return 0;
}

UINT CClientDlg::HeartBeatThread(LPVOID param)//心跳线程
{
	CClientDlg *myDlg = (CClientDlg*)param;
	HEAD head;
	char buf[10];
	while (1)
	{
		head.type = HEART;
		head.length = 0;
		memset(buf, 0, 10);
		memcpy(buf, &head, sizeof(head));
		EnterCriticalSection(&myDlg->g_cs);
		if (myDlg->ClientSocket == NULL)
		{
			break;
		}
		if (::send(myDlg->ClientSocket, buf, sizeof(head), 0) == SOCKET_ERROR)
		{
			myDlg->MessageBox(_T("发送心跳包出错!"));
			break;
		}
		LeaveCriticalSection(&myDlg->g_cs);
		Sleep(3000);
	}
	LeaveCriticalSection(&myDlg->g_cs);
	return 0;
}


void CClientDlg::OnClose()
{
	CDialog::OnCancel();
}
void CClientDlg::OnCancel(){}
void CClientDlg::OnOK(){}
