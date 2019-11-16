
// VideoCaptureDlg.h : 头文件
//

#pragma once

#include <vfw.h>             //video for windows (vfw) 视频库头文件
#pragma comment(lib,"vfw32") //video for windows (vfw) 视频静态库文件

class AvPublisher;

// CVideoCaptureDlg 对话框
class CVideoCaptureDlg : public CDialogEx
{
	// 构造
public:
	CVideoCaptureDlg(CWnd* pParent = NULL);	// 标准构造函数

	// 对话框数据
	enum { IDD = IDD_VIDEOCAPTURE_DIALOG };

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
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

public:
	CStatic m_Panel;
	CStatic m_Text;
	void StartCapture(void);
	void CaptureStatus(int nID, LPCTSTR lpsz);
	void VideoStream(LPVIDEOHDR lpVideoHdr);
	void RenderRGB(LPVIDEOHDR lpVideoHdr, BITMAPINFO bitmap_info);
	void RenderYUV(LPVIDEOHDR lpVideoHdr, BITMAPINFO bitmap_info);
	void CreateBitmapInfo(BITMAPINFO* pSrcBitmapInfo, BITMAPINFO* pDstBitmapInfo, int width, int height);
	void DisplayPicture(BITMAPINFO bitmap_info, int width, int height);
	void LOG(CString text);
	void StopCapture();
	void CleanUp();

private:
	void dumpYuy2(LPBYTE lpData, DWORD dwBytesUsed);
	void dumpRgb(unsigned char* rgb, int size);
	void dumpYuv420p(unsigned char* yuv420p, int size);
	void dumpH264(unsigned char* x264nal, int size);

private:
	CString         str;

	BOOL            m_bCapture;
	HWND			m_hWndVideo;
	// video data
	unsigned char*  m_Rgb_data;
	unsigned char*  m_Yuv_data;
	// bitmap_info
	BITMAPINFO		m_BitmapInfo;
	// dump files
	BOOL            m_DumpYuy2;
	FILE*			m_FileYuy2;
	BOOL            m_DumpRgb;
	FILE*			m_FileRgb;
	BOOL            m_DumpYuv420p;
	FILE*			m_FileYuv420p;
	BOOL            m_DumpH264;
	FILE*			m_FileH264;

	//fps
	int             m_fps;

	//AvPublisher
	AvPublisher* av_publisher_;
};
