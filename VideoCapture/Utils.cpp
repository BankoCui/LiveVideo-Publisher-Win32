
#include "stdafx.h"
#include "Utils.h"
#include <time.h>

//Change endian of a picture   
void change_endian_picture_rgb(unsigned char *image,int w,int h,int bpp)
{
    unsigned char *pixeldata=NULL;  
    for(int i =0;i<h;i++)  
        for(int j=0;j<w;j++)
		{  
            pixeldata=image+(i*w+j)*bpp/8;  
            if(bpp == 32)
			{
                CHANGE_ENDIAN_32(pixeldata);  
            } else if(bpp == 24)
			{  
                CHANGE_ENDIAN_24(pixeldata);  
            } else if (bpp == 16)
			{
				CHANGE_ENDIAN_16_5551(pixeldata);
			}
        }  
}

//change endian of a pixel (32bit)   
void CHANGE_ENDIAN_32(unsigned char *data)
{  
    char temp3,temp2;  
    temp3 = data[3];  
    temp2 = data[2];  
    data[3] = data[0];  
    data[2] = data[1];  
    data[0] = temp3;  
    data[1] = temp2;  
} 

//change endian of a pixel (24bit)   
void CHANGE_ENDIAN_24(unsigned char *data)
{
    char temp2 = data[2];
    data[2] = data[0];
    data[0] = temp2;
}

//change endian of a pixel (16bit)   
void CHANGE_ENDIAN_16_5551(unsigned char *data)
{
	short* rgb_16 = (short*)data;

	short R = (rgb_16[0] & 0X7C00) >> 10;
	short B = rgb_16[0] & 0x001F;

	rgb_16[0] = rgb_16[0] & (0XFFFF - 0X7C1F);
	
	rgb_16[0] = rgb_16[0] | (B << 10);
	rgb_16[0] = rgb_16[0] | R;
}

//change endian of a pixel (16bit)   
void CHANGE_ENDIAN_16_565(unsigned char *data)
{
	short* rgb_16 = (short*)data;

	short R = (rgb_16[0] & 0xF800) >> 11;
	short B = rgb_16[0] & 0x001F;

	rgb_16[0] = rgb_16[0] & (0XFFFF - 0XF81F);
	
	rgb_16[0] = rgb_16[0] | (B << 11);
	rgb_16[0] = rgb_16[0] | R;
}

void YUY2_to_RGB(BYTE *YUY2buff,BYTE *RGBbuff,int dwSize)
{
	//B = 1.164(Y - 16)         + 2.018(U - 128)
    //G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
    //R = 1.164(Y - 16) + 1.596(V - 128)
    BYTE *orgRGBbuff = RGBbuff;
    for(int count=0;count<dwSize;count+=4)  
    {  
        //Y0 U0 Y1 V0  
        float Y0 = *YUY2buff;  
        float U = *(++YUY2buff);  
        float Y1 = *(++YUY2buff);  
        float V = *(++YUY2buff);  
        ++YUY2buff;
        *(RGBbuff) =   (BYTE)(Y0 + (1.370705 * (V-128)));
        *(++RGBbuff) = (BYTE)(Y0 - (0.698001 * (V-128)) - (0.337633 * (U-128)));
        *(++RGBbuff) = (BYTE)(Y0 + (1.732446 * (U-128)));
        *(++RGBbuff) = (BYTE)(Y1 + (1.370705 * (V-128)));
        *(++RGBbuff) = (BYTE)(Y1 - (0.698001 * (V-128)) - (0.337633 * (U-128)));
        *(++RGBbuff) = (BYTE)(Y1 + (1.732446 * (U-128)));
        ++RGBbuff;
    }
}

