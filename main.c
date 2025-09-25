#include <REG52.H>
#include "beep.h"      // 包含音乐数组 buzzer_music[]

sbit BEEP = P3^4;      // 蜂鸣器，低电平有效
bit  play_flag;        // 播放完成标志

// 全局变量用于音乐播放控制
unsigned char code *music_ptr;  // 当前音符指针
unsigned int note_count;        // 当前音符剩余计数

// 音乐播放速度调节（数值越小播放越快，建议范围：1-4）
#define SPEED_FACTOR 2

void Timer1Init(void)      // 12 MHz 下 1 ms 中断
{
    TMOD &= 0x0F;
    TMOD |= 0x10;          // T1 模式 1
    ET1 = 1;
    EA  = 1;
}

void PlayMusic(unsigned char code *p)
{
    if(p[0]==0 && p[1]==0) 
    { 
        play_flag = 1; 
        TR1 = 0; 
        BEEP = 1; 
        return;
    }
    
    // 设置定时器重装值（决定音调）
    TH1 = p[0]; 
    TL1 = p[1];
    
    // 设置音符持续时间，并应用速度调节
    note_count = p[2]; 
    note_count <<= 8; 
    note_count |= p[3];
    note_count /= SPEED_FACTOR;  // 除以速度因子来加快播放
    
    // 更新音符指针到下一个音符
    music_ptr = p;
    
    TR1 = 1;  // 启动定时器
}

void Timer1_ISR(void) interrupt 3
{
    // 重新装载定时器值
    TH1 = music_ptr[0];  
    TL1 = music_ptr[1];
    
    // 翻转蜂鸣器输出产生方波
    BEEP = ~BEEP;
    
    // 减少音符计数
    if(--note_count == 0)
    {
        // 当前音符播放完毕，切换到下一个音符
        music_ptr += 4;  // 指向下一个音符数据
        PlayMusic(music_ptr);
    }
}

void main(void)
{
    Timer1Init();
    play_flag = 0;
    music_ptr = buzzer_music;  // 初始化音乐指针
    PlayMusic(buzzer_music);   // 开始播放音乐
    
    while(!play_flag);   // 等待播放完成
    while(1);           // 播放完成后停留在这里
}