
//Change endian of a picture   
void change_endian_picture_rgb(unsigned char *image,int w,int h,int bpp);

//change endian of a pixel (32bit)   
void CHANGE_ENDIAN_32(unsigned char *data);

//change endian of a pixel (24bit)   
void CHANGE_ENDIAN_24(unsigned char *data);

//change endian of a pixel (16bit) 
void CHANGE_ENDIAN_16_5551(unsigned char *data);

//change endian of a pixel (16bit) 
void CHANGE_ENDIAN_16_565(unsigned char *data);

void YUY2_to_RGB(BYTE *YUY2buff,BYTE *RGBbuff,int dwSize);

BOOL YUY2_to_RGB_2(unsigned char *YUY2buff,unsigned char  *RGBbuff,long dwSize);

void YUY2_YUV420P(BYTE *YUY2buff,BYTE *RGBbuff, int w, int h);

void GetCurrentSysTime(char strTime[]);