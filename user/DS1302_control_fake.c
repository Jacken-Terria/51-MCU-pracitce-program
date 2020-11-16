//通过按键控制DS1302时钟的开关(伪)+定时器控制延时+蜂鸣器(受按键控制)
#include "reg52.h"//一些特殊寄存器定义（官方提供）
#include "ds1302.h"//关于DS1302部分乱七八糟的定义

typedef unsigned char u8;
typedef unsigned int u16;

//中断按键
sbit key=P3^2;
sbit led=P2^0;

//动态数码管
sbit LSA=P2^2;
sbit LSB=P2^3;
sbit LSC=P2^4;

//蜂鸣器
sbit beep = P1^5;

u8 code smgduan[]={0x3f, 0x06, 0x5b, 0x4f, 0x66,
					0x6d, 0x7d, 0x07, 0x7f, 0x6f, 
					0x77, 0x7c, 0x39, 0x5e, 0x79, 
					0x71};
u8 Display[8];

void delay(u16 i)
{
	while(i--);
}

void EXIT0Init()//外部中断0使能函数
{
	IT0 = 1;
	EA = 1;
	EX0 = 1;

	led = 0;
}

void Timer0Init()//定时器中断0使能函数
{
	TMOD = 0x01|TMOD;//定时模式
	//设定装载值
	TH0 = 0xfc;
	TL0 = 0x18;
	//使能定时器中断
	ET0 = 1;

	EA = 1;
	//开启定时器
	TR0 = 1;
}

void UsartInit()
{
	TMOD = 0x20;
	//设置波特率4800
	TH1 = 0xf3;
	TL1 = 0xf3;
	//设置波特率倍增（SMOD=1）
	PCON = 0x80;
	SCON = 0x50;
	ES = 1;
	EA = 1;
	//开启中断
	TR1 = 1;
}

void Datapros()//时钟数据处理函数
{
	Ds1302ReadTim();
	Display[0] = smgduan[TIM[2]>>4];//时
	Display[1] = smgduan[TIM[2]&0x0f];
	Display[2] = 0x40;
	Display[3] = smgduan[TIM[1]>>4];//分
	Display[4] = smgduan[TIM[1]&0x0f];
	Display[5] = 0x40;
	Display[6] = smgduan[TIM[0]>>4];//秒
	Display[7] = smgduan[TIM[0]&0x0f];
}

void DigDisplay()
{
	u8 i;
	for(i=0; i<8; i++)
	{
		switch(i)
		{
			case 0:
				LSA=0; LSB=0; LSC=0;
				break;			
			case 1:
				LSA=1; LSB=0; LSC=0;
				break;			
			case 2:
				LSA=0; LSB=1; LSC=0;
				break;			
			case 3:
				LSA=1; LSB=1; LSC=0;
				break;			
			case 4:
				LSA=0; LSB=0; LSC=1;
				break;			
			case 5:
				LSA=1; LSB=0; LSC=1;
				break;			
			case 6:
				LSA=0; LSB=1; LSC=1;
				break;
			case 7:
				LSA=1; LSB=1; LSC=1;
				break;
		}
		P0 = Display[7-i];
		delay(100);
		P0 = 0x00;
	}
}

void main()
{
	Timer0Init();
	EXIT0Init();
	Ds1302Init();
	UsartInit();
	while(1)
	{
		Datapros();
		DigDisplay();
	}
}

void TimeSwitch() interrupt 0//按键中断服务函数
{
	u8 In=0;
	u8 i;

	delay(100);
	if(key==0 && flag==0 && In==0)//停止时钟按键
	{
		led = ~led;

		//设定重装载值
		for(i=0; i<7; i++)
			RETIM[i] = TIM[i];

		flag = 1;
		TIM_Init = 1;//使能初始化
		In = 1;
		TR0 = 0;//停止计时器
	}

	if(key==0 && flag==1 && In==0)
	{
		led = ~led;
		flag = 0;
		In = 1;
		TR0 = 1;
	}
}

void Time0() interrupt 1
{
	static u16 i=0;
	static u8 t_n=0;
	u8 t;

	//重装载1ms
	TH0 = 0xfc;
	TL0 = 0x18;
	i++;

	if(i==1000)//1s
	{	
		i = 0;//i清零
		while(t_n==9)//10s--目前改进点：尝试减少定时误差
		{
			t = 100;//控制占空比
			while(t--)//蜂鸣器响
			{
				beep = ~beep;
				delay(100);
			}
			t_n = 0;
		}
		t_n++;
	}
}

处于BUG状态中
void Usart() interrupt 4
{
	u8 receiveData;
	receiveData = SBUF;
	//把标志位置1，等待下一次接收中断
	RI = 0;
	SBUF = receiveData;
	while(!TI);
	//等待下一次发送中断
	TI = 0;
} 
