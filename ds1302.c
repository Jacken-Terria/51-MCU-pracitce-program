#include "ds1302.h"

//定义要使用的I/O口
sbit CE=P3^5;
sbit SCLK=P3^6;//时钟控制
sbit SDA=P3^4;//数据传输口

uchar Add_write[]={0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c};
//uchar Add_read[]={0x81};无用，已用Add_write+1代替
uchar flag=0;//按键标志位
uchar TIM_Init=0;//控制按键后的第一次重新初始化时钟

uchar TIM[7] = {0x00, 0x00, 0x12, 0x13, 0x11, 0x05, 0x20};
uchar RETIM[7] = {0x00, 0x00, 0x12, 0x13, 0x11, 0x50, 0x20};

void Ds1302Write(uchar addr, uchar dat)
{
	uchar n;
	CE = 0;
	_nop_();

	SCLK = 0;//先将SCLK置低电平。
	_nop_();
	CE = 1; //然后将RST(CE)置高电平。
	_nop_();

	for (n=0; n<8; n++)//开始传送八位地址命令
	{
		SDA = addr & 0x01;//数据从低位开始传送
		addr >>= 1;
		SCLK = 1;//数据在上升沿时，DS1302读取数据
		_nop_();
		SCLK = 0;
		_nop_();
	}
	for (n=0; n<8; n++)//写入8位数据
	{
		SDA = dat & 0x01;
		dat >>= 1;
		SCLK = 1;//数据在上升沿时，DS1302读取数据
		_nop_();
		SCLK = 0;
		_nop_();	
	}	
		 
	CE = 0;//传送数据结束
	_nop_();
}

uchar Ds1302Read(uchar addr)
{
	uchar n,dat,dat1;
	CE = 0;
	_nop_();

	SCLK = 0;//先将SCLK置低电平。
	_nop_();
	CE = 1;//然后将RST(CE)置高电平。
	_nop_();

	for(n=0; n<8; n++)//开始传送八位地址命令--（与写的程序相同）
	{
		SDA = addr & 0x01;//数据从低位开始传送
		addr >>= 1;
		SCLK = 1;//数据在上升沿时，DS1302读取数据
		_nop_();
		SCLK = 0;//DS1302下降沿时，放置数据
		_nop_();
	}
	_nop_();//等待DS1302把数据放到IO口上
	for(n=0; n<8; n++)//读取8位数据
	{
		dat1 = SDA;//从最低位开始接收
		dat = (dat>>1) | (dat1<<7);
		SCLK = 1;
		_nop_();
		SCLK = 0;//DS1302下降沿时，放置数据
		_nop_();
	}

	CE = 0;
	_nop_();	//以下为DS1302复位的稳定时间,必须的。
	SCLK = 1;
	_nop_();
	SDA = 0;
	_nop_();
	SDA = 1;
	_nop_();
	return dat;	
}

void Ds1302Init()
{
	uchar i;

	Ds1302Write(0x8e, 0x00);//关闭写保护

	for(i=0; i<7; i++)
		Ds1302Write(Add_write[i], TIM[i]);

	Ds1302Write(0x8e, 0x8e);
}

void Ds1302ReadTim()
{
	uchar i;
		
	 //正常情况（时钟不停）
	if(flag == 0)
	{
		
		if(TIM_Init==1)//按键后第一次初始化
		{
			for(i=0; i>7; i++)
				TIM[i] = RETIM[i];
			Ds1302Init();
			TIM_Init = 0;
		}
		for(i=0; i<7; i++)
			TIM[i] = Ds1302Read(Add_write[i]+1);
	}
}
