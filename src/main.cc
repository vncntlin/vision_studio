/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "main_functions.h"
#include "hx_drv_tflm.h"
#include "stdio.h"
#include "audio.h"
#include "language.h"

hx_drv_gpio_config_t hal_gpio_0;
hx_drv_gpio_config_t hal_gpio_1;
hx_drv_gpio_config_t hal_gpio_2;

volatile void delay_ms(uint32_t ms_input);
void GPIO_INIT(void);

#define accel_scale 10
typedef struct
{
    uint8_t symbol;
    uint32_t int_part;
    uint32_t frac_part;
} accel_type;

char string_buf[100] = "test\n";

#define accel_scale 10

// This is the default main used on systems that have the standard C entry
// point. Other devices (for example FreeRTOS or ESP32) that have different
// requirements for entry code (like an app_main function) should specialize
// this main.cc file in a target-specific subfolder.
int main(int argc, char *argv[])
{
    // uint32_t msec_x100 = 0;
    int32_t int_buf;
    accel_type accel_x, accel_y, accel_z;

    hx_drv_uart_initial(UART_BR_115200);

    if (hx_drv_accelerometer_initial() != HX_DRV_LIB_PASS)
        hx_drv_uart_print("Accelerometer Initialize Fail\n");
    else
        hx_drv_uart_print("Accelerometer Initialize Success\n");

    hx_drv_uart_print("Testing\n");
    GPIO_INIT();
    setup();

    int id = 0;
    // int output_record = 0;
    // int output = 0;
    int left_result = 0;
    int right_result = 0;
    uint32_t iter = 0;
    uint32_t rightres = 0;
    uint32_t leftres = 0;

    while (true)
    {
        // hx_drv_uart_print("Start time count: %5d.%1d\n", msec_x100 / 10, msec_x100 % 10);

        // =========================
        //      ACCELEROMETER
        // =========================

        uint32_t available_count = 0;
        float x, y, z;
        available_count = hx_drv_accelerometer_available_count();
        // hx_drv_uart_print("Accel get FIFO: %d\n", available_count);
        for (int i = 0; i < available_count; i++)
        {
            hx_drv_accelerometer_receive(&x, &y, &z);
        }

        int_buf = x * accel_scale; //scale value
        if (int_buf < 0)
        {
            int_buf = int_buf * -1;
            accel_x.symbol = '-';
        }
        else
        {
            accel_x.symbol = '+';
        }
        accel_x.int_part = int_buf / accel_scale;
        accel_x.frac_part = int_buf % accel_scale;

        int_buf = y * accel_scale; //scale value
        if (int_buf < 0)
        {
            int_buf = int_buf * -1;
            accel_y.symbol = '-';
        }
        else
        {
            accel_y.symbol = '+';
        }
        accel_y.int_part = int_buf / accel_scale;
        accel_y.frac_part = int_buf % accel_scale;

        int_buf = z * accel_scale; //scale value
        if (int_buf < 0)
        {
            int_buf = int_buf * -1;
            accel_z.symbol = '-';
        }
        else
        {
            accel_z.symbol = '+';
        }
        accel_z.int_part = int_buf / accel_scale;
        accel_z.frac_part = int_buf % accel_scale;

        // PRINT ACCEL
        // sprintf(string_buf, "%c%1d.%1d | %c%1d.%1d | %c%1d.%1d G\n",
        //         accel_x.symbol, accel_x.int_part, accel_x.frac_part,
        //         accel_y.symbol, accel_y.int_part, accel_y.frac_part,
        //         accel_z.symbol, accel_z.int_part, accel_z.frac_part);
        // hx_drv_uart_print(string_buf);
        // PRINT ACCEL

        if (accel_y.int_part >= 2)
        {
            hx_drv_uart_print("!!!DANGER DANGER!!!\n\n");
            PLAY_AUDIO(7);
        }

        // =========================
        //      IMAGE + NN
        // =========================

        switch (id)
        {
        case 0: // Right
            hx_drv_uart_print("RIGHT:\n");
            break;
        // case 1: // Center
        //     hx_drv_uart_print("Center:\n");
        //     break;
        case 1: // Left
            hx_drv_uart_print("LEFT:\n");
            break;
        default:
            break;
        }
        uint32_t tick_start = 0;
        uint32_t tick_end = 0;
        if (id == 0)
        {
            hx_drv_tick_start();
            hx_drv_tick_get(&tick_start);
            right_result = loop(id);
            hx_drv_tick_get(&tick_end);

            hx_drv_uart_print("START: %d, END: %d", tick_start, tick_end);
        }
        if (id == 1)
        {
            left_result = loop(id);
            hx_drv_uart_print("             --pcb--\n");
            hx_drv_uart_print("Right result = %03d\nLeft  result = %03d\n", right_result, left_result);

            for (iter = 0; iter < 3; iter = iter + 1)
            {
                if (iter == 0)
                {
                    rightres = right_result / 100 % 10;
                    leftres = left_result / 100 % 10;
                }
                else if (iter == 1)
                {
                    rightres = right_result / 10 % 10;
                    leftres = left_result / 10 % 10;
                }
                else
                {
                    rightres = right_result % 10;
                    leftres = left_result % 10;
                }

                // hx_drv_uart_print("TEST: %d, %d\n", rightres, leftres);

                if (rightres == 1 && leftres == 1) // center
                {
#ifdef ENGLISH
                    PLAY_AUDIO(iter + 1);
                    PLAY_AUDIO(5);
#else
                    PLAY_AUDIO(5);
                    PLAY_AUDIO(iter + 1);
#endif
                }
                else if (rightres == 1) // right
                {
#ifdef ENGLISH
                    PLAY_AUDIO(iter + 1);
                    PLAY_AUDIO(4);
#else
                    PLAY_AUDIO(4);
                    PLAY_AUDIO(iter + 1);
#endif
                }
                else if (leftres == 1) // left
                {
#ifdef ENGLISH
                    PLAY_AUDIO(iter + 1);
                    PLAY_AUDIO(6);
#else
                    PLAY_AUDIO(6);
                    PLAY_AUDIO(iter + 1);
#endif
                }
            }
            hx_drv_uart_print("===========================================\n\n\n");
        }

        // msec_x100 = msec_x100 + 5;
        delay_ms(50);

        if (id == 0)
            id = 1;
        else
            id = 0;
    }
}

