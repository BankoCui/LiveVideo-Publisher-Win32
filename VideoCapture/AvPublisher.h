#pragma once

class AvEncoder;
class Packetizer;
class PublishConnection;

class AvPublisher
{
public:
	enum PublisherColorSpaceType {
		PUBLISHER_COLOR_SPACE_TYPE_INVALID = -1,
		PUBLISHER_COLOR_SPACE_TYPE_YUV420P = 0
	};

	enum ConnectionType {
		CONN_TYPE_RTMP = 0,
		CONN_TYPE_RTSP = 1
	};

	AvPublisher(void);
	~AvPublisher(void);
	void SetVideoParams(PublisherColorSpaceType color_capce_type, int width, int height, int bitrate, int fps);
	void StartRtmp(char* url);
	void StartRtsp(char* url);
	void PublishVideoData(const unsigned char* frame_data, int len);
	void PublishAudioData(const unsigned char* frame_data, int len);
	void OnEncodedVideoHeader(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len);
	void OnEncodedVideoBody(unsigned char *video_body, int len, int is_key_frame);
	void OnEncodedAudioData();

private:
	int color_space_type_convert(PublisherColorSpaceType color_space_type);
	PublishConnection* CreateConnection(ConnectionType conn_type);
	Packetizer* CreatePacketizer(ConnectionType conn_type, PublishConnection* conn);
	AvEncoder* av_encoder_;
	Packetizer* packetizer_;
	PublishConnection* publish_connection_;
};

