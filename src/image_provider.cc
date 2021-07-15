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

#include "image_provider.h"
#include "model_settings.h"

#include "hx_drv_tflm.h"

hx_drv_sensor_image_config_t g_pimg_config;

TfLiteStatus GetImage(tflite::ErrorReporter *error_reporter, int image_width,
                      int image_height, int channels, int id, int8_t *image_data_person, int8_t *image_data_car, int8_t *image_data_bicycle)
{
  static bool is_initialized = false;

  if (!is_initialized)
  {
    if (hx_drv_sensor_initial(&g_pimg_config) != HX_DRV_LIB_PASS)
    {
      return kTfLiteError;
    }
    is_initialized = true;
  }

  hx_drv_sensor_capture(&g_pimg_config);
  uint8_t *img_ptr;
  switch (id)
  {
  case 0:
    img_ptr = (uint8_t *)g_pimg_config.raw_address;
    break;
  case 1:
    img_ptr = (uint8_t *)g_pimg_config.raw_address + g_pimg_config.img_width * (g_pimg_config.img_height / 3);
    break;
  // case 2:
  //   img_ptr = (uint8_t *)g_pimg_config.raw_address + g_pimg_config.img_width * (g_pimg_config.img_height * 2 / 4);
  //   break;
  default:
    img_ptr = 0;
    break;
  }

  hx_drv_image_rescale((uint8_t *)img_ptr,
                       g_pimg_config.img_width, g_pimg_config.img_height * 2 / 3,
                       image_data_person, image_width, image_height);
  hx_drv_image_rescale((uint8_t *)img_ptr,
                       g_pimg_config.img_width, g_pimg_config.img_height * 2 / 3,
                       image_data_car, image_width, image_height);
  hx_drv_image_rescale((uint8_t *)img_ptr,
                       g_pimg_config.img_width, g_pimg_config.img_height * 2 / 3,
                       image_data_bicycle, image_width, image_height);

  return kTfLiteOk;
}
