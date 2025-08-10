#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include "config.h"
#include <Arduino_GFX_Library.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Touch data structure
struct TouchPoint {
  uint16_t x;
  uint16_t y;
  uint8_t id;
  uint8_t weight;
  bool valid;
};

// Roast data structure
struct RoastData {
  uint32_t timestamp;
  float bean_temp;
  float ror;
};

// Button structure
struct TouchButton {
  uint16_t x, y, width, height;
  char* text;
  bool pressed;
  uint32_t last_press;
  bool was_pressed; // Track previous state for debouncing
};

// Roast stage enumeration
enum RoastStage { IDLE, PREHEAT, CHARGE, ROASTING };

// External declarations for global variables
extern uint8_t touch_i2c_addr;
extern TouchPoint last_touch;
extern bool roast_active;
extern uint32_t roast_start_time;
extern uint32_t last_data_time;
extern uint8_t crack_count;
extern uint32_t crack_times[2];
extern RoastStage roast_stage;
extern uint32_t charge_time;
extern RoastData roast_data[MAX_DATA_POINTS];
extern uint16_t data_count;
extern float temp_readings[TEMP_AVERAGE_SIZE];
extern uint8_t temp_index;
extern bool temp_buffer_filled;
extern float ror_readings[ROR_AVERAGE_SIZE];
extern uint8_t ror_index;
extern bool ror_buffer_filled;
extern float ror_smooth_buffer[ROR_SMOOTH_SIZE];
extern uint8_t ror_smooth_index;
extern bool ror_smooth_filled;
extern TouchButton start_button;
extern TouchButton crack_button;
extern OneWire oneWire;
extern DallasTemperature sensors;
extern Arduino_RGB_Display *gfx;

#endif // DATA_STRUCTURES_H 