static unsigned char clip255(long v)
{
	if(v < 0)
		v=0;
	else if( v > 255) 
		v=255;
	return (unsigned char)v;
} 
BOOL YUY2_to_RGB_2(unsigned char *YUY2buff,unsigned char  *RGBbuff,long dwSize)
{
	unsigned char *orgRGBbuff = RGBbuff;
	for( long count = 0; count < dwSize; count += 4 )
	{//Y0 U0 Y1 V0
		unsigned char Y0 = *YUY2buff;
		unsigned char U = *(++YUY2buff);
		unsigned char Y1 = *(++YUY2buff);
		unsigned char V = *(++YUY2buff);
		++YUY2buff;
		long Y,C,D,E;
		unsigned char R,G,B;
		Y = Y0;
		C = Y - 16;
		D = U - 128;
		E = V - 128;
		R = clip255(( 298 * C           + 409 * E + 128) >> 8);
		G = clip255(( 298 * C - 100 * D - 208 * E + 128)>> 8);
		B = clip255(( 298 * C + 516 * D           + 128) >> 8);
		*(RGBbuff)   = B;
		*(++RGBbuff) = G;
		*(++RGBbuff) = R;
		Y = Y1;
		C = Y-16;
		D = U-128;
		E = V-128;
		R = clip255(( 298 * C           + 409 * E + 128) >> 8);
		G = clip255(( 298 * C - 100 * D - 208 * E + 128)>> 8);
		B = clip255(( 298 * C + 516 * D           + 128) >> 8);
		*(++RGBbuff) = B;
		*(++RGBbuff) = G;
		*(++RGBbuff) = R;
		++RGBbuff;
	}
	return true;
}

/*
 * 每种采样格式的数据排列形式，包括打包模式和平面模式。基于此，才能在它们之间做格式转换
 * 从yuy2转为yuv420p（YUV422打包格式 转为 YUV420平面模式），要丢1/2的U和1/2的V
 * 丢弃：跨行丢弃
 *
 * YUY2:    y0  u0  y1  v0  y2  u1  y3  v1
 *          y4  u2  y5  v2  y6  u3  y7  v3
 *          y8  u4  y9  v4  y10 u5  y11 v5
 *          y12 u6  y13 v6  y14 u7  y15 v7
 *
 * YUV420P: y0  y1  y2  y3
 *          y4  y5  y6  y7
 *          y8  y9  y10 y11
 *          y12 y13 y14 y15
 *          u0  u1
 *          u4  u5
 *          v0  v1
 *          u4  u5
 *
 * YUY2：   y_size=W*H，u_size=1/2*W*H，v_size=1/2*W*H
 * YUV420P：y_size=W*H，u_size=1/4*W*H，v_size=1/4*W*H
 *
 * YUY2：   每行2*W个字节。
 *          1个像素对应1个Y，一个字节。2个像素对应一组UV，一组UV是2个字节。
 *          2个Y共用一组UV。2个像素共4个字节。
 */
void YUY2_YUV420P(BYTE *YUY2buff,BYTE *YUV420Pbuff, int w, int h)
{
	unsigned char * YUV420P_y = YUV420Pbuff;
	unsigned char * YUV420P_u = YUV420Pbuff + w*h;
	unsigned char * YUV420P_v = YUV420P_u + w*h/4;

	int y_size = w*h;
	for (int i = 0; i < y_size; i++) 
	{
		YUV420P_y[i] = YUY2buff[2*i];
	}

	for (int i = 0, k = 0; i < h; i++)
	{
		// deal one row, every two rows
		if (i%2 == 0)
		{
			// 2个像素4个字节，包含1个U
			// 反过来：1个U，对应2个像素，4个字节
			for (int j = 0; j < w/2; j++)
			{
				// 每行2*W个字节
				YUV420P_u[k] = YUY2buff[i*2*w + 4*j+1];
				k++;
			}
		}
	}

	for (int i = 0, k = 0; i < h; i++)
	{
		// deal one row, every two rows
		if (i%2 == 0)
		{
			// 2个像素4个字节，包含1个V
			// 反过来：1个V，对应2个像素，4个字节
			for (int j = 0; j < w/2; j++)
			{
				// 每行2*W个字节
				YUV420P_v[k] = YUY2buff[i*2*w + 4*j+3];
				k++;
			}
		}
	}
}

void GetCurrentSysTime(char strTime[]) {
	struct tm* curtm;
	time_t t = time(0);
	curtm = localtime(&t);
	int year = curtm->tm_year + 1900;
	int mont = curtm->tm_mon + 1;
	int mday = curtm->tm_mday;
	int hour = curtm->tm_hour;
	int minu = curtm->tm_min;
	int seco = curtm->tm_sec;
	char* format = "%04d-%02d-%02d %02d:%02d:%02d";
	sprintf(strTime, format, year, mont, mday, hour, minu, seco);
}