#include "decoded.h"
#include "lcd.h"
#include "sdio_sdcard.h" 
#include "ff.h"  
#include "exfuns.h"   
#include "text.h"
#include "string.h"		
#include "malloc.h"
#include "delay.h"
#include "piclib.h"	
#include "key.h"
#include "led.h"


//解码后的图片存放在磁盘中的路径
u8*const PICTURE_PATH="/PICTURE/01.bin";		


//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//长文件名最大长度
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	return rval;
}

//从指定点开始，获取num个像素点颜色值
//从左到右，从上到下扫描
//返回实际读到的点个数
u16 Get_DecodeRGB(u8* RGB_buf,u16 x, u16 y, u16 num)
{
	u16 i;
	u16 buf;
	u16 x0;
	
	x0 = x;	//保存扫描的起始点
	for(i=0;i<num;i++)
	{
		if(x>=lcddev.width)	//一行扫描完，转下一行
		{
			y++;
			x = x0;
		}
		else	x++;
		if(y>=lcddev.height)	return i;	//扫描完成
		
		buf = LCD_ReadPoint(x,y);
		RGB_buf[2*i] = buf>>8;	//获取高8位
		RGB_buf[2*i+1] = buf;	//获取第8位
	}
	return i;
}

//更新解码的图片
u8 Update_DecodeRGB(u8 *src)
{
	u8 *pname;
	u8 *RGB_buf;
	u8 res = 0;		   
 	u16 i, datanum;
	FIL *fftemp;
	UINT bwrite;
	u8 rval = 0; 
	
	res = 0XFF;		
	datanum = lcddev.width*2;
	pname = mymalloc(SRAMIN,100);	//申请100字节内存  
	RGB_buf = mymalloc(SRAMIN,datanum);	//申请lcddev.width*2字节内存  
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//分配内存	
	if(RGB_buf==NULL||pname==NULL||fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,RGB_buf);
		return 5;	//内存申请失败
	}
	//先查找文件是否正常 
	strcpy((char*)pname,(char*)src);	//copy src内容到pname
	strcat((char*)pname,(char*)PICTURE_PATH); 
	res = f_open(fftemp,(const TCHAR*)pname,FA_WRITE|FA_OPEN_ALWAYS); //以写打开文件,文件不存在则创建
 	if(res) rval|=1<<4;	//打开文件失败
	
	for(i=0;i<lcddev.height;i++)
	{
		Get_DecodeRGB(RGB_buf,0,i,datanum/2);	//每次读取一行点
		res = f_write(fftemp,RGB_buf,datanum,&bwrite);	//写入到文件
		if((res!=FR_OK)||(bwrite!=datanum))	break;				//执行错误
	}
	
	f_close(fftemp);	//关闭文件
	myfree(SRAMIN,fftemp);	//释放内存
	myfree(SRAMIN,pname);	//释放内存
	myfree(SRAMIN,RGB_buf);	//释放内存
	return res;
}

//显示图片
//mode为0表示检测是否有图片文件，mode为1表示显示图片
u8 Show_Decoded_Picture(u8 mode,u8 *src)
{
	u8 *pname;
	u8 *RGB_buf;
	u8 res = 0;		   
 	u16 i, j, datanum;
	u16 *RGB_pic;
	FIL *fftemp;
	UINT bread;
	u8 rval = 0; 
	
	res = 0XFF;		
	datanum = lcddev.width*2;
	pname = mymalloc(SRAMIN,100);	//申请100字节内存  
	RGB_buf = mymalloc(SRAMIN,datanum);	//申请lcddev.width*2字节内存 
	RGB_pic	= mymalloc(SRAMIN,datanum);	//申请lcddev.width*2字节内存 
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//分配内存	
	if(RGB_buf==NULL||pname==NULL||fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,RGB_buf);
		return 5;	//内存申请失败
	}
	
	//先查找文件是否正常 
	strcpy((char*)pname,(char*)src);	//copy src内容到pname
	strcat((char*)pname,(char*)PICTURE_PATH); 
	
	res = f_open(fftemp,(const TCHAR*)pname,FA_READ); //以读打开文件
	if(mode==0)	return res;	//检测图片文件是否正常
	
 	if(res) rval|=1<<4;	//打开文件失败
	
	for(i=0;i<lcddev.height;i++)
	{
		res = f_read(fftemp,RGB_buf,datanum,&bread);	//读出文件
		if(res!=FR_OK)	break;				//执行错误
		for(j=0;j<bread/2;j++)
		{
			RGB_pic[j] = (u16)RGB_buf[2*j]<<8|(u16)RGB_buf[2*j+1];	//转换成u16
		}
		LCD_Color_Fill(0,i,j-1,i,RGB_pic);	//画一行
		
		if(bread!=datanum)	break;	//已读完
	}
	
	f_close(fftemp);	//关闭文件
	myfree(SRAMIN,fftemp);	//释放内存
	myfree(SRAMIN,pname);	//释放内存
	myfree(SRAMIN,RGB_buf);	//释放内存
	myfree(SRAMIN,RGB_pic);	//释放内存
	
	return res;
}

