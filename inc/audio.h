#ifndef AUDIO_H
#define AUDIO_H

#include "hx_drv_tflm.h"
#include "synopsys_wei_gpio.h"
#include "language.h"

#ifdef ENGLISH
extern unsigned char bike_data[];
extern unsigned char person_data[];
extern unsigned char car_data[];
extern unsigned char right_data[];
extern unsigned char front_data[];
extern unsigned char left_data[];
#else
extern unsigned char bike_chinese_data[];
extern unsigned char person_chinese_data[];
extern unsigned char car_chinese_data[];
extern unsigned char right_chinese_data[];
extern unsigned char front_chinese_data[];
extern unsigned char left_chinese_data[];
#endif

extern unsigned char danger_data[];

void PLAY_AUDIO(int track);

#endif /* AUDIO_H */