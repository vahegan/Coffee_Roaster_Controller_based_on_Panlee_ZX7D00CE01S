#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include <driver/ledc.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>

// Include our custom header files
#include "config.h"
#include "data_structures.h"
#include "temperature.h"
#include "graphics.h"
#include "web_interface.h"

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.printf("[%lu] Coffee Roaster Controller\n", millis());

  // Initialize display and backlight FIRST
  Serial.println("Initializing display and backlight...");
  setup_backlight();
  if (!gfx->begin()) {
    Serial.printf("[%lu] Display initialization failed!\n", millis());
    while (1);
  }
  Serial.printf("[%lu] Display initialized successfully\n", millis());

  // Setup web server (includes WiFi, SPIFFS, NTP)
  setup_webserver();

  // Continue with the rest of your setup (AW9523, MAX31850, touch, etc.)
  // Initialize AW9523
  if (!init_aw9523()) {
    Serial.printf("[%lu] AW9523 initialization failed!\n", millis());
    while (1);
  }
  Serial.printf("[%lu] AW9523 initialized\n", millis());

  // Initialize MAX31850 1-Wire
  max31850_init();
  reset_temp_averaging();
  Serial.printf("[%lu] MAX31850 1-Wire initialized\n", millis());

  // Reset display and touch
  aw9523_write(AW9523_P10, 0);
  aw9523_write(AW9523_P11, 0);
  delay(2);
  aw9523_write(AW9523_P10, 1);
  aw9523_write(AW9523_P11, 1);
  delay(120);

  // Initialize touch
  if (find_gt911()) {
    if (init_touch(touch_i2c_addr)) {
      Serial.printf("[%lu] Touch initialized successfully\n", millis());
    } else {
      Serial.printf("[%lu] Touch initialization failed!\n", millis());
      touch_i2c_addr = 0;
    }
  } else {
    Serial.printf("[%lu] No touch controller found!\n", millis());
  }

  // Draw initial screen
  draw_initial_screen();
  roast_stage = IDLE;
  charge_time = 0;
}

