#pragma once

#include <stdio.h>
#include "stdint.h"
#include "inttypes.h"

extern "C" {
#include "x264_config.h"
#include "x264.h"
}

#pragma comment(lib, "libgcc.a")
#pragma comment(lib, "libmingwex.a")
#pragma comment(lib, "libx264.a")
#pragma comment(lib, "libx264.dll.a")

class H264encoder
{
public:
	enum ColorSpaceType {
		PIXEL_TYPE_INVALID = -1,
		PIXEL_TYPE_YUV420P
	};

	H264encoder(void);
	~H264encoder(void);
	int create(ColorSpaceType color_space_type, int width, int height, int bitrate, int fps);
	void destroy();
	int encode(char* frame_data, x264_nal_t** x264_nal, int* pkt_size);

private:
	int color_space_type_convert(ColorSpaceType p1_color_space_type);

private:
	x264_t* x264_handle_;
	x264_param_t* x264_param_;
	x264_picture_t x264_pic_in_, x264_pic_out_;
	x264_nal_t* x264_nal_;
	int color_space_type_;
};

