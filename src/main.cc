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

hx_drv_gpio_config_t hal_gpio_0;
hx_drv_gpio_config_t hal_gpio_1;
hx_drv_gpio_config_t hal_gpio_2;

volatile void delay_ms(uint32_t ms_input);
void GPIO_INIT(void);

char string_buf[100] = "test\n";

// This is the default main used on systems that have the standard C entry
// point. Other devices (for example FreeRTOS or ESP32) that have different
// requirements for entry code (like an app_main function) should specialize
// this main.cc file in a target-specific subfolder.
int main(int argc, char *argv[])
{
    uint32_t msec_x100 = 0;

    hx_drv_uart_initial(UART_BR_115200);
    hx_drv_uart_print("Testing\n");
    GPIO_INIT();
    setup();

    int id = 0;
    int output_record = 0;
    int output = 0;
    int left_result = 0;
    int right_result = 0;
    while (true)
    {
        // hx_drv_uart_print("Start time count: %5d.%1d\n", msec_x100 / 10, msec_x100 % 10);
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

        if (id == 0)
        {
            right_result = loop(id);
        }
        if (id == 1)
        {
            left_result = loop(id);
            hx_drv_uart_print("----------------------------------------------\n");
            hx_drv_uart_print("Right result = %d, Left result = %d\n", right_result, left_result);
            if ((right_result != 0) || (left_result != 0))
            {
                
                if (right_result == left_result) // center
                {
                    output = 3 * right_result + 1;
                }
                else if (right_result != 0) // right
                {
                    output = 3 * right_result + 0;
                }
                else // left
                {
                    output = 3 * left_result + 2;
                }

                if (output != output_record)
                {
                    PLAY_AUDIO(output / 3);
                    PLAY_AUDIO(output % 3 + 4);
                    output_record = output;
                }
            }
        }
        hx_drv_uart_print("----------------------------------------------\n");
        // msec_x100 = msec_x100 + 5;
        delay_ms(500);

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