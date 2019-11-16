#include "StdAfx.h"
#include "AvPublisher.h"
#include "AvEncoder.h"
#include "Packetizer.h"
#include "PublishConnection.h"

AvPublisher::AvPublisher(void)
{
	av_encoder_ = NULL;
	packetizer_ = NULL;
	publish_connection_ = NULL;
}

AvPublisher::~AvPublisher(void)
{
	if (av_encoder_)
	{
		delete av_encoder_;
		av_encoder_ = NULL;
	}
	if (packetizer_)
	{
		delete packetizer_;
		packetizer_ = NULL;
	}
	if (publish_connection_)
	{
		delete publish_connection_;
		publish_connection_ = NULL;
	}
}

void AvPublisher::SetVideoParams(PublisherColorSpaceType color_capce_type, int width, int height, int bitrate, int fps)
{
	AvEncoder::EncoderColorSpaceType color_space_t = (AvEncoder::EncoderColorSpaceType)color_space_type_convert(color_capce_type);
	av_encoder_ = new AvEncoder(this);
	av_encoder_->SetParams(color_space_t, width, height, bitrate, fps);
}

int AvPublisher::color_space_type_convert(PublisherColorSpaceType color_space_type)
{
	switch(color_space_type) {
	case PUBLISHER_COLOR_SPACE_TYPE_YUV420P:
		return AvEncoder::ENCODER_COLOR_SPACE_TYPE_YUV420P;
	default:
		return AvEncoder::ENCODER_COLOR_SPACE_TYPE_INVALID;
	}
}

PublishConnection* AvPublisher::CreateConnection(ConnectionType conn_type)
{
	switch(conn_type) {
	case CONN_TYPE_RTMP:
		return new RtmpPublishConnection();
	case CONN_TYPE_RTSP:
		return new RtspPublishConnection();
	default:
		return NULL;
	}
}

Packetizer* AvPublisher::CreatePacketizer(ConnectionType conn_type, PublishConnection* conn)
{
	switch(conn_type) {
	case CONN_TYPE_RTMP:
		return new RtmpPacketizer(conn);
		break;
	case CONN_TYPE_RTSP:
		return new RtspPacketizer(conn);
	default:
		return NULL;
	}
}

void AvPublisher::StartRtmp(char* url)
{
	ConnectionType conn_type = CONN_TYPE_RTMP;
	publish_connection_ = CreateConnection(conn_type);
	packetizer_ = CreatePacketizer(conn_type, publish_connection_);
	publish_connection_->SetPacketizer(packetizer_);

	publish_connection_->Init(url);
}

void AvPublisher::StartRtsp(char* url)
{
	ConnectionType conn_type = CONN_TYPE_RTSP;
	publish_connection_ = CreateConnection(conn_type);
	packetizer_ = CreatePacketizer(conn_type, publish_connection_);
	publish_connection_->SetPacketizer(packetizer_);

	publish_connection_->Init(url);
}

void AvPublisher::PublishVideoData(const unsigned char* frame_data, int len)
{
	if (av_encoder_)
	{
		av_encoder_->EncodeVideoFrame(frame_data, len);
	}
}

void AvPublisher::PublishAudioData(const unsigned char* frame_data, int len)
{

}

void AvPublisher::OnEncodedVideoHeader(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len)
{
	if (publish_connection_)
	{
		publish_connection_->AddEncodedVideoHeader(pps, sps, pps_len, sps_len);
	}
}

void AvPublisher::OnEncodedVideoBody(unsigned char *video_body, int len, int is_key_frame)
{
	if (publish_connection_)
	{
		publish_connection_->AddEncodedVideoBody(video_body, len, is_key_frame);
	}
}

void AvPublisher::OnEncodedAudioData()
{

}