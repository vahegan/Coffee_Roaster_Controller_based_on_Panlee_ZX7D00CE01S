#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "data_structures.h"
#include "temperature.h"
#include "graphics.h"
#include "web_interface.h"

// Global variables
uint8_t touch_i2c_addr = 0;
TouchPoint last_touch = {0, 0, 0, 0, false};
bool roast_active = false;
uint32_t roast_start_time = 0;
uint32_t last_data_time = 0;
uint8_t crack_count = 0;
uint32_t crack_times[2] = {0, 0};
RoastStage roast_stage = IDLE;
uint32_t charge_time = 0;
RoastData roast_data[MAX_DATA_POINTS];
uint16_t data_count = 0;
float temp_readings[TEMP_AVERAGE_SIZE];
uint8_t temp_index = 0;
bool temp_buffer_filled = false;
float ror_readings[ROR_AVERAGE_SIZE];
uint8_t ror_index = 0;
bool ror_buffer_filled = false;
float ror_smooth_buffer[ROR_SMOOTH_SIZE];
uint8_t ror_smooth_index = 0;
bool ror_smooth_filled = false;

// Touch buttons
TouchButton start_button = {BUTTON_MARGIN_LEFT, START_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, (char*)"START", false, 0, false};
TouchButton crack_button = {BUTTON_MARGIN_LEFT, CRACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, (char*)"CRACK", false, 0, false}; 