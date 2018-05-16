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


//������ͼƬ����ڴ����е�·��
u8*const PICTURE_PATH="/PICTURE/01.bin";		


//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//��Ŀ¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	return rval;
}

//��ָ���㿪ʼ����ȡnum�����ص���ɫֵ
//�����ң����ϵ���ɨ��
//����ʵ�ʶ����ĵ����
u16 Get_DecodeRGB(u8* RGB_buf,u16 x, u16 y, u16 num)
{
	u16 i;
	u16 buf;
	u16 x0;
	
	x0 = x;	//����ɨ�����ʼ��
	for(i=0;i<num;i++)
	{
		if(x>=lcddev.width)	//һ��ɨ���꣬ת��һ��
		{
			y++;
			x = x0;
		}
		else	x++;
		if(y>=lcddev.height)	return i;	//ɨ�����
		
		buf = LCD_ReadPoint(x,y);
		RGB_buf[2*i] = buf>>8;	//��ȡ��8λ
		RGB_buf[2*i+1] = buf;	//��ȡ��8λ
	}
	return i;
}

//���½����ͼƬ
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
	pname = mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�  
	RGB_buf = mymalloc(SRAMIN,datanum);	//����lcddev.width*2�ֽ��ڴ�  
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�	
	if(RGB_buf==NULL||pname==NULL||fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,RGB_buf);
		return 5;	//�ڴ�����ʧ��
	}
	//�Ȳ����ļ��Ƿ����� 
	strcpy((char*)pname,(char*)src);	//copy src���ݵ�pname
	strcat((char*)pname,(char*)PICTURE_PATH); 
	res = f_open(fftemp,(const TCHAR*)pname,FA_WRITE|FA_OPEN_ALWAYS); //��д���ļ�,�ļ��������򴴽�
 	if(res) rval|=1<<4;	//���ļ�ʧ��
	
	for(i=0;i<lcddev.height;i++)
	{
		Get_DecodeRGB(RGB_buf,0,i,datanum/2);	//ÿ�ζ�ȡһ�е�
		res = f_write(fftemp,RGB_buf,datanum,&bwrite);	//д�뵽�ļ�
		if((res!=FR_OK)||(bwrite!=datanum))	break;				//ִ�д���
	}
	
	f_close(fftemp);	//�ر��ļ�
	myfree(SRAMIN,fftemp);	//�ͷ��ڴ�
	myfree(SRAMIN,pname);	//�ͷ��ڴ�
	myfree(SRAMIN,RGB_buf);	//�ͷ��ڴ�
	return res;
}

//��ʾͼƬ
//modeΪ0��ʾ����Ƿ���ͼƬ�ļ���modeΪ1��ʾ��ʾͼƬ
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
	pname = mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�  
	RGB_buf = mymalloc(SRAMIN,datanum);	//����lcddev.width*2�ֽ��ڴ� 
	RGB_pic	= mymalloc(SRAMIN,datanum);	//����lcddev.width*2�ֽ��ڴ� 
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�	
	if(RGB_buf==NULL||pname==NULL||fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,RGB_buf);
		return 5;	//�ڴ�����ʧ��
	}
	
	//�Ȳ����ļ��Ƿ����� 
	strcpy((char*)pname,(char*)src);	//copy src���ݵ�pname
	strcat((char*)pname,(char*)PICTURE_PATH); 
	
	res = f_open(fftemp,(const TCHAR*)pname,FA_READ); //�Զ����ļ�
	if(mode==0)	return res;	//���ͼƬ�ļ��Ƿ�����
	
 	if(res) rval|=1<<4;	//���ļ�ʧ��
	
	for(i=0;i<lcddev.height;i++)
	{
		res = f_read(fftemp,RGB_buf,datanum,&bread);	//�����ļ�
		if(res!=FR_OK)	break;				//ִ�д���
		for(j=0;j<bread/2;j++)
		{
			RGB_pic[j] = (u16)RGB_buf[2*j]<<8|(u16)RGB_buf[2*j+1];	//ת����u16
		}
		LCD_Color_Fill(0,i,j-1,i,RGB_pic);	//��һ��
		
		if(bread!=datanum)	break;	//�Ѷ���
	}
	
	f_close(fftemp);	//�ر��ļ�
	myfree(SRAMIN,fftemp);	//�ͷ��ڴ�
	myfree(SRAMIN,pname);	//�ͷ��ڴ�
	myfree(SRAMIN,RGB_buf);	//�ͷ��ڴ�
	myfree(SRAMIN,RGB_pic);	//�ͷ��ڴ�
	
	return res;
}

