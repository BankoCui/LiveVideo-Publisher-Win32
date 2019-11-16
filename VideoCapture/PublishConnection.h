#pragma once
#include <queue>
#include <string>

class Packetizer;
struct RTMP;

class PublishConnection
{
public:
	PublishConnection(void);
	~PublishConnection(void);
	void SetPacketizer(Packetizer* packetizer);
	virtual void Init(const char* url) = 0;
	void AddEncodedVideoHeader(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len);
	void AddEncodedVideoBody(unsigned char *video_body, int len, int is_key_frame);
	// callback from Packetizer
	void OnPacketVideoData(void* packet);

protected:
	virtual void Send(void* packet) = 0;

private:
	static void SendDataThreadFunction(LPVOID lpParameter);
	void SendDataLooper();

	Packetizer* packetizer_;

	std::queue<void*> packet_queue_;
	HANDLE h_stream_event_;
	HANDLE h_stream_thread_;
};

class RtmpPublishConnection :
	public PublishConnection
{
public:
	RtmpPublishConnection(void);
	~RtmpPublishConnection(void);
	virtual void Init(const char* url);

protected:
	virtual void Send(void* packet);

private:
	RTMP* rtmp_;
	std::string rtmp_url_;
};

class RtspPublishConnection :
	public PublishConnection
{
public:
	RtspPublishConnection(void);
	~RtspPublishConnection(void);
	virtual void Init(const char* url);

protected:
	virtual void Send(void* packet);

private:
	std::string rtsp_url_;
};