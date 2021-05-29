#ifndef AUDIO_H
#define AUDIO_H

#include "hx_drv_tflm.h"
#include "synopsys_wei_gpio.h"

extern unsigned char bike_data[];
extern unsigned char person_data[];
extern unsigned char car_data[];
extern unsigned char right_data[];
extern unsigned char front_data[];
extern unsigned char left_data[];

void PLAY_AUDIO(int track);

#endif /* AUDIO_H */