void loop() {
  server.handleClient(); // Process incoming HTTP requests
  // Read touch
  TouchPoint touch = read_touch();
  
  // Handle button presses
  if (touch.valid) {
    if (button_pressed(&start_button, touch)) {
      if (!roast_active && roast_stage == IDLE) {
        // Start roast (preheat phase)
        roast_active = true;
        roast_start_time = millis();
        data_count = 0;
        crack_count = 0;
        crack_times[0] = 0;
        crack_times[1] = 0;
        reset_temp_averaging();
        charge_time = 0;
        start_button.text = (char*)"STOP";
        crack_button.text = (char*)"CHARGE";
        roast_stage = PREHEAT;
        draw_initial_screen();
        Serial.printf("[%lu] Roast started (preheat)\n", millis());
      } else {
        // Stop roast
        roast_active = false;
        roast_stage = IDLE;
        start_button.text = (char*)"START";
        crack_button.text = (char*)"CRACK";
        draw_button(&start_button);
        draw_button(&crack_button);
        draw_crack_lines();
        gfx->flush();
        saveRoastProfile();
        Serial.printf("[%lu] Roast stopped\n", millis());
      }
    }
    if (button_pressed(&crack_button, touch)) {
      if (roast_active && roast_stage == PREHEAT) {
        // Charge pressed
        charge_time = (millis() - roast_start_time) / 1000;
        draw_graph_axes();
        draw_roast_data();
        draw_charge_line();
        draw_crack_lines();
        gfx->flush();
        crack_button.text = (char*)"CRACK";
        roast_stage = ROASTING;
        draw_button(&crack_button);
        Serial.printf("[%lu] Charge pressed at %d seconds\n", millis(), charge_time);
      } else if (roast_active && roast_stage == ROASTING && crack_count < 2) {
        uint32_t current_time = (millis() - roast_start_time) / 1000;
        crack_times[crack_count] = current_time;
        crack_count++;
        Serial.printf("[%lu] Crack %d detected at %d seconds (crack_times[%d] = %d)\n", 
                      millis(), crack_count, current_time, crack_count-1, crack_times[crack_count-1]);
        draw_graph_axes();
        draw_roast_data();
        draw_charge_line();
        draw_crack_lines();
        gfx->flush();
      } else if (!roast_active) {
        Serial.printf("[%lu] Crack/Charge button pressed but roast not active\n", millis());
      } else if (crack_count >= 2) {
        Serial.printf("[%lu] Already detected 2 cracks (max reached)\n", millis());
      } else {
        Serial.printf("[%lu] Crack/Charge button pressed but condition not met - roast_active: %d, crack_count: %d\n", millis(), roast_active, crack_count);
      }
    }
  }
  // High-frequency temperature reading (every 200ms for responsiveness)
  // This provides 5x faster temperature updates while maintaining 1-second CSV output
  if (millis() - last_data_time >= TEMP_READING_INTERVAL) {
    float temp = max31850_read_temp(false); // Get smoothed temp for display and logging
    last_data_time = millis();
    
    // Update display with current temperature (real-time)
    gfx->setTextColor(TEMP_DOT_COLOR);
    gfx->setTextSize(2);
    gfx->fillRect(TEXT_X+DISPLAY_TEMP_X_OFFSET, T_TEXT_Y, DISPLAY_TEMP_WIDTH, DISPLAY_TEMP_HEIGHT, BLACK_RGB888);
    gfx->setCursor(TEXT_X+DISPLAY_TEMP_X_OFFSET, T_TEXT_Y);
    gfx->printf("%.0fC", temp);
    
    // Update graph and save data only every second (for CSV compatibility)
    static uint32_t last_csv_time = 0;
    if (millis() - last_csv_time >= CSV_OUTPUT_INTERVAL) {
      last_csv_time = millis();
      
      // Update date/time display
      struct tm timeinfo;
      getLocalTime(&timeinfo);
      gfx->flush();
      gfx->setTextColor(YELLOW_RGB888);
      gfx->setCursor(DISPLAY_DATE_X, DISPLAY_DATE_Y);
      gfx->fillRect(DISPLAY_DATE_X, DISPLAY_DATE_Y, DISPLAY_DATE_WIDTH, DISPLAY_DATE_HEIGHT, BLACK_RGB888);
      gfx->printf("%02d.%02d.%d %02d:%02d",timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min);
      
      // Update graph and save data
      if (roast_active) { 
        if (temp > -900) { // Valid reading
          uint32_t current_time = (millis() - roast_start_time) / 1000;
          if (data_count < MAX_DATA_POINTS) {
            roast_data[data_count].timestamp = current_time;
            roast_data[data_count].bean_temp = temp; // Store smoothed temp for ROR calculation
            float ror = calculate_artisan_ror(data_count);
            roast_data[data_count].ror = ror;
            data_count++;
            draw_roast_data();
            if (charge_time > 0) draw_charge_line();
            draw_crack_lines();
            if (data_count > 1) {
              float display_ror = roast_data[data_count-1].ror;
              gfx->setTextColor(ROR_DOT_COLOR);
              gfx->fillRect(TEXT_X+DISPLAY_ROR_X_OFFSET, ROR_TEXT_Y, DISPLAY_ROR_WIDTH, DISPLAY_ROR_HEIGHT, BLACK_RGB888);
              gfx->setCursor(TEXT_X+DISPLAY_ROR_X_OFFSET, ROR_TEXT_Y);
              gfx->printf("%.0f", display_ror);
            }
            gfx->setTextColor(WHITE_RGB888);
            gfx->fillRect(TEXT_X, TIME_TEXT_Y, DISPLAY_TIME_WIDTH, DISPLAY_TIME_HEIGHT, BLACK_RGB888);
            gfx->setCursor(TEXT_X, TIME_TEXT_Y);
            gfx->printf("%02d:%02d", current_time / 60, current_time % 60);
            gfx->flush();
            Serial.printf("[%lu] Time: %d:%02d, Temp: %.1fC, ROR: %.1fC/min\n", 
                          millis(), current_time / 60, current_time % 60, temp, roast_data[data_count-1].ror);
          }
        }
      }
    }
  }
  delay(50); // Keep 50ms delay for touch responsiveness
} 