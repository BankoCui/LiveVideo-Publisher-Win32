
#define TEXT_WORD_SIZE          (1*2*16)

void systemTimeTable(char* strTime, int time_len, unsigned char* text_table);

/*
 * yuv420p���ˮӡOSD��
 * ����ʽ��ʹ��16*16�ĵ�������������
 * 1) 2 ���ֽ� 1�飨16bit�������Ա�ʾ1�У�16�У�
 * 2��32���ֽ�16�飬���Ա�ʾ16��
 * 3) ÿ������32���ֽڣ���������ʽ��ʾ���洢��ַ���±����
 * 4����short��ʾ2���ֽڣ�1�У�16bit�����������ݣ���һ���ֽڴ洢�ڵ�λ���ڶ����ֽڴ洢�ڸ�λ
 */

char *draw_Font_Func(char *ptr_frame, int width, int height,
	char* strTime,
	int startx, int starty,
	int color);