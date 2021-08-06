#include "audio.h"
#include "language.h"

void PLAY_AUDIO(int track)
{
    // track 0: "A person is..."
    // track 1: "A car is..."
    // track 2: "A bike is..."
    // track 3: "...on your right"
    // track 4: "...in front of you"
    // track 5: "...on your left"

    uint32_t sec_cnt = 0;
    uint32_t real_sec = 0;
    uint32_t bitnum = 0;
    uint32_t bclk = 0;
    uint32_t audiolr = 0;
    uint32_t resolution = 16;
    uint32_t offset = 0;
    uint32_t addr = 0;
    char audio_buffer[2];
    uint32_t i = 0;
    uint32_t num_samples;
    unsigned char *audio_data;

    switch (track)
    {
    case 1: // person
#ifdef CHINESE
        num_samples = 9816 / 2;
        audio_data = person_chinese_data;
        hx_drv_uart_print("PERSON\n");
#else
        num_samples = 15023 / 2;
        audio_data = person_data;
        hx_drv_uart_print("A person is ");
#endif
        break;
    case 2: // car
#ifdef CHINESE
        num_samples = 9528 / 2;
        audio_data = car_chinese_data;
        hx_drv_uart_print("CAR\n");
#else
        num_samples = 13871 / 2;
        audio_data = car_data;
        hx_drv_uart_print("A car is ");
#endif
        break;
    case 3: // bike
#ifdef CHINESE
        num_samples = 13560 / 2;
        audio_data = bike_chinese_data;
        hx_drv_uart_print("BIKE\n");
#else
        num_samples = 13871 / 2;
        audio_data = bike_data;
        hx_drv_uart_print("A bike is ");
#endif
        break;
    case 4: // right
#ifdef CHINESE
        num_samples = 12696 / 2;
        audio_data = right_chinese_data;
        hx_drv_uart_print("RIGHT ");
#else
        num_samples = 13295 / 2;
        audio_data = right_data;
        hx_drv_uart_print("on your right.\n");
#endif
        break;
    case 5: // front
#ifdef CHINESE
        num_samples = 13272 / 2;
        audio_data = front_chinese_data;
        hx_drv_uart_print("FRONT ");
#else
        num_samples = 14447 / 2;
        audio_data = front_data;
        hx_drv_uart_print("in front of you.\n");
#endif
        break;
    case 6: // left
#ifdef CHINESE
        num_samples = 12984 / 2;
        audio_data = left_chinese_data;
        hx_drv_uart_print("LEFT ");
#else
        num_samples = 13295 / 2;
        audio_data = left_data;
        hx_drv_uart_print("on your left.\n");
#endif
        break;
    case 7: // danger
        num_samples = 28271;
        // num_samples = 10000 / 2;
        audio_data = danger_data;
        hx_drv_uart_print("Danger, Danger.\n");
        break;
    default:
        num_samples = 0;
        audio_data = 0;
        break;
    }

    // ---START---
    // hx_drv_uart_print("PLAY AUDIO TRACK: %d\n", track);
    while (1)
    {
        real_sec = sec_cnt % 64;
        bitnum = (real_sec % (resolution * 2)) / 2;
        audiolr = real_sec / (resolution * 2);
        bclk = real_sec % 2;

        // Read Audio Data
        if (real_sec == 2)
        {
            if (i < num_samples)
            {
                audio_buffer[0] = (audio_data[2 * i] & 0x00ff);
                audio_buffer[1] = (audio_data[2 * i + 1] & 0x00ff);
                i++;
            }
            else
            {
                break;
            }
        }

        // data bit line
        addr = 1 - (((bitnum + 7) / 8) % 2);
        offset = (8 - (bitnum % 8)) % 8;

        switch ((audio_buffer[addr] >> offset) & 0x01)
        {
        case 0:
            hal_gpio_set(&hal_gpio_1, GPIO_PIN_RESET);
            break;
        case 1:
            hal_gpio_set(&hal_gpio_1, GPIO_PIN_SET);
            break;
        default:
            break;
        }

        // BCLK
        switch (bclk)
        {
        case 0:
            hal_gpio_set(&hal_gpio_0, GPIO_PIN_RESET);
            break;
        case 1:
            hal_gpio_set(&hal_gpio_0, GPIO_PIN_SET);
            break;
        default:
            break;
        }

        // LRCK
        switch (audiolr)
        {
        case 0:
            hal_gpio_set(&hal_gpio_2, GPIO_PIN_RESET);
            break;
        case 1:
            hal_gpio_set(&hal_gpio_2, GPIO_PIN_SET);
            break;
        default:
            break;
        }

        // board_delay_cycle(30);
        sec_cnt++;
    }
    // hx_drv_uart_print("AUDIO FINISH\n");
}
