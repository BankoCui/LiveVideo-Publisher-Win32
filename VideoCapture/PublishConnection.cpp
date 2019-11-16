#include "StdAfx.h"
#include "PublishConnection.h"
#include "Packetizer.h"

extern "C"
{
#include "librtmp/rtmp.h"
}

// class PublishConnection
PublishConnection::PublishConnection(void)
{
	h_stream_thread_ = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendDataThreadFunction, this, 0, NULL);
	// lpEventAttributes
	// bManualReset, FALSE: 事件有信号状态在WaitForSingleObject自动重置为无信号状态，然后再手动设置为有信号状态
	// bInitialState, TRUE: 初始状态为有信号状态
	// lpName
	h_stream_event_ = ::CreateEvent(NULL, FALSE, TRUE, NULL);
}

PublishConnection::~PublishConnection(void)
{
	if (h_stream_thread_)
	{
		CloseHandle(h_stream_thread_);
		h_stream_thread_ = NULL;
	}
	::ResetEvent(h_stream_event_);
	if (h_stream_event_)
	{
		CloseHandle(h_stream_event_);
		h_stream_event_ = NULL;
	}
}

void PublishConnection::SendDataThreadFunction(LPVOID lpParameter)
{
	PublishConnection* publish_connection = (PublishConnection*)lpParameter;
	publish_connection->SendDataLooper();
}

void PublishConnection::SendDataLooper()
{
	while(true)
	{
		void* packet = NULL;
		if(WaitForSingleObject(h_stream_event_, INFINITE) == WAIT_OBJECT_0)
		{
		}
		if(!packet_queue_.empty())
		{
			packet = (unsigned char*)packet_queue_.front();
			packet_queue_.pop();
		}
		::SetEvent(h_stream_event_);
		if (packet)
		{
			Send(packet);
		}
		Sleep(10);
	}
}

void PublishConnection::SetPacketizer(Packetizer* packetizer)
{
	packetizer_ = packetizer;
}

void PublishConnection::AddEncodedVideoHeader(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len)
{
	if (packetizer_)
	{
		packetizer_->AddEncodedVideoHeader(pps, sps, pps_len, sps_len);
	}
}

void PublishConnection::AddEncodedVideoBody(unsigned char *video_body, int len, int is_key_frame)
{
	if (packetizer_)
	{
		packetizer_->AddEncodedVideoBody(video_body, len, is_key_frame);
	}
}

void PublishConnection::OnPacketVideoData(void* packet)
{
	WaitForSingleObject(h_stream_event_, INFINITE);
	packet_queue_.push(packet);
	::SetEvent(h_stream_event_);
}

// class RtmpPublishConnection
RtmpPublishConnection::RtmpPublishConnection(void)
{
	rtmp_ = NULL;
}

RtmpPublishConnection::~RtmpPublishConnection(void)
{
	if (rtmp_)
	{
		RTMP_Close(rtmp_);
		RTMP_Free(rtmp_);
		rtmp_ = NULL;
	}
}

void RtmpPublishConnection::Init(const char* url)
{
#ifdef WIN32
	WSADATA wsaData;
	int nRet;
	if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		return;
	}
#endif

	rtmp_ = RTMP_Alloc();
	//    初始化
	RTMP_Init(rtmp_);
	rtmp_->Link.lFlags |= RTMP_LF_LIVE;
	rtmp_->Link.timeout = 10;
	rtmp_url_ = url;
	BOOL result = RTMP_SetupURL(rtmp_, (char*)rtmp_url_.c_str());
	RTMP_EnableWrite(rtmp_);
	if (!RTMP_Connect(rtmp_, NULL)) {
		goto end;
	}
	//    链接流
	RTMP_ConnectStream(rtmp_, 0);

	return; // success

end:
	RTMP_Close(rtmp_);
	RTMP_Free(rtmp_);
	rtmp_ = NULL;
}

void RtmpPublishConnection::Send(void* packet)
{
	RTMPPacket* rtmp_packet = (RTMPPacket*)packet;
	if (rtmp_packet)
	{
		if (rtmp_)
		{
			rtmp_packet->m_nInfoField2 = rtmp_->m_stream_id;
			int i = RTMP_SendPacket(rtmp_, rtmp_packet, 1);
		}
		RTMPPacket_Free(rtmp_packet);
		free(packet);
	}
}

// class RtspPublishConnection
RtspPublishConnection::RtspPublishConnection(void)
{
}

RtspPublishConnection::~RtspPublishConnection(void)
{
}

void RtspPublishConnection::Init(const char* url)
{

}

void RtspPublishConnection::Send(void* packet)
{

}