
// VideoCaptureDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VideoCapture.h"
#include "VideoCaptureDlg.h"
#include "afxdialogex.h"

#include "AvPublisher.h"

#include "Utils.h"
#include "text_water_mark.h"

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


// CVideoCaptureDlg 对话框

CVideoCaptureDlg::CVideoCaptureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoCaptureDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bCapture    = FALSE;

	m_Rgb_data    = NULL;
	m_Yuv_data    = NULL;

	m_DumpYuy2    = FALSE;
	m_FileYuy2    = NULL;
	m_DumpRgb     = FALSE;
	m_FileRgb     = NULL;
	m_DumpYuv420p = FALSE;
	m_FileYuv420p = NULL;
	m_DumpH264    = TRUE;
	m_FileH264    = NULL;

	av_publisher_  = NULL;
}

void CVideoCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_Panel);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_Text);
}

BEGIN_MESSAGE_MAP(CVideoCaptureDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CVideoCaptureDlg 消息处理程序

BOOL CVideoCaptureDlg::OnInitDialog()
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
	StartCapture();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CVideoCaptureDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CVideoCaptureDlg::OnPaint()
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
HCURSOR CVideoCaptureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/************************************************************************/
/*                OnStatusCallBack  视频采集回调方法                    */
/************************************************************************/
LRESULT WINAPI OnStatusCallBack(HWND hWnd ,int nID, LPCTSTR lpsz)
{
	//CVideoCaptureDlg* pDlg = (CVideoCaptureDlg*)AfxGetMainWnd();
	//VC++6.0下可以使用，VS2010崩溃。有2种方法可以解决。

	CVideoCaptureDlg* pDlg = (CVideoCaptureDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->CaptureStatus(nID, lpsz);

	return 1;
}

/************************************************************************/
/*              OnVideoStreamCallBack  视频采集回调方法                 */
/************************************************************************/
LRESULT WINAPI OnVideoStreamCallBack(HWND hWnd , LPVIDEOHDR lpVideoHdr)
{
	//CVideoCaptureDlg* pDlg = (CVideoCaptureDlg*)AfxGetMainWnd();
	//VC++6.0下可以使用，VS2010崩溃。有2种方法可以解决。

	CVideoCaptureDlg* pDlg = (CVideoCaptureDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->VideoStream(lpVideoHdr);

	return 1;
}

void CVideoCaptureDlg::StartCapture(void)
{
	m_hWndVideo = capCreateCaptureWindow(NULL, WS_POPUP, 0, 0, 10, 10, m_hWnd, 0);
	//最后一个参数1和0有区别
	//20190416，最后一个参数值1失效，直接的结果是导致capDriverConnect一直失败
	//20190416，参数一个参数值改为0，最终可以使capDriverConnect成功

	//连接驱动程序
	while ( !capDriverConnect(m_hWndVideo,0))
	{
		;
	}

	::SetParent(m_hWndVideo , *this);
	::SetWindowLong(m_hWndVideo , GWL_STYLE , WS_CHILD);

	CRect wndRC;
	m_Panel.GetClientRect(wndRC);
	m_Panel.MapWindowPoints(this , wndRC);
	wndRC.DeflateRect(1,1,1,1);

	::SetWindowPos(m_hWndVideo , NULL , wndRC.left , wndRC.top , wndRC.Width() , wndRC.Height() , SWP_NOZORDER);
	::ShowWindow(m_hWndVideo , SW_SHOW);

	CAPDRIVERCAPS caps;  //defines the capabilities of the capture driver
	capDriverGetCaps(m_hWndVideo , &caps, sizeof(caps) );

	if (caps.fHasOverlay)
	{
		capOverlay(m_hWndVideo , FALSE);
	}

	CAPTUREPARMS cap_params ; //contains parameters that control the streaming video capture process
	capCaptureGetSetup(m_hWndVideo , &cap_params , sizeof(cap_params)) ;

	cap_params.fYield			= TRUE ;   //如果为TRUE，则将在后台执行
	cap_params.fAbortLeftMouse  = FALSE;
	cap_params.fAbortRightMouse = FALSE;
	cap_params.fLimitEnabled	= FALSE;
	cap_params.vKeyAbort		= FALSE;
	cap_params.fCaptureAudio	= FALSE;

	m_fps = 1000*1000* 1.0 /cap_params.dwRequestMicroSecPerFrame + 0.5; // 0.5是为了四舍五入

	capCaptureSetSetup(m_hWndVideo, &cap_params, sizeof(cap_params));
	capSetCallbackOnStatus(m_hWndVideo, OnStatusCallBack);
	capSetCallbackOnVideoStream(m_hWndVideo, OnVideoStreamCallBack);

	capPreviewRate(m_hWndVideo, 30);
	capPreview(m_hWndVideo, TRUE);

	//开始捕捉
	//使用 capSetCallbackOnFrame 的时候，可以不用 capCaptureSequenceNoFile ，但是视频帧里面没有时间信息
	//使用 capSetCallbackOnVideoStream 的时候，要用 capCaptureSequenceNoFile
	capCaptureSequenceNoFile(m_hWndVideo);
}

void CVideoCaptureDlg::CaptureStatus(int nID, LPCTSTR lpsz)
{
	CString str;
	switch(nID)
	{
	case IDS_CAP_BEGIN:
		m_bCapture = TRUE;
		//start_time;
		break;
	case IDS_CAP_END:
		m_bCapture = FALSE;
		CleanUp();
		break;
	default:
		break;
	}
}

void CVideoCaptureDlg::VideoStream(LPVIDEOHDR lpVideoHdr)
{
	capGetVideoFormat(m_hWndVideo , &m_BitmapInfo , sizeof(BITMAPINFO));

	str.Format(_T("time:%ds, flag:%d, dwBytesUsed:%d, biBitCount:%d, biWidth:%d, biHeight:%d, biCompression:%d, biSize:%d, biSizeImage:%d"),
		lpVideoHdr->dwTimeCaptured/1000, lpVideoHdr->dwFlags, lpVideoHdr->dwBytesUsed,
		m_BitmapInfo.bmiHeader.biBitCount,
		m_BitmapInfo.bmiHeader.biWidth,
		m_BitmapInfo.bmiHeader.biHeight, 
		m_BitmapInfo.bmiHeader.biCompression,
		m_BitmapInfo.bmiHeader.biSize,
		m_BitmapInfo.bmiHeader.biSizeImage);
	LOG(str);

	switch(m_BitmapInfo.bmiHeader.biCompression)
	{
	case BI_RGB:
		RenderRGB(lpVideoHdr, m_BitmapInfo);
		break;
	case BI_RLE8:
		RenderRGB(lpVideoHdr, m_BitmapInfo);
		break;
	case BI_RLE4:
		RenderRGB(lpVideoHdr, m_BitmapInfo);
		break;
	case BI_BITFIELDS:
		RenderRGB(lpVideoHdr, m_BitmapInfo);
		break;
	case 844715353:
		RenderYUV(lpVideoHdr, m_BitmapInfo);
		break;
	default:
		break;
	}
}

void CVideoCaptureDlg::RenderRGB(LPVIDEOHDR lpVideoHdr, BITMAPINFO bitmap_info)
{
	// 在使用GDI渲染RGB数据时：
	// 注意BMP在y方向是反着存储的，一次必须设置一个负值，才能使图像正着显示出来   
	LONG bitmap_h = bitmap_info.bmiHeader.biHeight;
	bitmap_info.bmiHeader.biHeight = bitmap_h > 0 ? (-1*bitmap_h) : bitmap_h;

	CRect rect ;
	GetClientRect(rect);
	CDC* pDC = GetDC();

	LONG image_w = bitmap_info.bmiHeader.biWidth;
	LONG image_h = abs(bitmap_info.bmiHeader.biHeight);

	int canvas_w = rect.Width()/2;
	int canvas_h = canvas_w * image_h/image_w;

	// 大小端转换
	if (m_Rgb_data == NULL)
	{
		m_Rgb_data = new unsigned char[lpVideoHdr->dwBytesUsed];
	}
	memset(m_Rgb_data, 0, lpVideoHdr->dwBytesUsed);
	memcpy(m_Rgb_data, lpVideoHdr->lpData, lpVideoHdr->dwBytesUsed);

	change_endian_picture_rgb(m_Rgb_data, image_w, image_h, bitmap_info.bmiHeader.biBitCount);

	StretchDIBits(pDC->GetSafeHdc(),
		0, 0, canvas_w, canvas_h,
		0, 0, image_w , image_h,
		(CONST VOID*)m_Rgb_data, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(pDC) ;
}

void CVideoCaptureDlg::RenderYUV(LPVIDEOHDR lpVideoHdr, BITMAPINFO bitmap_info)
{
	int width  = bitmap_info.bmiHeader.biWidth;
	int height = bitmap_info.bmiHeader.biHeight;
	int rgb_size = width*height*3;
	int yuv_size = width*height*3/2;

	// 1. yuy2
	dumpYuy2(lpVideoHdr->lpData, lpVideoHdr->dwBytesUsed);

	// 2. rgb24
	if (m_Rgb_data == NULL)
	{
		m_Rgb_data = new unsigned char[rgb_size];
	}
	YUY2_to_RGB(lpVideoHdr->lpData, m_Rgb_data, lpVideoHdr->dwBytesUsed);
	//YUY2_to_RGB_2(lpVideoHdr->lpData, rgb, lpVideoHdr->dwBytesUsed);
	//以上2种YUY2转换RGB24的方式各有利弊
	//方式1转换结果：颜色没有发生变化，只是高亮度的区域显示斑点
	//方式2转换结果：颜色发生了变化，但是显示没有出现斑点
	//所以还有可以优化的空间

	dumpRgb(m_Rgb_data, rgb_size);

	// 3. render rgb24
	// 在使用GDI渲染RGB数据时：
	// 注意BMP在y方向是反着存储的，因此高度必须设置一个负值，才能使图像正着显示出来 bitmap_info_temp
	BITMAPINFO bitmap_info_dst;
	CreateBitmapInfo(&bitmap_info, &bitmap_info_dst, width, height);
	change_endian_picture_rgb(m_Rgb_data, width, height, 24);
	DisplayPicture(bitmap_info_dst, width, height);

	// 4. yuv420p
	if (m_Yuv_data == NULL)
	{
		m_Yuv_data = new unsigned char[yuv_size];
	}
	YUY2_YUV420P(lpVideoHdr->lpData, m_Yuv_data, width, height);

	char strTime[256];
	memset(strTime, 0, sizeof(strTime));
	GetCurrentSysTime(strTime);

	draw_Font_Func((char*)m_Yuv_data, width, height, strTime, 20, 10, 0);

	dumpYuv420p(m_Yuv_data, yuv_size);

	// 5. publish
	if (av_publisher_ == NULL)
	{
		//char* url = "rtmp://192.168.61.129:1935/live/123";
		char* url = "rtmp://localhost/live/stream";
		av_publisher_ = new AvPublisher();
		av_publisher_->SetVideoParams(AvPublisher::PUBLISHER_COLOR_SPACE_TYPE_YUV420P, width, height, 480000, m_fps);
		av_publisher_->StartRtmp(url);
	}
	av_publisher_->PublishVideoData(m_Yuv_data, yuv_size);
}

void CVideoCaptureDlg::CreateBitmapInfo(BITMAPINFO* pSrcBitmapInfo, BITMAPINFO* pDstBitmapInfo, int width, int height)
{
	if (pSrcBitmapInfo)
	{
		memcpy(pDstBitmapInfo, pSrcBitmapInfo, sizeof(BITMAPINFO));
	}
	else
	{
		DWORD dwBmpHdr = sizeof(BITMAPINFO);
		pDstBitmapInfo->bmiHeader.biClrImportant = 0;
		pDstBitmapInfo->bmiHeader.biSize = dwBmpHdr;
		pDstBitmapInfo->bmiHeader.biSizeImage = 0;
		pDstBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
		pDstBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
		pDstBitmapInfo->bmiHeader.biClrUsed = 0;
		pDstBitmapInfo->bmiHeader.biPlanes = 1;
		pDstBitmapInfo->bmiHeader.biWidth = width;
	}
	pDstBitmapInfo->bmiHeader.biHeight = -1*height;
	pDstBitmapInfo->bmiHeader.biCompression = BI_RGB;
	pDstBitmapInfo->bmiHeader.biBitCount = 24;
}

void CVideoCaptureDlg::DisplayPicture(BITMAPINFO bitmap_info, int width, int height)
{
	CRect rect ;
	GetClientRect(rect);
	CDC* pDC = GetDC();
	HDC pHDC = pDC->GetSafeHdc();

	int canvas_w = rect.Width()/4;
	int canvas_h = canvas_w * height/width;

	SetStretchBltMode(pHDC, HALFTONE);
	StretchDIBits(pHDC,
		0, 0, canvas_w, canvas_h,
		0, 0, width, height,
		(CONST VOID*)m_Rgb_data, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(pDC);
}

void CVideoCaptureDlg::dumpYuy2(LPBYTE lpData, DWORD dwBytesUsed)
{
	if (m_DumpYuy2 && NULL == m_FileYuy2)
	{
		m_FileYuy2 = fopen("test.yuy2", "wb");
	}
	if (m_FileYuy2)
	{
		fwrite(lpData, 1, dwBytesUsed, m_FileYuy2);
		fflush(m_FileYuy2);
	}
}

void CVideoCaptureDlg::dumpRgb(unsigned char* rgb, int size)
{
	if (m_DumpRgb && NULL == m_FileRgb)
	{
		m_FileRgb = fopen("test.rgb", "wb");
	}
	if (m_FileRgb)
	{
		fwrite(rgb, 1, size, m_FileRgb);
		fflush(m_FileRgb);
	}
}

void CVideoCaptureDlg::dumpYuv420p(unsigned char* yuv420p, int size)
{
	if (m_DumpYuv420p && NULL == m_FileYuv420p)
	{
		m_FileYuv420p = fopen("test.yuv420p", "wb");
	}
	if (m_FileYuv420p)
	{
		fwrite(yuv420p, 1, size, m_FileYuv420p);
		fflush(m_FileYuv420p);
	}
}

void CVideoCaptureDlg::dumpH264(unsigned char* x264nal, int size)
{
	if (m_DumpH264 && NULL == m_FileH264)
	{
		m_FileH264 = fopen("test.h264", "wb");
	}
	if (m_FileH264)
	{
		fwrite(x264nal, 1, size, m_FileH264);
		fflush(m_FileH264);
	}
}

void CVideoCaptureDlg::LOG(CString text)
{
	m_Text.SetWindowText(text);
}

void CVideoCaptureDlg::StopCapture()
{
	//终止回调函数
	// 	capSetCallbackOnFrame(m_hWndVideo , NULL);	
	capSetCallbackOnVideoStream(m_hWndVideo , NULL);
	//停止捕捉
	capCaptureStop(m_hWndVideo);
	//停止驱动程序连接
	capDriverDisconnect(m_hWndVideo);
}

void CVideoCaptureDlg::CleanUp()
{
	if (m_Rgb_data)
	{
		delete[] m_Rgb_data;
		m_Rgb_data = NULL;
	}
	if (m_Yuv_data)
	{
		delete[] m_Yuv_data;
		m_Yuv_data = NULL;
	}
	if (m_FileYuy2)
	{
		fclose(m_FileYuy2);
		m_FileYuy2 = NULL;
	}
	if (m_FileRgb)
	{
		fclose(m_FileRgb);
		m_FileRgb = NULL;
	}
	if (m_FileYuv420p)
	{
		fclose(m_FileYuv420p);
		m_DumpYuv420p = NULL;
	}
	if (m_FileH264)
	{
		fclose(m_FileH264);
		m_FileH264 = NULL;
	}
	if (av_publisher_)
	{
		delete av_publisher_;
		av_publisher_ = NULL;
	}
}

void CVideoCaptureDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	StopCapture();
}
