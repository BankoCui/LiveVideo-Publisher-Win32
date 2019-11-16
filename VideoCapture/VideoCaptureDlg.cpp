
// VideoCaptureDlg.cpp : ʵ���ļ�
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


// CVideoCaptureDlg �Ի���

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


// CVideoCaptureDlg ��Ϣ�������

BOOL CVideoCaptureDlg::OnInitDialog()
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
	StartCapture();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CVideoCaptureDlg::OnPaint()
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
HCURSOR CVideoCaptureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/************************************************************************/
/*                OnStatusCallBack  ��Ƶ�ɼ��ص�����                    */
/************************************************************************/
LRESULT WINAPI OnStatusCallBack(HWND hWnd ,int nID, LPCTSTR lpsz)
{
	//CVideoCaptureDlg* pDlg = (CVideoCaptureDlg*)AfxGetMainWnd();
	//VC++6.0�¿���ʹ�ã�VS2010��������2�ַ������Խ����

	CVideoCaptureDlg* pDlg = (CVideoCaptureDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->CaptureStatus(nID, lpsz);

	return 1;
}

/************************************************************************/
/*              OnVideoStreamCallBack  ��Ƶ�ɼ��ص�����                 */
/************************************************************************/
LRESULT WINAPI OnVideoStreamCallBack(HWND hWnd , LPVIDEOHDR lpVideoHdr)
{
	//CVideoCaptureDlg* pDlg = (CVideoCaptureDlg*)AfxGetMainWnd();
	//VC++6.0�¿���ʹ�ã�VS2010��������2�ַ������Խ����

	CVideoCaptureDlg* pDlg = (CVideoCaptureDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->VideoStream(lpVideoHdr);

	return 1;
}

void CVideoCaptureDlg::StartCapture(void)
{
	m_hWndVideo = capCreateCaptureWindow(NULL, WS_POPUP, 0, 0, 10, 10, m_hWnd, 0);
	//���һ������1��0������
	//20190416�����һ������ֵ1ʧЧ��ֱ�ӵĽ���ǵ���capDriverConnectһֱʧ��
	//20190416������һ������ֵ��Ϊ0�����տ���ʹcapDriverConnect�ɹ�

	//������������
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

	cap_params.fYield			= TRUE ;   //���ΪTRUE�����ں�ִ̨��
	cap_params.fAbortLeftMouse  = FALSE;
	cap_params.fAbortRightMouse = FALSE;
	cap_params.fLimitEnabled	= FALSE;
	cap_params.vKeyAbort		= FALSE;
	cap_params.fCaptureAudio	= FALSE;

	m_fps = 1000*1000* 1.0 /cap_params.dwRequestMicroSecPerFrame + 0.5; // 0.5��Ϊ����������

	capCaptureSetSetup(m_hWndVideo, &cap_params, sizeof(cap_params));
	capSetCallbackOnStatus(m_hWndVideo, OnStatusCallBack);
	capSetCallbackOnVideoStream(m_hWndVideo, OnVideoStreamCallBack);

	capPreviewRate(m_hWndVideo, 30);
	capPreview(m_hWndVideo, TRUE);

	//��ʼ��׽
	//ʹ�� capSetCallbackOnFrame ��ʱ�򣬿��Բ��� capCaptureSequenceNoFile ��������Ƶ֡����û��ʱ����Ϣ
	//ʹ�� capSetCallbackOnVideoStream ��ʱ��Ҫ�� capCaptureSequenceNoFile
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
	// ��ʹ��GDI��ȾRGB����ʱ��
	// ע��BMP��y�����Ƿ��Ŵ洢�ģ�һ�α�������һ����ֵ������ʹͼ��������ʾ����   
	LONG bitmap_h = bitmap_info.bmiHeader.biHeight;
	bitmap_info.bmiHeader.biHeight = bitmap_h > 0 ? (-1*bitmap_h) : bitmap_h;

	CRect rect ;
	GetClientRect(rect);
	CDC* pDC = GetDC();

	LONG image_w = bitmap_info.bmiHeader.biWidth;
	LONG image_h = abs(bitmap_info.bmiHeader.biHeight);

	int canvas_w = rect.Width()/2;
	int canvas_h = canvas_w * image_h/image_w;

	// ��С��ת��
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
	//����2��YUY2ת��RGB24�ķ�ʽ��������
	//��ʽ1ת���������ɫû�з����仯��ֻ�Ǹ����ȵ�������ʾ�ߵ�
	//��ʽ2ת���������ɫ�����˱仯��������ʾû�г��ְߵ�
	//���Ի��п����Ż��Ŀռ�

	dumpRgb(m_Rgb_data, rgb_size);

	// 3. render rgb24
	// ��ʹ��GDI��ȾRGB����ʱ��
	// ע��BMP��y�����Ƿ��Ŵ洢�ģ���˸߶ȱ�������һ����ֵ������ʹͼ��������ʾ���� bitmap_info_temp
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
	//��ֹ�ص�����
	// 	capSetCallbackOnFrame(m_hWndVideo , NULL);	
	capSetCallbackOnVideoStream(m_hWndVideo , NULL);
	//ֹͣ��׽
	capCaptureStop(m_hWndVideo);
	//ֹͣ������������
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