//在一定矩形区域刷新图片
u8 Picture_Fill(u16 x,u16 y,u16 width,u16 height,u8 *src)
{
	u8 *pname;
	u8 *RGB_buf;
	u8 res = 0;		   
 	u16 i, j, datanum;
	u16 *RGB_pic;
	FIL *fftemp;
	UINT bread;
	u8 rval = 0; 
	DWORD pointer;
	
	res = 0XFF;		
	datanum = width*2;
	pname = mymalloc(SRAMIN,100);	//申请100字节内存  
	RGB_buf = mymalloc(SRAMIN,datanum);	//申请width*2字节内存 
	RGB_pic	= mymalloc(SRAMIN,datanum);	//申请width*2字节内存 	
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//分配内存	
	if(RGB_buf==NULL||pname==NULL||fftemp==NULL||RGB_pic==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,RGB_buf);
		myfree(SRAMIN,RGB_pic);
		return 5;	//内存申请失败
	}
	
	//先查找文件是否正常 
	strcpy((char*)pname,(char*)src);	//copy src内容到pname
	strcat((char*)pname,(char*)PICTURE_PATH); 
	res = f_open(fftemp,(const TCHAR*)pname,FA_READ); //以读打开文件
 	if(res) rval|=1<<4;	//打开文件失败
	
	for(i=0;i<height;i++)
	{
		pointer = ((y+i)*lcddev.width+x)*2;
		res = f_lseek(fftemp,pointer);	//移动到指定点
		res = f_read(fftemp,RGB_buf,datanum,&bread);	//读出文件，width*2个字节
		if(res!=FR_OK)	break;				//执行错误
		for(j=0;j<width;j++)
		{
			RGB_pic[j] = (u16)RGB_buf[2*j]<<8|(u16)RGB_buf[2*j+1];	//转换成u16,RGB格式
		}
		LCD_Color_Fill(x,y+i,x+width-1,y+i,RGB_pic);	//画指定矩形区域内的一行
		
		if(bread!=datanum)	break;	//已读完
	}
	
	f_close(fftemp);	//关闭文件
	myfree(SRAMIN,fftemp);	//释放内存
	myfree(SRAMIN,pname);	//释放内存
	myfree(SRAMIN,RGB_buf);	//释放内存
	myfree(SRAMIN,RGB_pic);	//释放内存
	
	return res;
}


