#include "graphics.h"
#include <Wire.h>
#include <algorithm>

// RGB panel configuration
Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
  39, // DE
  38, // VSYNC
  5,  // HSYNC
  9,  // PCLK
  10, 11, 12, 13, 14,  // R0-R4
  21, 0, 46, 3, 8, 18, // G0-G5
  17, 16, 15, 7, 6,    // B0-B4
  1,  // hsync_polarity (1 = positive)
  40, // hsync_front_porch
  8,  // hsync_pulse_width
  88, // hsync_back_porch
  1,  // vsync_polarity
  23, // vsync_front_porch
  5,  // vsync_pulse_width
  44, // vsync_back_porch
  0,  // pclk_active_neg (0 = active low)
  16000000, // prefer_speed (16 MHz)
  1, // frame buffer count (single buffering)
  USE_RGB565 // RGB888
);

// Display initialization
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
  TFT_WIDTH, TFT_HEIGHT, bus,
  0, // rotation
  false, // auto_flush
  NULL, // bus
  -1, // rst (handled via AW9523)
  NULL, // init_operations
  0 // init_operations_len
);

// Simple 5x7 font bitmap for vertical text (rotated 90 degrees)
const uint8_t font_5x7[][5] = {
  {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
  {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
  {0x00, 0x07, 0x00, 0x07, 0x00}, // "
  {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
  {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
  {0x23, 0x13, 0x08, 0x64, 0x62}, // %
  {0x36, 0x49, 0x55, 0x22, 0x50}, // &
  {0x00, 0x05, 0x03, 0x00, 0x00}, // '
  {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
  {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
  {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
  {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
  {0x00, 0x00, 0xA0, 0x60, 0x00}, // ,
  {0x08, 0x08, 0x08, 0x08, 0x08}, // -
  {0x00, 0x60, 0x60, 0x00, 0x00}, // .
  {0x20, 0x10, 0x08, 0x04, 0x02}, // /
  {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
  {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
  {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
  {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
  {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
  {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
  {0x00, 0x36, 0x36, 0x00, 0x00}, // :
  {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
  {0x08, 0x14, 0x22, 0x41, 0x00}, // <
  {0x14, 0x14, 0x14, 0x14, 0x14}, // =
  {0x00, 0x41, 0x22, 0x14, 0x08}, // >
  {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
  {0x32, 0x49, 0x59, 0x51, 0x3E}, // @
  {0x7C, 0x12, 0x11, 0x12, 0x7C}, // A
  {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
  {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
  {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
  {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
  {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
  {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
  {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
  {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
  {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
  {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
  {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
  {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
  {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
  {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
  {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
  {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
  {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
  {0x46, 0x49, 0x49, 0x49, 0x31}, // S
  {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
  {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
  {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
  {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
  {0x63, 0x14, 0x08, 0x14, 0x63}, // X
  {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
  {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
};

void setup_backlight() {
  ledc_timer_config_t timer_conf = {
    .speed_mode = LEDC_MODE,
    .duty_resolution = LEDC_DUTY_RES,
    .timer_num = LEDC_TIMER,
    .freq_hz = LEDC_FREQUENCY,
    .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&timer_conf);

  ledc_channel_config_t channel_conf = {
    .gpio_num = BL_PWM_PIN,
    .speed_mode = LEDC_MODE,
    .channel = LEDC_CHANNEL,
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER,
    .duty = 255, // 100% brightness
    .hpoint = 0
  };
  ledc_channel_config(&channel_conf);
}

void draw_initial_screen() {
  gfx->fillScreen(BLACK);
  // Title
  gfx->setTextColor(YELLOW_RGB888);
  gfx->setTextSize(2);
      gfx->setCursor(GRAPH_X+GRAPH_TITLE_X_OFFSET, GRAPH_TITLE_Y);
  gfx->print("Coffee Roaster Controller");
  // Draw buttons - only show start button initially, crack button appears after roast starts
  draw_button(&start_button);
  // Note: crack_button is drawn separately when needed
  // Draw graph axes
  draw_graph_axes();
  // Legend
  gfx->setTextSize(2);

  gfx->setTextColor(TEMP_DOT_COLOR);
  gfx->setCursor(TEXT_X, T_TEXT_Y);
  gfx->print("BT");

  gfx->setTextColor(ROR_DOT_COLOR);
  gfx->setCursor(TEXT_X, ROR_TEXT_Y);
  gfx->print("ROR");
  gfx->flush();
}

void draw_crack_button_if_visible() {
  // Only draw crack button if roast is active AND button is marked as visible
  if (roast_active && crack_button.visible) {
    draw_button(&crack_button);
  }
}

void clear_crack_button_area() {
  // Clear the area where the crack button would be to hide it
  gfx->fillRect(crack_button.x, crack_button.y, crack_button.width, crack_button.height, BLACK_RGB888);
}

void draw_button(TouchButton* button) {
  uint16_t color = button->pressed ? YELLOW_RGB888 : WHITE_RGB888;
  gfx->fillRoundRect(button->x, button->y, button->width, button->height, 10, color);
  gfx->drawRoundRect(button->x, button->y, button->width, button->height, 10, BLACK_RGB888);

  gfx->setTextColor(BLACK_RGB888);
  gfx->setTextSize(2);
  int text_size = 2;
  int char_width = 6 * text_size;
  int char_height = 8 * text_size;
  
  // Check if we have three-line text
  if (button->text2 != NULL && button->text3 != NULL) {
    // Three-line text - calculate positioning for all three lines
    int line1_len = strlen(button->text);
    int line2_len = strlen(button->text2);
    int line3_len = strlen(button->text3);
    int max_line_len = max(max(line1_len, line2_len), line3_len);
    int text_width = char_width * max_line_len;
    int total_text_height = char_height * 3;
    
    // Center the entire text block in the button
    int text_x = button->x + (button->width - text_width) / 2;
    int text_y = button->y + (button->height - total_text_height) / 2;
    
    // Draw first line
    gfx->setCursor(text_x, text_y);
    gfx->print(button->text);
    
    // Draw second line
    gfx->setCursor(text_x, text_y + char_height);
    gfx->print(button->text2);
    
    // Draw third line
    gfx->setCursor(text_x, text_y + char_height * 2);
    gfx->print(button->text3);
  } else if (button->text2 != NULL) {
    // Two-line text - calculate positioning for both lines
    int line1_len = strlen(button->text);
    int line2_len = strlen(button->text2);
    int max_line_len = max(line1_len, line2_len);
    int text_width = char_width * max_line_len;
    int total_text_height = char_height * 2;
    
    // Center the entire text block in the button
    int text_x = button->x + (button->width - text_width) / 2;
    int text_y = button->y + (button->height - total_text_height) / 2;
    
    // Draw first line
    gfx->setCursor(text_x, text_y);
    gfx->print(button->text);
    
    // Draw second line below the first
    gfx->setCursor(text_x, text_y + char_height);
    gfx->print(button->text2);
  } else {
    // Single-line text (original behavior)
    int text_len = strlen(button->text);
    int text_width = char_width * text_len;
    int text_height = char_height;
    int text_x = button->x + (button->width - text_width) / 2;
    int text_y = button->y + (button->height - text_height) / 2;
    gfx->setCursor(text_x, text_y);
    gfx->print(button->text);
  }
}

bool button_pressed(TouchButton* button, TouchPoint touch) {
    bool currently_pressed = (touch.x >= button->x && touch.x <= button->x + button->width &&
                              touch.y >= button->y && touch.y <= button->y + button->height);

    // Debug: Log the current state
    static uint32_t last_state_debug = 0;
    if (millis() - last_state_debug > 2000) { // Log every 2 seconds
        Serial.printf("[%lu] Button '%s' state: currently_pressed=%d, was_pressed=%d, touch_valid=%d, touch_pos=(%d,%d), button_bounds=(%d,%d,%d,%d)\n", 
                      millis(), button->text, currently_pressed, button->was_pressed, touch.valid, touch.x, touch.y, 
                      button->x, button->y, button->x + button->width, button->y + button->height);
        last_state_debug = millis();
    }
    
    // Additional debug for CRACK button specifically
    if (strcmp(button->text, "CRACK") == 0 && touch.valid) {
        static uint32_t last_crack_debug = 0;
        if (millis() - last_crack_debug > 1000) { // Log every second for CRACK button
            Serial.printf("[%lu] CRACK button debug: touch=(%d,%d), bounds=(%d,%d,%d,%d), currently_pressed=%d, was_pressed=%d\n", 
                          millis(), touch.x, touch.y, button->x, button->y, button->x + button->width, button->y + button->height, 
                          currently_pressed, button->was_pressed);
            last_crack_debug = millis();
        }
    }

    // Reset button state immediately when no touch is detected
    if (!currently_pressed && button->was_pressed) {
        button->was_pressed = false;
        if (DEBUG_MODE_TOUCH) {
          Serial.printf("[%lu] Reset button '%s' state (no touch detected)\n", millis(), button->text);
        }
    }
    
    // Force reset button state if it's been "pressed" for too long (prevents stuck state)
    if (button->was_pressed && (millis() - button->last_press > 2000)) { // 2 second timeout
        button->was_pressed = false;
        Serial.printf("[%lu] Force reset button '%s' state (timeout)\n", millis(), button->text);
    }

    // Only register a press if the button was not pressed in the previous loop
    bool just_pressed = currently_pressed && !button->was_pressed && (millis() - button->last_press > 120);

    if (just_pressed) {
        button->last_press = millis();
        Serial.printf("[%lu] Button '%s' pressed\n", millis(), button->text);
    }

    // Debug: Log when touch is detected but not registered as press
    if (currently_pressed && !just_pressed) {
        static uint32_t last_debug = 0;
        if (millis() - last_debug > 1000) { // Only log once per second to avoid spam
            if (DEBUG_MODE_TOUCH) {
              Serial.printf("[%lu] Touch detected on '%s' but not registered as press (was_pressed: %d, time_since_last: %lu)\n", 
                          millis(), button->text, button->was_pressed, millis() - button->last_press);
            }
            last_debug = millis();
        }
    }

    // Update state for next loop
    button->was_pressed = currently_pressed;

    return just_pressed;
}

void draw_graph_axes() {
  // Clear graph area
  gfx->fillRect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, WHITE_RGB888);
  gfx->drawRect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, BLACK_RGB888);
  
  // Draw time axis (horizontal) with minute marks and numbers
  int nTicksX = GRAPH_TIME_MAX / 60; // Number of ticks = maximum roast time (s) / (seconds in a minute)
  for (int i = 0; i < nTicksX; i++) {
    int x = GRAPH_X + (i * GRAPH_WIDTH / nTicksX);
    gfx->drawLine(x, GRAPH_Y + GRAPH_HEIGHT, x, GRAPH_Y + GRAPH_HEIGHT - 10, BLACK_RGB888);
    gfx->setTextColor(WHITE_RGB888);
    gfx->setTextSize(2);
          gfx->setCursor(x - GRAPH_TIME_LABEL_X_OFFSET, GRAPH_Y + GRAPH_HEIGHT + GRAPH_TIME_LABEL_Y_OFFSET);
    gfx->print(i);
  }
  
  // Draw half-minute marks (smaller ticks)
  for (int i = 1; i < nTicksX*2; i++) {
    if (i % 2 == 1) { // Only odd numbers (half minutes)
      int x = GRAPH_X + (i * GRAPH_WIDTH / nTicksX*2);
      gfx->drawLine(x, GRAPH_Y + GRAPH_HEIGHT, x, GRAPH_Y + GRAPH_HEIGHT - 5, BLACK_RGB888);
    }
  }
  
  // Draw temperature axis (vertical) with degree marks and numbers
  int nTicksY = GRAPH_TEMP_MAX /20; // Number of ticks = maximum temp / tick increment
  for (int i = 0; i <= nTicksY; i++) {
    int y = GRAPH_Y + GRAPH_HEIGHT - (i * GRAPH_HEIGHT / nTicksY);
    gfx->drawLine(GRAPH_X, y, GRAPH_X + 10, y, TEMP_DOT_COLOR);
    gfx->setTextColor(WHITE_RGB888);
    gfx->setTextSize(2);
    gfx->setCursor(GRAPH_X - GRAPH_TEMP_LABEL_X_OFFSET, y - GRAPH_TEMP_LABEL_Y_OFFSET);
    gfx->print(i * 20);
  }
  // Draw ROR axis (vertical) with marks and numbers
  // ROR axis should be symmetric around zero, centered in the middle of the graph
  int ror_range = ROR_GRAPH_MAX; // Use positive range since we're symmetric around zero
  int nTicksROR = (ror_range * 2) / 10; // 10°C/min spacing, covering -ror_range to +ror_range
  
  for (int i = 0; i <= nTicksROR; i++) {
    int y = GRAPH_Y + GRAPH_HEIGHT - (i * GRAPH_HEIGHT / nTicksROR);
    gfx->drawLine(GRAPH_X+GRAPH_WIDTH-10, y, GRAPH_X+GRAPH_WIDTH, y, ROR_DOT_COLOR);
    gfx->setTextColor(ROR_DOT_COLOR);
    gfx->setTextSize(2);
    gfx->setCursor(GRAPH_X+GRAPH_WIDTH-ROR_AXIS_TEXT_X_OFFSET, y - ROR_AXIS_TEXT_Y_OFFSET);
    
    // Calculate ROR value: start from -ror_range, go to +ror_range
    int ror_value = -ror_range + (i * 10);
    gfx->print(ror_value);
  }
  
  // Draw a prominent zero line
  int zero_y = GRAPH_Y + (GRAPH_HEIGHT / 2);
  gfx->setTextColor(WHITE_RGB888);
  gfx->setTextSize(2);
  gfx->setCursor(GRAPH_X+GRAPH_WIDTH-ROR_AXIS_TEXT_X_OFFSET, zero_y - ROR_AXIS_TEXT_Y_OFFSET);
  gfx->print("0");
  // Draw a longer line for zero
  gfx->drawLine(GRAPH_X+GRAPH_WIDTH-15, zero_y, GRAPH_X+GRAPH_WIDTH, zero_y, WHITE_RGB888);
}

void draw_roast_data() {
  if (data_count < 1) return;
  // Draw bean temperature dots (blue)
  for (int i = 0; i < data_count; i++) {
    int x = GRAPH_X + (roast_data[i].timestamp * GRAPH_WIDTH / GRAPH_TIME_MAX);
    float temp = roast_data[i].bean_temp;
    if (temp > GRAPH_TEMP_MAX) temp = GRAPH_TEMP_MAX;
    if (temp < 0) temp = 0;
    int y = GRAPH_Y + GRAPH_HEIGHT - (int)((temp * (float)GRAPH_HEIGHT) / (float)GRAPH_TEMP_MAX);
    if (x >= GRAPH_X && x <= GRAPH_X + GRAPH_WIDTH && y >= GRAPH_Y && y <= GRAPH_Y + GRAPH_HEIGHT) {
      gfx->fillCircle(x, y, TEMP_DOT_SIZE, TEMP_DOT_COLOR);
    }
  }
  // Draw ROR dots (red, symmetric around zero)
  for (int i = 0; i < data_count; i++) {
    float ror_val = roast_data[i].ror;
    // Clip ROR to symmetric graph range around zero
    int ror_range = ROR_GRAPH_MAX; // Use positive range since we're symmetric around zero
    if (ror_val < -ror_range) ror_val = -ror_range;
    if (ror_val > ror_range) ror_val = ror_range;
  
    int x = GRAPH_X + (roast_data[i].timestamp * GRAPH_WIDTH / GRAPH_TIME_MAX);
    // Center zero line in middle of graph
    int y = GRAPH_Y + (GRAPH_HEIGHT / 2) - (int)((ror_val * (float)GRAPH_HEIGHT) / (2.0 * (float)ror_range));
  
    if (x >= GRAPH_X && x <= GRAPH_X + GRAPH_WIDTH && y >= GRAPH_Y && y <= GRAPH_Y + GRAPH_HEIGHT) {
      gfx->fillCircle(x, y, ROR_DOT_SIZE, ROR_DOT_COLOR);
    }
  }
}

void draw_vertical_text(int x, int y, const char* text, uint16_t color) {
  // Calculate string length
  int len = strlen(text);
  
  // Clear the area first with WHITE background
  gfx->fillRect(x, y, VERTICAL_TEXT_WIDTH, len * FONT_CHAR_SPACING, WHITE_RGB888);
  
  // For each character in the string (in reverse order for proper vertical stacking)
  for (int char_idx = 0; char_idx < len; char_idx++) {
    char c = text[len - 1 - char_idx]; // Reverse the character order
    int char_index = 0;
    if (c >= ' ' && c <= 'Z') {
      char_index = c - ' ';
    }
    // Draw character rotated 90 degrees (counterclockwise)
    // Each row in the font becomes a column in the rotated character
    for (int row = 0; row < VERTICAL_TEXT_WIDTH; row++) {
      uint8_t row_bits = 0;
      // Build the row_bits from the font columns
      for (int col = 0; col < FONT_CHAR_WIDTH; col++) {
        if (font_5x7[char_index][col] & (1 << row)) {
          row_bits |= (1 << (4 - col));
        }
      }
      // Draw the row as a vertical line (rotated)
      for (int col = 0; col < FONT_CHAR_WIDTH; col++) {
        if (row_bits & (1 << col)) {
          gfx->drawPixel(x + row, y + char_idx * FONT_CHAR_SPACING + col, color);
        }
      }
    }
  }
}

void draw_crack_lines() {
  for (int i = 0; i < crack_count; i++) {
    if (crack_times[i] > 0) {
      int x = GRAPH_X + (crack_times[i] * GRAPH_WIDTH / GRAPH_TIME_MAX);
      gfx->drawLine(x, GRAPH_Y, x, GRAPH_Y + GRAPH_HEIGHT, GREEN_RGB888);
      int minutes = crack_times[i] / 60;
      int seconds = crack_times[i] % 60;
      float crack_temp = 0;
      bool found = false;
      for (int j = 0; j < data_count; j++) {
        if (roast_data[j].timestamp >= crack_times[i]) {
          crack_temp = roast_data[j].bean_temp;
          found = true;
          break;
        }
      }
      if (!found && data_count > 0) crack_temp = roast_data[data_count-1].bean_temp;
      char time_str[10];
      sprintf(time_str, "%d:%02d", minutes, seconds);
      char combined_text[40];
      
      // Different labels for each crack event
      switch (i) {
        case 0: // 1st crack start
          sprintf(combined_text, "1stCrackStart%s %.0fC", time_str, crack_temp);
          break;
        case 1: // 1st crack end
          sprintf(combined_text, "1stCrackEnd%s %.0fC", time_str, crack_temp);
          break;
        case 2: // 2nd crack start
          sprintf(combined_text, "2ndCrackStart%s %.0fC", time_str, crack_temp);
          break;
        case 3: // 2nd crack end
          sprintf(combined_text, "2ndCrackEnd%s %.0fC", time_str, crack_temp);
          break;
        default:
          sprintf(combined_text, "Crack%d%s %.0fC", i+1, time_str, crack_temp);
          break;
      }
      
      draw_vertical_text(x + CRACK_LABEL_X_OFFSET, Y_MARK_20 - (strlen(combined_text) * FONT_CHAR_SPACING), combined_text, GREEN_RGB888);
    }
  }
}

void draw_charge_line() {
  int x = GRAPH_X + (charge_time * GRAPH_WIDTH / GRAPH_TIME_MAX);
  gfx->drawLine(x, GRAPH_Y, x, GRAPH_Y + GRAPH_HEIGHT, MAGENTA_RGB888);
  float charge_temp = 0;
  bool found = false;
  for (int i = 0; i < data_count; i++) {
    if (roast_data[i].timestamp >= charge_time) {
      charge_temp = roast_data[i].bean_temp;
      found = true;
      break;
    }
  }
  if (!found && data_count > 0) charge_temp = roast_data[data_count-1].bean_temp;
  char label[40];
  sprintf(label, "CHARGE %d:%02d %.0fC", charge_time/60, charge_time%60, charge_temp);
  draw_vertical_text(x + CHARGE_LABEL_X_OFFSET, Y_MARK_20 - (strlen(label) * FONT_CHAR_SPACING), label, MAGENTA_RGB888);
}

bool find_gt911() {
  Wire.beginTransmission(TOUCH_I2C_ADDR1);
  if (Wire.endTransmission() == 0) {
    touch_i2c_addr = TOUCH_I2C_ADDR1;
    Serial.printf("[%lu] GT911 found at address 0x%02X\n", millis(), touch_i2c_addr);
    return true;
  }
  
  Wire.beginTransmission(TOUCH_I2C_ADDR2);
  if (Wire.endTransmission() == 0) {
    touch_i2c_addr = TOUCH_I2C_ADDR2;
    Serial.printf("[%lu] GT911 found at address 0x%02X\n", millis(), touch_i2c_addr);
    return true;
  }
  
  Serial.printf("[%lu] GT911 not found at either address!\n", millis());
  return false;
}

bool init_touch(uint8_t i2c_addr) {
  Serial.printf("[%lu] Checking GT911 at 0x%02X...\n", millis(), i2c_addr);
  Wire.beginTransmission(i2c_addr);
  if (Wire.endTransmission() != 0) {
    Serial.printf("[%lu] GT911 not found at 0x%02X!\n", millis(), i2c_addr);
    return false;
  }
  
  // Reset sequence
  aw9523_write(AW9523_P11, 0); // TP_RST low
  delay(10);
  aw9523_write(AW9523_P11, 1); // TP_RST high
  delay(150);
  
  return true;
}

TouchPoint read_touch() {
  TouchPoint touch = {0, 0, 0, 0, false};
  
  if (touch_i2c_addr == 0) {
    return touch;
  }
  
  // Read status register
  Wire.beginTransmission(touch_i2c_addr);
  Wire.write(0x81); Wire.write(0x4E);
  if (Wire.endTransmission() != 0) {
    return touch;
  }
  
  Wire.requestFrom(touch_i2c_addr, 1);
  if (Wire.available() < 1) {
    return touch;
  }
  
  uint8_t status = Wire.read();
  uint8_t touch_count = status & 0x0F;
  bool buffer_ready = (status & 0x80) != 0;
  
  // Clear status register
  if (buffer_ready || touch_count > 0) {
    Wire.beginTransmission(touch_i2c_addr);
    Wire.write(0x81); Wire.write(0x4E);
    Wire.write(0x00);
    Wire.endTransmission();
  }
  
  if (touch_count == 0) {
    // Debug: Log when no touch is detected
    static uint32_t last_no_touch_debug = 0;
    if (millis() - last_no_touch_debug > 2000) { // Log every 2 seconds to avoid spam
      if (DEBUG_MODE_TOUCH) {
        Serial.printf("[%lu] No touch detected (touch_count: %d)\n", millis(), touch_count);
      } 
      last_no_touch_debug = millis();
    }
    return touch;
  }
  
  // Read coordinates and pressure
  Wire.beginTransmission(touch_i2c_addr);
  Wire.write(0x81); Wire.write(0x50);
  if (Wire.endTransmission() != 0) {
    return touch;
  }
  
  Wire.requestFrom(touch_i2c_addr, 6);
  if (Wire.available() < 6) {
    return touch;
  }
  
  uint8_t xl = Wire.read();
  uint8_t xh = Wire.read();
  uint8_t yl = Wire.read();
  uint8_t yh = Wire.read();
  uint8_t pressure_l = Wire.read();
  uint8_t pressure_h = Wire.read();
  
  uint16_t raw_x = ((xh & 0x0F) << 8) | xl;
  uint16_t raw_y = ((yh & 0x0F) << 8) | yl;
  
  if (raw_x < TFT_WIDTH && raw_y < TFT_HEIGHT) {
    touch.x = raw_x;
    touch.y = raw_y;
    touch.id = 0;
    touch.weight = pressure_l;
    touch.valid = true;
    
    // Debug: Log touch detection
    static uint32_t last_touch_debug = 0;
    if (millis() - last_touch_debug > 500) { // Log every 500ms to avoid spam
      if (DEBUG_MODE_TOUCH) {
        Serial.printf("[%lu] Touch detected: (%d,%d) weight:%d\n", millis(), touch.x, touch.y, touch.weight);
      }
      last_touch_debug = millis();
    }
  }
  return touch;
}

bool init_aw9523() {
  Wire.begin(48, 47, 100000); // SDA, SCL, 100 kHz
  delay(1000);
  
  Serial.printf("[%lu] Checking AW9523 at 0x%02X...\n", millis(), AW9523_ADDR);
  Wire.beginTransmission(AW9523_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.printf("[%lu] AW9523 not found at 0x%02X!\n", millis(), AW9523_ADDR);
    return false;
  }
  
  // Configure P0 for 1-Wire (DQ as output, VDD and PULLUP as outputs)
  Wire.beginTransmission(AW9523_ADDR);
  Wire.write(AW9523_REG_CONFIG_P0);
  Wire.write(0xF8); // P00-P02 as outputs, P03-P07 as inputs
  if (Wire.endTransmission() != 0) {
    Serial.printf("[%lu] AW9523 P0 config failed!\n", millis());
    return false;
  }
  
  // Configure P1 for LCD and touch reset
  Wire.beginTransmission(AW9523_ADDR);
  Wire.write(AW9523_REG_CONFIG_P1);
  Wire.write(0xFC); // P10, P11 as output
  if (Wire.endTransmission() != 0) {
    Serial.printf("[%lu] AW9523 P1 config failed!\n", millis());
    return false;
  }
  
  // Set initial states
  Wire.beginTransmission(AW9523_ADDR);
  Wire.write(AW9523_REG_OUTPUT_P0);
  Wire.write(0x07); // DQ=1, VDD=1, PULLUP=1 (inactive)
  if (Wire.endTransmission() != 0) {
    Serial.printf("[%lu] AW9523 P0 write failed!\n", millis());
    return false;
  }
  
  Wire.beginTransmission(AW9523_ADDR);
  Wire.write(AW9523_REG_OUTPUT_P1);
  Wire.write(0x03); // P10, P11 high
  if (Wire.endTransmission() != 0) {
    Serial.printf("[%lu] AW9523 P1 write failed!\n", millis());
    return false;
  }
  
  return true;
}

void aw9523_write(uint8_t port, bool value) {
  static uint8_t p0_state = 0x07;
  static uint8_t p1_state = 0x03;
  
  if (port <= 7) { // P0 port
    bitWrite(p0_state, port, value);
    Wire.beginTransmission(AW9523_ADDR);
    Wire.write(AW9523_REG_OUTPUT_P0);
    Wire.write(p0_state);
    Wire.endTransmission();
  } else if (port >= 10 && port <= 11) { // P1 port
    bitWrite(p1_state, port - 10, value);
    Wire.beginTransmission(AW9523_ADDR);
    Wire.write(AW9523_REG_OUTPUT_P1);
    Wire.write(p1_state);
    Wire.endTransmission();
  }
} 