#ifndef __DECODED_H
#define __DECODED_H

#include "sys.h"

u16 pic_get_tnum(u8 *path);	//��ȡͼƬ�ļ�����
u16 Get_DecodeRGB(u8 *RGB_buf,u16 x, u16 y, u16 num);	//��ָ���㿪ʼ����ȡnum�����ص���ɫֵ
u8 Update_DecodeRGB(u8 *src);	//���½����ͼƬ
u8 Show_Decoded_Picture(u8 mode,u8 *src);	//��ʾͼƬ
u8 Picture_Fill(u16 x,u16 y,u16 width,u16 height,u8 *src);	//��һ����������ˢ��ͼƬ
void Load_Windows(u16 x,u8 update);	//��������

#endif

