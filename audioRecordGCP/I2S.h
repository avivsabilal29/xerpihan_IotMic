#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"
#define SAMPLE_RATE (44100)
#define PIN_I2S_BCLK 26
#define PIN_I2S_LRC 22
#define PIN_I2S_DIN 34
#define PIN_I2S_DOUT 25
void I2S_Init(i2s_mode_t MODE, i2s_bits_per_sample_t BPS);
int I2S_Read(char* data, int numData);
void I2S_Write(char* data, int numData);
