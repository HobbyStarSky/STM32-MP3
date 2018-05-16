#ifndef __DECODED_H
#define __DECODED_H

#include "sys.h"

u16 pic_get_tnum(u8 *path);	//获取图片文件个数
u16 Get_DecodeRGB(u8 *RGB_buf,u16 x, u16 y, u16 num);	//从指定点开始，获取num个像素点颜色值
u8 Update_DecodeRGB(u8 *src);	//更新解码的图片
u8 Show_Decoded_Picture(u8 mode,u8 *src);	//显示图片
u8 Picture_Fill(u16 x,u16 y,u16 width,u16 height,u8 *src);	//在一定矩形区域刷新图片
void Load_Windows(u16 x,u8 update);	//加载桌面

#endif