//加载桌面
void Load_Windows(u16 x,u8 update)	
{
	DIR picdir;	 		//图片目录
	FILINFO picfileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totpicnum;
	u16 curindex;		//图片当前索引
	u16 *picindextbl;	//图片索引表
	u8 res1, res2;
	u16 temp;
	u8 key;
	u8 t = 0;
	
	if(update==1)	//更新桌面背景
	{
		LCD_Clear(BLACK);	//清屏
		LCD_ShowString(30,30,200,16,16,"Update Picture:",1);		//显示提示信息
		LCD_ShowString(50,50,200,16,16,"Picture UP:KEY0",1);
		LCD_ShowString(50,70,200,16,16,"Picture DOWN:KEY2",1);
		LCD_ShowString(50,90,200,16,16,"Picture Update:WAKE_UP",1);
		LCD_ShowString(30,110,300,16,16,"Please take KEY1 to continue...",1);
		while(key!=KEY1_PRES)
		{
			key = KEY_Scan(0);
			delay_ms(10);
		}
		LCD_Clear(BLACK);	//清屏
		
		while(f_opendir(&picdir,"0:/PICTURE"))//打开图片文件夹
		{	    
			Show_Str(30,x,240,16,"PICTURE文件夹错误!",16,0);
			delay_ms(200);				  
			LCD_Fill(30,170,240,186,WHITE);//清除显示	     
			delay_ms(200);				  
		}  
		totpicnum = pic_get_tnum("0:/PICTURE"); //得到总有效文件数

		while(totpicnum==NULL)//图片文件为0		
		{	    
			Show_Str(30,x,240,16,"没有图片文件!",16,0);
			delay_ms(200);				  
			LCD_Fill(30,x,240,186,BLACK);//清除显示	     
			delay_ms(200);				  
		}

		picfileinfo.lfsize = _MAX_LFN*2+1;		//长文件名最大长度
		picfileinfo.lfname = mymalloc(SRAMIN,picfileinfo.lfsize);	//为长文件缓存区分配内存
		pname = mymalloc(SRAMIN,picfileinfo.lfsize);				//为带路径的文件名分配内存
		picindextbl = mymalloc(SRAMIN,2*totpicnum);				//申请2*totpicnum个字节的内存,用于存放图片索引

		while(picfileinfo.lfname==NULL||pname==NULL||picindextbl==NULL)//内存分配出错
		{	    
			Show_Str(30,x,240,16,"内存分配失败!",16,0);
			delay_ms(200);				  
			LCD_Fill(30,x,240,186,BLACK);//清除显示	     
			delay_ms(200);				  
		}  	
		//记录索引
		res1 = f_opendir(&picdir,"0:/PICTURE"); //打开目录
		if(res1==FR_OK)
		{
			curindex=0;//当前索引为0
			while(1)//全部查询一遍
			{
				temp=picdir.index;	//记录当前index
				res1=f_readdir(&picdir,&picfileinfo);       		//读取目录下的一个文件
				
				if(res1!=FR_OK||picfileinfo.fname[0]==0)	break;	//错误了/到末尾了,退出		
				
				fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
				res1=f_typetell(fn);	
				if((res1&0XF0)==0X50)//取高四位,看看是不是图片文件	
				{
					picindextbl[curindex]=temp;//记录索引
					curindex++;
				}	    
			} 
		}   

		curindex=0;											//从0开始显示
		res1=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//打开目录
		while(res1==FR_OK)//打开成功
		{	
			dir_sdi(&picdir,picindextbl[curindex]);			//改变当前目录索引	  		
			res1=f_readdir(&picdir,&picfileinfo);       		//读取目录下的一个文件
			
			if(res1!=FR_OK&&picfileinfo.fname[0]==0) break;		//错误了/到末尾了,退出
			
			fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
			strcpy((char*)pname,"0:/PICTURE/");				//复制路径(目录)
			strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
			
			Show_Str(20,20,240,16,"准备更新桌面...",16,1);
			delay_ms(1000);
			
			LCD_Clear(BLACK);	//清屏
			ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);	//显示图片
		
			while(1) 
			{
				key=KEY_Scan(0);		//扫描按键
				
				if((t%20)==0)	LED0 = !LED0; //LED0闪烁,提示程序正在运行.
				if(key==KEY2_PRES)		//上一张
				{
					if(curindex)curindex--;
					else curindex=totpicnum-1;
					break;
				}else if(key==KEY0_PRES)//下一张
				{
					curindex++;		   	
					if(curindex>=totpicnum)curindex=0;//到末尾的时候,自动从头开始
					break;
				}else if(key==WKUP_PRES)
				{
					res2 = Update_DecodeRGB("0:/");	//保存图片
					if(res2)	Show_Str(30,20,240,16,"文件读取错误！",16,1);
					else 
					{
						Show_Str(30,20,240,16,"图片保存完成！",16,1);
						delay_ms(1000);
						
						LCD_Clear(BLACK);	//清屏
						Show_Decoded_Picture(1,"0:/"); //显示图片
						Show_Str(30,20,240,16,"桌面更新完成!",16,1);
						delay_ms(1000);
						Picture_Fill(30,20,240,16,"0:/");
					}
					break;
				}
				
				t++;
				delay_ms(10); 
			}					    
			res1 = 0; 
			if(res2==0) break;
		} 											  
		myfree(SRAMIN,picfileinfo.lfname);	//释放内存			    
		myfree(SRAMIN,pname);				//释放内存			    
		myfree(SRAMIN,picindextbl);			//释放内存	
	}
	else if(update==2)
	{
		LCD_Clear(BLACK);	//清屏
		Show_Decoded_Picture(1,"0:/"); //显示图片
		Show_Str(30,20,240,16,"桌面更新完成!",16,1);
		delay_ms(1000);
		Picture_Fill(30,20,240,16,"0:/");
	}
}