//��һ����������ˢ��ͼƬ
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
	pname = mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�  
	RGB_buf = mymalloc(SRAMIN,datanum);	//����width*2�ֽ��ڴ� 
	RGB_pic	= mymalloc(SRAMIN,datanum);	//����width*2�ֽ��ڴ� 	
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�	
	if(RGB_buf==NULL||pname==NULL||fftemp==NULL||RGB_pic==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,RGB_buf);
		myfree(SRAMIN,RGB_pic);
		return 5;	//�ڴ�����ʧ��
	}
	
	//�Ȳ����ļ��Ƿ����� 
	strcpy((char*)pname,(char*)src);	//copy src���ݵ�pname
	strcat((char*)pname,(char*)PICTURE_PATH); 
	res = f_open(fftemp,(const TCHAR*)pname,FA_READ); //�Զ����ļ�
 	if(res) rval|=1<<4;	//���ļ�ʧ��
	
	for(i=0;i<height;i++)
	{
		pointer = ((y+i)*lcddev.width+x)*2;
		res = f_lseek(fftemp,pointer);	//�ƶ���ָ����
		res = f_read(fftemp,RGB_buf,datanum,&bread);	//�����ļ���width*2���ֽ�
		if(res!=FR_OK)	break;				//ִ�д���
		for(j=0;j<width;j++)
		{
			RGB_pic[j] = (u16)RGB_buf[2*j]<<8|(u16)RGB_buf[2*j+1];	//ת����u16,RGB��ʽ
		}
		LCD_Color_Fill(x,y+i,x+width-1,y+i,RGB_pic);	//��ָ�����������ڵ�һ��
		
		if(bread!=datanum)	break;	//�Ѷ���
	}
	
	f_close(fftemp);	//�ر��ļ�
	myfree(SRAMIN,fftemp);	//�ͷ��ڴ�
	myfree(SRAMIN,pname);	//�ͷ��ڴ�
	myfree(SRAMIN,RGB_buf);	//�ͷ��ڴ�
	myfree(SRAMIN,RGB_pic);	//�ͷ��ڴ�
	
	return res;
}


