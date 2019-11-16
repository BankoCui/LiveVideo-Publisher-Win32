
#define TEXT_WORD_SIZE          (1*2*16)

void systemTimeTable(char* strTime, int time_len, unsigned char* text_table);

/*
 * yuv420p添加水印OSD：
 * 处理方式：使用16*16的点阵来绘制文字
 * 1) 2 个字节 1组（16bit），用以表示1行（16列）
 * 2）32个字节16组，用以表示16行
 * 3) 每个文字32个字节，以数组形式表示，存储地址按下标递增
 * 4）用short表示2个字节（1行，16bit）的数据内容，第一个字节存储在低位，第二个字节存储在高位
 */

char *draw_Font_Func(char *ptr_frame, int width, int height,
	char* strTime,
	int startx, int starty,
	int color);