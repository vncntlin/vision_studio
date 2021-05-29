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
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "main_functions.h"

#include "detection_responder.h"
#include "image_provider.h"
#include "model_settings.h"
#include "vww_cnn_v4_10_person.h"
#include "vww_cnn_v4_15_car.h"
#include "vww_cnn_v4_15_bicycle.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace
{
  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model_person = nullptr;
  const tflite::Model *model_car = nullptr;
  const tflite::Model *model_bicycle = nullptr;
  tflite::MicroInterpreter *interpreter_person = nullptr;
  tflite::MicroInterpreter *interpreter_car = nullptr;
  tflite::MicroInterpreter *interpreter_bicycle = nullptr;
  TfLiteTensor *input_person = nullptr;
  TfLiteTensor *input_car = nullptr;
  TfLiteTensor *input_bicycle = nullptr;

  // In order to use optimized tensorflow lite kernels, a signed int8_t quantized
  // model is preferred over the legacy unsigned model format. This means that
  // throughout this project, input images must be converted from unisgned to
  // signed format. The easiest and quickest way to convert from unsigned to
  // signed 8-bit integers is to subtract 128 from the unsigned value to get a
  // signed value.

  // An area of memory to use for input, output, and intermediate arrays.
  constexpr int kTensorArenaSize_person = 144 * 1024;
  constexpr int kTensorArenaSize_car = 144 * 1024;
  constexpr int kTensorArenaSize_bicycle = 144 * 1024;
  static uint8_t tensor_arena_person[kTensorArenaSize_person];
  static uint8_t tensor_arena_car[kTensorArenaSize_car];
  static uint8_t tensor_arena_bicycle[kTensorArenaSize_bicycle];
} // namespace

// The name of this function is important for Arduino compatibility.
void setup()
{
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model_person = tflite::GetModel(vww_cnn_v4_10_person_tflite);
  if (model_person->version() != TFLITE_SCHEMA_VERSION)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model_person->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  model_car = tflite::GetModel(vww_cnn_v4_15_car_tflite);
  if (model_car->version() != TFLITE_SCHEMA_VERSION)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model_car->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  model_bicycle = tflite::GetModel(vww_cnn_v4_15_bicycle_tflite);
  if (model_bicycle->version() != TFLITE_SCHEMA_VERSION)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model_bicycle->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  // tflite::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver<7> micro_op_resolver;
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddMaxPool2D();
  micro_op_resolver.AddRelu();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();

  // static tflite::MicroMutableOpResolver<5> micro_op_resolver_car;
  // micro_op_resolver_car.AddConv2D();
  // micro_op_resolver_car.AddFullyConnected();
  // micro_op_resolver_car.AddMaxPool2D();
  // micro_op_resolver_car.AddReshape();
  // micro_op_resolver_car.AddSoftmax();

  // static tflite::MicroMutableOpResolver<5> micro_op_resolver;
  // micro_op_resolver.AddAveragePool2D();
  // micro_op_resolver.AddConv2D();
  // micro_op_resolver.AddDepthwiseConv2D();
  // micro_op_resolver.AddReshape();
  // micro_op_resolver.AddSoftmax();

  // Build an interpreter to run the model with.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroInterpreter static_interpreter_person(
      model_person, micro_op_resolver, tensor_arena_person, kTensorArenaSize_person, error_reporter);
  interpreter_person = &static_interpreter_person;
  static tflite::MicroInterpreter static_interpreter_car(
      model_car, micro_op_resolver, tensor_arena_car, kTensorArenaSize_car, error_reporter);
  interpreter_car = &static_interpreter_car;
  static tflite::MicroInterpreter static_interpreter_bicycle(
      model_bicycle, micro_op_resolver, tensor_arena_bicycle, kTensorArenaSize_bicycle, error_reporter);
  interpreter_bicycle = &static_interpreter_bicycle;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter_person->AllocateTensors();
  if (allocate_status != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed(person)");
    return;
  }

  allocate_status = interpreter_car->AllocateTensors();
  if (allocate_status != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed(car)");
    return;
  }

  allocate_status = interpreter_bicycle->AllocateTensors();
  if (allocate_status != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed(bicycle)");
    return;
  }

  // Get information about the memory area to use for the model's input.
  input_person = interpreter_person->input(0);
  input_car = interpreter_car->input(0);
  input_bicycle = interpreter_bicycle->input(0);
}

// The name of this function is important for Arduino compatibility.
int loop(int id)
{
  // Get image from provider.
  if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels, id,
                            input_person->data.int8, input_car->data.int8, input_bicycle->data.int8))
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
  }

  // Run the model on this input and make sure it succeeds.
  if (kTfLiteOk != interpreter_person->Invoke())
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.(person)");
  }
  if (kTfLiteOk != interpreter_car->Invoke())
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.(car)");
  }
  if (kTfLiteOk != interpreter_bicycle->Invoke())
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.(bicycle)");
  }

  TfLiteTensor *output_person = interpreter_person->output(0);
  TfLiteTensor *output_car = interpreter_car->output(0);
  TfLiteTensor *output_bicycle = interpreter_bicycle->output(0);

  // Process the inference results.
  int8_t person_score = output_person->data.uint8[kPersonIndex];
  int8_t no_person_score = output_person->data.uint8[kNotAPersonIndex];
  int8_t car_score = output_car->data.uint8[kPersonIndex];
  int8_t no_car_score = output_car->data.uint8[kNotAPersonIndex];
  int8_t bicycle_score = output_bicycle->data.uint8[kPersonIndex];
  int8_t no_bicycle_score = output_bicycle->data.uint8[kNotAPersonIndex];
  RespondToDetectionPerson(error_reporter, person_score, no_person_score);
  RespondToDetectionCar(error_reporter, car_score, no_car_score);
  RespondToDetectionBicycle(error_reporter, bicycle_score, no_bicycle_score);

  if (person_score > no_person_score)
    return 1;
  else if (car_score > no_car_score)
    return 2;
  else if (bicycle_score > no_bicycle_score)
    return 3;
  else
    return 0;
}
