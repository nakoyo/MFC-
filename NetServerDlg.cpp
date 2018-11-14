/*单向通信，客户端发送*/
// NetServerDlg.cpp : 实现文件
///Next 多个客户端

#include "stdafx.h"
#include "NetServer.h"
#include "NetServerDlg.h"
#include "process.h"
#include "Afxsock.h" 
#include "afxdialogex.h"
#include "winsock2.h"

#pragma comment(lib,"ws2_32.lib")
#ifdef _DEBUG

//所有的类定义放到这里头
#define new DEBUG_NEW


#endif
/*Version2.0 2018-10-23  确定结构体进行数据传递*/
////自定义
	HANDLE hThread;//Accept线程的句柄
	
	#define N 2 //客户端最大数
	CTime m_time;//定义时间
	int n=0;//当前客户端数量
	volatile BOOL m_Flag=TRUE;//accept的标志量，生命周期
	SOCKET newsock[N+1],sock_server;//定义套接字
	struct sockaddr_in addr,client_addr;
	int addr_len =sizeof(struct sockaddr_in);
	/*数据相关*/
	CString serverIp;
	char buff[256];//发送数据流
	int size;//发送长度
	char getbuff[256];//获取数据流
	int getsize;//
	/*数据相关*/
////自定义
	//线程数据
	 struct threadInfo{
		HANDLE headThread;
		int numThread;
		SOCKET sockThread;
		CString mesThread;
	}data[N];
	// threadInfo myinfo;
	//
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


// CNetServerDlg 对话框




CNetServerDlg::CNetServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNetServerDlg::IDD, pParent)
	, md_static(_T(""))
	, md_hostname(_T(""))
	, md_ip(_T(""))
	, md_log(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNetServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, EDIT_STATIC, md_static);
	DDX_Text(pDX, EDIT_HOSTNAME, md_hostname);
	DDX_Text(pDX, EDIT_IP, md_ip);
	DDX_Text(pDX, EDIT_LOG, md_log);
}

BEGIN_MESSAGE_MAP(CNetServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_MY_THREAD_MESSAGE,OnMyThreadMessage)
	ON_BN_CLICKED(BUT_Server, &CNetServerDlg::OnBnClickedServer)
	ON_BN_CLICKED(BUT_STOP, &CNetServerDlg::OnBnClickedStop)
END_MESSAGE_MAP()


// CNetServerDlg 消息处理程序

BOOL CNetServerDlg::OnInitDialog()
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
	md_log+=_T("Start The Application\r\n");
	GetDlgItem(BUT_STOP)->EnableWindow(FALSE);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CNetServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CNetServerDlg::OnPaint()
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
HCURSOR CNetServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*客户端内容线程*/
UINT ThreadClient(LPVOID lpParameter){
	//////////窗体消息
	CNetServerDlg *dlg;
	dlg=(CNetServerDlg *)lpParameter;
	DWORD id=GetCurrentThreadId();
	///////////结构体消息
	threadInfo *pInfo=(threadInfo*)lpParameter;
	int num = (int)pInfo->numThread;
	CString s;
	s.Format(_T("此客户端的id：%d",pInfo->numThread));
	AfxMessageBox((s));
	s=pInfo->mesThread;
	s.Format(_T("线程携带的消息："+s));
	AfxMessageBox((s));
	///////////
	PostMessage(dlg->m_hWnd,WM_MY_THREAD_MESSAGE,id,1);
	//if(getsize=recv(newsock[n]
	while(!m_Flag){
		AfxEndThread(0);
	}
	return 0;
}

/*服务器accept接受线程*/
UINT ThreadAccept(LPVOID param)
{
	//初始化消息事件
	CNetServerDlg *dlg;
	dlg=(CNetServerDlg *)param;
	//
	n=0;
	while(m_Flag)
	{
		n++;
		if(true){
			newsock[n]=accept(sock_server,(LPSOCKADDR)&client_addr,&addr_len);
				if(newsock[n]!=INVALID_SOCKET){
					//创建客户端线程，实现多线程
					//interface
					data[n].mesThread="我是第一个！";
					data[n].sockThread=newsock[n];
					data[n].numThread=n;
					CWinThread *pThreadClient=AfxBeginThread(ThreadClient,&data[n]);
					data[n].headThread=pThreadClient->m_hThread;
					
					//CString s="Server said:"+serverIp+"\r\n";
					//sprintf(buff,"%s",s);
					//if(size=send(newsock[n],buff,sizeof(buff),0)>0){
					//	AfxMessageBox(_T("发送成功"));
					//}
					///*再接受客户端的消息*/
					//while(true)
					//{
					//	getsize=recv(newsock[n],getbuff,sizeof(getbuff),0);
					//	if(getsize>0)
					//	{
					//		PostMessage(dlg->m_hWnd,WM_MY_THREAD_MESSAGE,1,1);
					//	}
					//}
				}
				else {n --;closesocket(newsock[n]);}
		}
		else break;
	}
	
		AfxEndThread(0);
		return 0;
}
	/*
	* 接受 客户端的消息
	*/