//��������
void Load_Windows(u16 x,u8 update)	
{
	DIR picdir;	 		//ͼƬĿ¼
	FILINFO picfileinfo;//�ļ���Ϣ
	u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 totpicnum;
	u16 curindex;		//ͼƬ��ǰ����
	u16 *picindextbl;	//ͼƬ������
	u8 res1, res2;
	u16 temp;
	u8 key;
	u8 t = 0;
	
	if(update==1)	//�������汳��
	{
		LCD_Clear(BLACK);	//����
		LCD_ShowString(30,30,200,16,16,"Update Picture:",1);		//��ʾ��ʾ��Ϣ
		LCD_ShowString(50,50,200,16,16,"Picture UP:KEY0",1);
		LCD_ShowString(50,70,200,16,16,"Picture DOWN:KEY2",1);
		LCD_ShowString(50,90,200,16,16,"Picture Update:WAKE_UP",1);
		LCD_ShowString(30,110,300,16,16,"Please take KEY1 to continue...",1);
		while(key!=KEY1_PRES)
		{
			key = KEY_Scan(0);
			delay_ms(10);
		}
		LCD_Clear(BLACK);	//����
		
		while(f_opendir(&picdir,"0:/PICTURE"))//��ͼƬ�ļ���
		{	    
			Show_Str(30,x,240,16,"PICTURE�ļ��д���!",16,0);
			delay_ms(200);				  
			LCD_Fill(30,170,240,186,WHITE);//�����ʾ	     
			delay_ms(200);				  
		}  
		totpicnum = pic_get_tnum("0:/PICTURE"); //�õ�����Ч�ļ���

		while(totpicnum==NULL)//ͼƬ�ļ�Ϊ0		
		{	    
			Show_Str(30,x,240,16,"û��ͼƬ�ļ�!",16,0);
			delay_ms(200);				  
			LCD_Fill(30,x,240,186,BLACK);//�����ʾ	     
			delay_ms(200);				  
		}

		picfileinfo.lfsize = _MAX_LFN*2+1;		//���ļ�����󳤶�
		picfileinfo.lfname = mymalloc(SRAMIN,picfileinfo.lfsize);	//Ϊ���ļ������������ڴ�
		pname = mymalloc(SRAMIN,picfileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
		picindextbl = mymalloc(SRAMIN,2*totpicnum);				//����2*totpicnum���ֽڵ��ڴ�,���ڴ��ͼƬ����

		while(picfileinfo.lfname==NULL||pname==NULL||picindextbl==NULL)//�ڴ�������
		{	    
			Show_Str(30,x,240,16,"�ڴ����ʧ��!",16,0);
			delay_ms(200);				  
			LCD_Fill(30,x,240,186,BLACK);//�����ʾ	     
			delay_ms(200);				  
		}  	
		//��¼����
		res1 = f_opendir(&picdir,"0:/PICTURE"); //��Ŀ¼
		if(res1==FR_OK)
		{
			curindex=0;//��ǰ����Ϊ0
			while(1)//ȫ����ѯһ��
			{
				temp=picdir.index;	//��¼��ǰindex
				res1=f_readdir(&picdir,&picfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
				
				if(res1!=FR_OK||picfileinfo.fname[0]==0)	break;	//������/��ĩβ��,�˳�		
				
				fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
				res1=f_typetell(fn);	
				if((res1&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
				{
					picindextbl[curindex]=temp;//��¼����
					curindex++;
				}	    
			} 
		}   

		curindex=0;											//��0��ʼ��ʾ
		res1=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//��Ŀ¼
		while(res1==FR_OK)//�򿪳ɹ�
		{	
			dir_sdi(&picdir,picindextbl[curindex]);			//�ı䵱ǰĿ¼����	  		
			res1=f_readdir(&picdir,&picfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
			
			if(res1!=FR_OK&&picfileinfo.fname[0]==0) break;		//������/��ĩβ��,�˳�
			
			fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
			strcpy((char*)pname,"0:/PICTURE/");				//����·��(Ŀ¼)
			strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
			
			Show_Str(20,20,240,16,"׼����������...",16,1);
			delay_ms(1000);
			
			LCD_Clear(BLACK);	//����
			ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);	//��ʾͼƬ
		
			while(1) 
			{
				key=KEY_Scan(0);		//ɨ�谴��
				
				if((t%20)==0)	LED0 = !LED0; //LED0��˸,��ʾ������������.
				if(key==KEY2_PRES)		//��һ��
				{
					if(curindex)curindex--;
					else curindex=totpicnum-1;
					break;
				}else if(key==KEY0_PRES)//��һ��
				{
					curindex++;		   	
					if(curindex>=totpicnum)curindex=0;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
					break;
				}else if(key==WKUP_PRES)
				{
					res2 = Update_DecodeRGB("0:/");	//����ͼƬ
					if(res2)	Show_Str(30,20,240,16,"�ļ���ȡ����",16,1);
					else 
					{
						Show_Str(30,20,240,16,"ͼƬ������ɣ�",16,1);
						delay_ms(1000);
						
						LCD_Clear(BLACK);	//����
						Show_Decoded_Picture(1,"0:/"); //��ʾͼƬ
						Show_Str(30,20,240,16,"����������!",16,1);
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
		myfree(SRAMIN,picfileinfo.lfname);	//�ͷ��ڴ�			    
		myfree(SRAMIN,pname);				//�ͷ��ڴ�			    
		myfree(SRAMIN,picindextbl);			//�ͷ��ڴ�	
	}
	else if(update==2)
	{
		LCD_Clear(BLACK);	//����
		Show_Decoded_Picture(1,"0:/"); //��ʾͼƬ
		Show_Str(30,20,240,16,"����������!",16,1);
		delay_ms(1000);
		Picture_Fill(30,20,240,16,"0:/");
	}
}


