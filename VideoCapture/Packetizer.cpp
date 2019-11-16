#include "StdAfx.h"
#include "Packetizer.h"
#include "PublishConnection.h"

extern "C"
{
#include "librtmp/rtmp.h"
}

// class Packing
Packetizer::Packetizer(PublishConnection* connection)
{
	connection_ = connection;
	start_time = RTMP_GetTime();
}

Packetizer::~Packetizer(void)
{
}

// class RtmpPacking
RtmpPacketizer::RtmpPacketizer(PublishConnection* connection)
	: Packetizer(connection)
{

}

RtmpPacketizer::~RtmpPacketizer(void)
{

}

void RtmpPacketizer::AddEncodedVideoHeader(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len)
{
	int body_size = 13 + sps_len + 3 + pps_len;
	RTMPPacket *packet = (RTMPPacket *) malloc(sizeof(RTMPPacket));
	RTMPPacket_Alloc(packet, body_size);

	RTMPPacket_Reset(packet);
	char *body = packet->m_body;
	int i = 0;
	body[i++] = 0x17;
	body[i++] = 0x00;
	//composition time 0x000000
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	/*AVCDecoderConfigurationRecord*/
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xFF;

	/*sps*/
	body[i++] = 0xE1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i], sps, sps_len);
	i += sps_len;

	/*pps*/
	body[i++] = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len) & 0xff;
	memcpy(&body[i], pps, pps_len);
	i += pps_len;

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = body_size;
	packet->m_nChannel = 0x04;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;

	// put packet
	if (connection_)
	{
		connection_->OnPacketVideoData(packet);
	}
}
void RtmpPacketizer::AddEncodedVideoBody(unsigned char *video_body, int len, int is_key_frame)
{
	/*È¥µôÖ¡½ç¶¨·û *00 00 00 01*/
	if (video_body[2] == 0x00) { //
		video_body += 4;
		len -= 4;
	} else if (video_body[2] == 0x01) { //00 00 01
		video_body += 3;
		len -= 3;
	}
	int body_size = len + 9;
	RTMPPacket *packet = (RTMPPacket *) malloc(sizeof(RTMPPacket));
	RTMPPacket_Alloc(packet, len + 9);
	char *body = packet->m_body;
	/*key frame*/
	body[0] = 0x27;
	if (is_key_frame) {
		body[0] = 0x17;
	}
	body[1] = 0x01; /*nal unit*/
	body[2] = 0x00;
	body[3] = 0x00;
	body[4] = 0x00;

	body[5] = (len >> 24) & 0xff;
	body[6] = (len >> 16) & 0xff;
	body[7] = (len >> 8) & 0xff;
	body[8] = (len) & 0xff;

	/*copy data*/
	memcpy(&body[9], video_body, len);

	packet->m_hasAbsTimestamp = 0;
	packet->m_nBodySize = body_size;
	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nChannel = 0x04;
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	//	packet->m_nTimeStamp = -1;
	packet->m_nTimeStamp = RTMP_GetTime() - start_time;
	
	// put packet
	if (connection_)
	{
		connection_->OnPacketVideoData(packet);
	}
}

// class RtspPacketizer
RtspPacketizer::RtspPacketizer(PublishConnection* connection)
	: Packetizer(connection)
{

}

RtspPacketizer::~RtspPacketizer(void)
{

}

void RtspPacketizer::AddEncodedVideoHeader(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len)
{

}
void RtspPacketizer::AddEncodedVideoBody(unsigned char *video_body, int len, int is_key_frame)
{

}