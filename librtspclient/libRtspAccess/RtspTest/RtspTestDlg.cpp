
// RtspTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RtspTest.h"
#include "RtspTestDlg.h"

#include "libRtspAccess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	EnableActiveAccessibility();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CRtspTestDlg 对话框




CRtspTestDlg::CRtspTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRtspTestDlg::IDD, pParent)
{
	EnableActiveAccessibility();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRtspTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_url);
}

BEGIN_MESSAGE_MAP(CRtspTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CRtspTestDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CRtspTestDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CRtspTestDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CRtspTestDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CRtspTestDlg 消息处理程序

BOOL CRtspTestDlg::OnInitDialog()
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_url.SetWindowText("rtsp://192.168.0.213:8557/PSIA/Streaming/channels/2?videoCodecType=H.264");


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRtspTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRtspTestDlg::OnPaint()
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
HCURSOR CRtspTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CALLBACK CRtspTestDlg::AVDataCallBack(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData)
{
	return 0;
	static FILE* pFile = NULL;
	if (pFile == NULL)
	{
		pFile = fopen("record.H264", "wb");
	}

	if (pFile != NULL)
	{
		if (iAVSelect == 0)
		{
			fwrite(pAVData, iDataLen, 1, pFile);
		}
	}

// 	char data[64] = {0};
// 	sprintf(data, "Select = %d DataLen = %d w = %d h = %d\n", iAVSelect, iDataLen, iWidth, iHeight);
// 	OutputDebugStringA(data);
	return 0;
}

void CRtspTestDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	//_CrtSetBreakAlloc(706);
	//_CrtSetBreakAlloc(707);
	//_CrtSetBreakAlloc(1039);
	//_CrtSetBreakAlloc(1040);
	CString url;
	m_url.GetWindowText(url);
	
	//RTSP_StartStream("rtsp://192.168.0.214:8557/PSIA/Streaming/channels/2?videoCodecType=H.264", AVDataCallBack, (void *)this);
	RTSP_StartStream(url, AVDataCallBack, (void *)this);
}

void CRtspTestDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	RTSP_StopStream();
}

void CRtspTestDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	for(int i = 0; i < 50; i++)
	{
		RTSP_StartStream("rtsp://192.168.0.214:8557/PSIA/Streaming/channels/2?videoCodecType=H.264", AVDataCallBack, (void *)this);
		Sleep(3000);
		RTSP_StopStream();
		Sleep(1000);
	}
	
}

void CRtspTestDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	srand((unsigned int)time(NULL));
	char* newtest = NULL;
	for (int i = 0; i < 100000; i++)
	{
		char temp[64];
		int size = rand()%(1024*1024*30) + 30*1024*1024;

		sprintf(temp, "%d %d\n", i, size);
		OutputDebugStringA(temp);

		newtest = new char[size];
		memset(newtest, 7, size);
		//Sleep(100);
		delete[] newtest;
		newtest = NULL;
	}



}
