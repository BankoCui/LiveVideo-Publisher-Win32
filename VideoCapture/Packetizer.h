#pragma once

class PublishConnection;

class Packetizer
{
public:
	Packetizer(PublishConnection* connection);
	~Packetizer(void);
	virtual void AddEncodedVideoHeader(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len) = 0;
	virtual void AddEncodedVideoBody(unsigned char *video_body, int len, int is_key_frame) = 0;

protected:
	PublishConnection* connection_;
	long start_time;
};

class RtmpPacketizer :
	public Packetizer
{
public:
	RtmpPacketizer(PublishConnection* connection);
	~RtmpPacketizer(void);
	virtual void AddEncodedVideoHeader(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len);
	virtual void AddEncodedVideoBody(unsigned char *video_body, int len, int is_key_frame);
};

class RtspPacketizer :
	public Packetizer
{
public:
	RtspPacketizer(PublishConnection* connection);
	~RtspPacketizer(void);
	virtual void AddEncodedVideoHeader(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len);
	virtual void AddEncodedVideoBody(unsigned char *video_body, int len, int is_key_frame);
};