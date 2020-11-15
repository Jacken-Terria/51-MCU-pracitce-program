#ifndef __DS1302_H
#define __DS1302_H

#include "reg52.h"
#include "intrins.h"

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

void Ds1302Write(uchar add, uchar dat);
uchar Ds1302Read(uchar add);
void Ds1302Init();
void Ds1302ReadTim();
void Ds1302Reinit();//重复初始化函数

extern uchar TIM[7];
extern uchar RETIM[7];
extern uchar Time_switch;
extern uchar flag;
extern uchar TIM_Init;
#endif
