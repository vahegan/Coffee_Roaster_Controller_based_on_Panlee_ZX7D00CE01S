#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "config.h"
#include "data_structures.h"
#include <Arduino_GFX_Library.h>
#include <driver/ledc.h>

// Function declarations for graphics and touch functions
void setup_backlight();
void draw_initial_screen();
void draw_button(TouchButton* button);
bool button_pressed(TouchButton* button, TouchPoint touch);
void draw_graph_axes();
void draw_roast_data();
void draw_crack_lines();
void draw_charge_line();
void draw_vertical_text(int x, int y, const char* text, uint16_t color);
bool find_gt911();
bool init_touch(uint8_t i2c_addr);
TouchPoint read_touch();
bool init_aw9523();
void aw9523_write(uint8_t port, bool value);

// Display and bus objects
extern Arduino_ESP32RGBPanel *bus;
extern Arduino_RGB_Display *gfx;

#endif // GRAPHICS_H 