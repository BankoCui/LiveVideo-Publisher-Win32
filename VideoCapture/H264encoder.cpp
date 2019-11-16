#include "H264encoder.h"


H264encoder::H264encoder(void)
{
}


H264encoder::~H264encoder(void)
{
	destroy();
}

int H264encoder::create(ColorSpaceType color_space_type, int width, int height, int bitrate, int fps)
{
	int ret = -1;
	color_space_type_ = color_space_type_convert(color_space_type);

	x264_param_ = new x264_param_t;
	//* 配置为默认参数
	x264_param_default_preset(x264_param_, x264_preset_names[0], x264_tune_names[7]);
	x264_param_->i_level_idc = 51;
	//* video Properties
	x264_param_->i_csp = color_space_type_;
	x264_param_->i_width = width; //* width
	x264_param_->i_height = height; //* height
	x264_param_->i_threads = 1;
	
	//* muxing parameters
	x264_param_->i_fps_den = 1; //* 帧率分母
	x264_param_->i_fps_num = fps;//* 帧率分子
	x264_param_->i_timebase_den = x264_param_->i_fps_num;
	x264_param_->i_timebase_num = x264_param_->i_fps_den;

	//码率相关设置 关键帧间隔时间的帧率
	x264_param_->i_keyint_max = fps * 2;

	/**
     * CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
     */
	x264_param_->rc.i_rc_method = X264_RC_ABR;
	x264_param_->rc.i_bitrate = bitrate / 1000;
	x264_param_->rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;
	x264_param_->rc.i_vbv_buffer_size = bitrate / 1000;

	//* Log参数
	x264_param_->i_log_level = X264_LOG_DEBUG;
	//* Rate control Parameters

	//设置输入  0 pts 来做音视频同步
	//x264_param_->b_vfr_input = 0;

	//* 设置编码等级profile : baseline、main、height ( 直播baseline:流畅 、Main:有停顿现象 、本地FLV文件:main)
	x264_param_apply_profile(x264_param_, x264_profile_names[0]);

	x264_handle_ = x264_encoder_open(x264_param_);

	x264_picture_init(&x264_pic_out_);
	x264_picture_alloc(&x264_pic_in_, color_space_type_, width, height);

	return ret = 1;
}

void H264encoder::destroy()
{
	x264_picture_clean(&x264_pic_in_);
	if (x264_handle_)
	{
		x264_encoder_close(x264_handle_);
		x264_handle_ = NULL;
	}

	if (x264_param_)
	{
		delete x264_param_;
		x264_param_ = NULL;
	}
}

int H264encoder::encode(char* frame_data, x264_nal_t** x264_nal, int* pic_nal_count)
{
	int y_data_size = x264_param_->i_width*x264_param_->i_height;
	int u_data_size = y_data_size >> 2;
	int v_data_size = y_data_size >> 2;

	if (color_space_type_ == X264_CSP_I420)
	{
		memcpy(x264_pic_in_.img.plane[0], (uint8_t*)frame_data, y_data_size);
		memcpy(x264_pic_in_.img.plane[1], (uint8_t*)frame_data + y_data_size, u_data_size);
		memcpy(x264_pic_in_.img.plane[2], (uint8_t*)frame_data + y_data_size + u_data_size , v_data_size);
	}

	int local_pic_nal_count = -1;
	x264_pic_in_.i_pts++; // 一定要有，否则会报：x264 [warning]: non-strictly-monotonic PTS，和x264_param_->b_vfr_input = 0;有一个即可
	int ret_frame = x264_encoder_encode(x264_handle_, &x264_nal_, &local_pic_nal_count, &x264_pic_in_, &x264_pic_out_);
	printf("ret_frame:%d, local_pic_nal_count:%d\n", ret_frame, local_pic_nal_count);
	// 因为要进行编码，所以编码器需要缓存很多帧才好生成I、P、B帧
	// local_pic_nal_count == 0 或者 local_pic_nal_count > 0
	*x264_nal = x264_nal_;
	*pic_nal_count = local_pic_nal_count;
	return 0;
}

int H264encoder::color_space_type_convert(ColorSpaceType p1_color_space_type)
{
	int color_space_type = X264_CSP_NONE;
	switch(p1_color_space_type)
	{
	case PIXEL_TYPE_YUV420P:
		color_space_type = X264_CSP_I420;
		break;
	default:
		break;
	}
	return color_space_type;
}