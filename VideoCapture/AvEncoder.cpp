#include "StdAfx.h"
#include "AvEncoder.h"
#include "H264encoder.h"
#include "AvPublisher.h"

AvEncoder::AvEncoder(AvPublisher* av_publisher)
	: av_publisher_(av_publisher)
{
	h_encode_thread_ = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EncodeThreadFunction, this, 0, NULL);
	// lpEventAttributes
	// bManualReset, FALSE: 事件有信号状态在WaitForSingleObject自动重置为无信号状态，然后再手动设置为有信号状态
	// bInitialState, TRUE: 初始状态为有信号状态
	// lpName
	h_encode_event_ = ::CreateEvent(NULL, FALSE, TRUE, NULL);

	h264_encoder_ = new H264encoder();
}

AvEncoder::~AvEncoder(void)
{
	if (h264_encoder_)
	{
		delete h264_encoder_;
		h264_encoder_ = NULL;
	}

	if (h_encode_thread_)
	{
		CloseHandle(h_encode_thread_);
		h_encode_thread_ = NULL;
	}
	::ResetEvent(h_encode_event_);
	if (h_encode_event_)
	{
		CloseHandle(h_encode_event_);
		h_encode_event_ = NULL;
	}
}

void AvEncoder::EncodeThreadFunction(LPVOID lpParameter)
{
	AvEncoder* av_encoder = (AvEncoder*) lpParameter;
	av_encoder->EncodeFrameLooper();
}

void AvEncoder::EncodeFrameLooper()
{
	while(true)
	{
		BinaryMsgData* msg_data = NULL;
		if(WaitForSingleObject(h_encode_event_, INFINITE) == WAIT_OBJECT_0)
		{
		}
		if(!frame_queue_.empty())
		{
			msg_data = (BinaryMsgData*)frame_queue_.front();
			frame_queue_.pop();
		}
		::SetEvent(h_encode_event_);
		if (msg_data)
		{
			DoEncodeVideoFrame(msg_data->data());
			delete msg_data;
			msg_data = NULL;

		}
		Sleep(10);
	}
}

void AvEncoder::SetParams(EncoderColorSpaceType color_capce_type, int width, int height, int bitrate, int fps)
{
	if (h264_encoder_)
	{
		H264encoder::ColorSpaceType color_space_t = (H264encoder::ColorSpaceType)color_space_type_convert(color_capce_type);
		h264_encoder_->create(color_space_t, width, height, bitrate, fps);
	}
}

void AvEncoder::EncodeVideoFrame(const unsigned char* frame_data, int len)
{
	WaitForSingleObject(h_encode_event_, INFINITE);
	frame_queue_.push(new BinaryMsgData(frame_data, len));
	::SetEvent(h_encode_event_);
}

void AvEncoder::EncodeAudioFrame(const unsigned char* frame_data, int len)
{

}

void AvEncoder::DoEncodeVideoFrame(const unsigned char* frame_data)
{
	if (h264_encoder_)
	{
		x264_nal_t* x264_nal = NULL;
		int pic_nal_count = -1;

		int ret = h264_encoder_->encode((char*)frame_data, &x264_nal, &pic_nal_count);
		if (ret < 0)
		{
			printf("failed to call h264encoder.encode(); ret=%d\n", ret);
			return;
		}
		if (av_publisher_)
		{
			if (pic_nal_count > 0 && x264_nal != NULL)
			{
				unsigned char sps[100];
				unsigned char pps[100];
				int sps_len;
				int pps_len;

				for (int i = 0; i < pic_nal_count; ++i)
				{
					if (x264_nal[i].i_type == NAL_SPS) {
						sps_len = x264_nal[i].i_payload - 4;
						memcpy(sps, x264_nal[i].p_payload + 4, sps_len);
					} else if (x264_nal[i].i_type == NAL_PPS) {
						pps_len = x264_nal[i].i_payload - 4;
						memcpy(pps, x264_nal[i].p_payload + 4, pps_len);
						av_publisher_->OnEncodedVideoHeader(pps, sps, pps_len, sps_len);
						//发送整个头信息
					} else {
						//关键帧与非关键帧
						//unsigned char* h264_buf = x264_nal[i].p_payload;
						//int nal_type = (h264_buf[2] == 0x00) ? h264_buf[4]&0x1f : (h264_buf[2] == 0x01) ? h264_buf[3]&0x1f : -1;
						int is_key_frame = (x264_nal[i].i_type == NAL_SLICE_IDR);
						av_publisher_->OnEncodedVideoBody(x264_nal[i].p_payload, x264_nal[i].i_payload, is_key_frame);
					}
				}
			}
		}
	}
}

void AvEncoder::DoEncodeAudioFrame(const unsigned char* frame_data)
{

}

int AvEncoder::color_space_type_convert(EncoderColorSpaceType color_space_type)
{
	switch(color_space_type) {
	case ENCODER_COLOR_SPACE_TYPE_YUV420P:
		return H264encoder::PIXEL_TYPE_YUV420P;
	default:
		return H264encoder::PIXEL_TYPE_INVALID;
	}
}