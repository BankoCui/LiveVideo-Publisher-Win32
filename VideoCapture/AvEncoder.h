#pragma once
#include <queue>

class AvPublisher;
class H264encoder;

class BinaryMsgData
{
public:
	BinaryMsgData(const unsigned char* data, int len)
		: len_(len)
	{
		data_ = new unsigned char[len];
		memcpy(data_, data, len);
	}
	~BinaryMsgData()
	{
		delete[] data_;
		data_ = NULL;
	}
	unsigned char* data() {return data_;}
	unsigned int len() {return len_;}

private:
	unsigned char* data_;
	int len_;
};

class AvEncoder
{
public:
	enum EncoderColorSpaceType {
		ENCODER_COLOR_SPACE_TYPE_INVALID = -1,
		ENCODER_COLOR_SPACE_TYPE_YUV420P = 0
	};

	AvEncoder(AvPublisher* av_publisher);
	~AvEncoder(void);
	void SetParams(EncoderColorSpaceType color_capce_type, int width, int height, int bitrate, int fps);
	void EncodeVideoFrame(const unsigned char* frame_data, int len);
	void EncodeAudioFrame(const unsigned char* frame_data, int len);

private:
	static void EncodeThreadFunction(LPVOID lpParameter);
	void EncodeFrameLooper();
	void DoEncodeVideoFrame(const unsigned char* frame_data);
	void DoEncodeAudioFrame(const unsigned char* frame_data);
	int color_space_type_convert(EncoderColorSpaceType color_space_type);
	std::queue<BinaryMsgData*> frame_queue_;
	HANDLE h_encode_event_;
	HANDLE h_encode_thread_;

	AvPublisher* av_publisher_;
	H264encoder* h264_encoder_;
};