volatile void delay_ms(uint32_t ms_input)
{
    volatile uint32_t i = 0;
    volatile uint32_t j = 0;

    for (i = 0; i < ms_input; i++)
        for (j = 0; j < 40000; j++)
            ;
}

void GPIO_INIT(void)
{
    if (hal_gpio_init(&hal_gpio_0, HX_DRV_PGPIO_0, HX_DRV_GPIO_OUTPUT, GPIO_PIN_RESET) == HAL_OK)
        hx_drv_uart_print("GPIO0 Initialized: OK\n");
    else
        hx_drv_uart_print("GPIO0 Initialized: Error\n");

    if (hal_gpio_init(&hal_gpio_1, HX_DRV_PGPIO_1, HX_DRV_GPIO_OUTPUT, GPIO_PIN_RESET) == HAL_OK)
        hx_drv_uart_print("GPIO1 Initialized: OK\n");
    else
        hx_drv_uart_print("GPIO1 Initialized: Error\n");

    if (hal_gpio_init(&hal_gpio_2, HX_DRV_PGPIO_2, HX_DRV_GPIO_OUTPUT, GPIO_PIN_RESET) == HAL_OK)
        hx_drv_uart_print("GPIO2 Initialized: OK\n");
    else
        hx_drv_uart_print("GPIO2 Initialized: Error\n");

    // if (hal_gpio_init(&hal_led_r, HX_DRV_LED_RED, HX_DRV_GPIO_OUTPUT, GPIO_PIN_RESET) == HAL_OK)
    //     hx_drv_uart_print("GPIO_LED_RED Initialized: OK\n");
    // else
    //     hx_drv_uart_print("GPIO_LED_RED Initialized: Error\n");

    // if (hal_gpio_init(&hal_led_g, HX_DRV_LED_GREEN, HX_DRV_GPIO_OUTPUT, GPIO_PIN_RESET) == HAL_OK)
    //     hx_drv_uart_print("GPIO_LED_GREEN Initialized: OK\n");
    // else
    //     hx_drv_uart_print("GPIO_LED_GREEN Initialized: Error\n");
}