//开启服务
void CNetServerDlg::OnBnClickedServer()
{
	
	WSADATA wsaData;
	WORD wVersionRequested=MAKEWORD(2,2);
	if(WSAStartup(wVersionRequested,&wsaData)!=0){
		AfxMessageBox(_T("加载套接字失败"));
		exit(0);
	}
	/*创建套接字*/
	if((sock_server=socket(AF_INET,SOCK_STREAM,0))<0){
		AfxMessageBox(_T("加载套接字失败"));
		WSACleanup();
		exit(0);
	}
	/*设置套接字为非阻塞模式*/
	unsigned long ul = 1;
	int nRet =ioctlsocket(sock_server,FIONBIO,(unsigned long *)&ul);
	if(nRet == SOCKET_ERROR){
		AfxMessageBox(_T("设置非阻塞模式失败"));
		closesocket(sock_server);
		WSACleanup();
		exit(0);
	}
	/*绑定端口*/
	memset((void *)&addr,0,addr_len); //将sockaddr初始化为0
	addr.sin_family=AF_INET;          //协议族
	addr.sin_port=htons(65432);       //端口号
	addr.sin_addr.s_addr=htonl(INADDR_ANY); //监听本机分配的所有IP地址
	if((bind(sock_server,(struct sockaddr *)&addr,sizeof(addr)))!=0)
	{
		AfxMessageBox(_T("绑定失败！"));
		closesocket(sock_server);
		WSACleanup();
		exit(0);
	}
	/*开始监听*/
	if(listen(sock_server,0)!=0)
	{
		AfxMessageBox(_T("监听失败！"));
		closesocket(sock_server);
		exit(0);
	} 
	else{	
			m_time=CTime::GetCurrentTime();
			md_log+=m_time.Format(_T("%Y-%m-%d %H:%M:%S\r\n"));
			md_log+=_T("服务器成功开启！\r\n");
			md_static="Listening...";//界面显示状态
			GetDlgItem(BUT_Server)->EnableWindow(FALSE);
			GetDlgItem(BUT_STOP)->EnableWindow(TRUE);
			//更改启动按钮，避免重复启动
			char name[128];CString str;
			hostent * pHost;
			gethostname(name,128);
			str.Format(_T("%s  "),name);
			md_hostname=str;
			
			pHost=gethostbyname(name);
			
			str.Format(_T("%s"),inet_ntoa(*((in_addr *)pHost->h_addr)));
			md_ip=str;
			serverIp=str;
			UpdateData(FALSE);
			CEdit *pEditLog=(CEdit *)GetDlgItem(EDIT_LOG);
			pEditLog->LineScroll(pEditLog->GetLineCount()-1);
	}
	//创建Accept线程
	CWinThread *pThreadAccept=AfxBeginThread(ThreadAccept,this);
	hThread=pThreadAccept->m_hThread;
}
	/*
	*清楚状态，停止服务
	*/
void CNetServerDlg::OnBnClickedStop()
{
	// TODO: 在此添加控件通知处理程序代码
	WSACleanup();
	closesocket(sock_server);
	for(int i=0;i<N;i++){closesocket(newsock[i]);}
	md_static="STOP";m_Flag=FALSE;//退出accept线程的循环
	md_ip="";md_hostname="";
	GetDlgItem(BUT_Server)->EnableWindow(TRUE);
	m_time=CTime::GetCurrentTime();
	md_log+=m_time.Format(_T("%Y-%m-%d %H:%M:%S\r\n"));
	md_log+=_T("服务器关闭！\r\n");
	UpdateData(FALSE);
	CEdit *pEditLog=(CEdit *)GetDlgItem(EDIT_LOG);
	pEditLog->LineScroll(pEditLog->GetLineCount()-1);
}
	/*
	*线程消息事件
	*/
afx_msg LRESULT CNetServerDlg::OnMyThreadMessage(WPARAM wParam,LPARAM lParam){
	m_time=CTime::GetCurrentTime();
	md_log+=m_time.Format(_T("%Y-%m-%d %H:%M:%S\r\n"));
	CString s="   Client said:";
	CString end="\r\n";
	md_log+=(_T(s+CA2CT(getbuff)+end));
	memset(getbuff,0,sizeof(getbuff)/sizeof(char));
	UpdateData(FALSE);
	CEdit *pEditLog=(CEdit *)GetDlgItem(EDIT_LOG);
	pEditLog->LineScroll(pEditLog->GetLineCount()-1);
	return 0;
}
