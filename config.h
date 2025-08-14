#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials - Load from secrets.h if available
#include "secrets.h"

// Fallback values for when secrets.h is missing (for other developers)
#ifndef WIFI_SSID
#define WIFI_SSID "YourWiFiName"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YourWiFiPassword"
#endif

#ifndef TIME_ZONE
#define TIME_ZONE 4
#endif

#define TEMP_OFFSET 0 // Temperature calibration offset

#define DEBUG_MODE_TOUCH 0

// Display dimensions
#define TFT_WIDTH  800
#define TFT_HEIGHT 480

// Button configuration
#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT 80
#define BUTTON_MARGIN_LEFT 10
#define BUTTON_MARGIN_BOTTOM 20
#define BUTTON_SPACING 20

// Positions of current temp, ROR, and TIME
#define TEXT_X 5
#define TIME_TEXT_Y 105
#define T_TEXT_Y 45
#define ROR_TEXT_Y 75

// Display text positioning
#define DISPLAY_DATE_X 5
#define DISPLAY_DATE_Y 15
#define DISPLAY_TEMP_X_OFFSET 45
#define DISPLAY_ROR_X_OFFSET 45

// Display element dimensions
#define DISPLAY_DATE_WIDTH 200
#define DISPLAY_DATE_HEIGHT 25
#define DISPLAY_TEMP_WIDTH 55
#define DISPLAY_TEMP_HEIGHT 30
#define DISPLAY_ROR_WIDTH 55
#define DISPLAY_ROR_HEIGHT 30
#define DISPLAY_TIME_WIDTH 100
#define DISPLAY_TIME_HEIGHT 30

// Font and text dimensions
#define FONT_CHAR_WIDTH 5
#define FONT_CHAR_HEIGHT 7
#define FONT_CHAR_SPACING 8
#define VERTICAL_TEXT_WIDTH 7

// Graph label positioning
#define CRACK_LABEL_X_OFFSET 5
#define CHARGE_LABEL_X_OFFSET 5

// Human-friendly RGB888 color definitions
#define BLUE_RGB888    0x00FF00 // 0xFFFF00
#define YELLOW_RGB888  0x0000FF // 0xFF00FF 0x8000FF 
#define BLACK_RGB888   0xFF0000 // 0x000000
#define WHITE_RGB888   0x00FFFF 
#define GREEN_RGB888   0x000F0F
#define MAGENTA_RGB888 0x00F0F0 // 0x00FFF0
#define CYAN_RGB888    0x00FF0F 
#define PINK_RGB888    0x00F0FA

// Graph dot configuration
#define TEMP_DOT_COLOR GREEN_RGB888
#define TEMP_DOT_SIZE 2
#define ROR_DOT_COLOR PINK_RGB888
#define ROR_DOT_SIZE 2

// Button positions (calculated from bottom)
#define START_BUTTON_Y (TFT_HEIGHT - 2*BUTTON_HEIGHT - BUTTON_SPACING - BUTTON_MARGIN_BOTTOM)
#define CRACK_BUTTON_Y (TFT_HEIGHT - BUTTON_HEIGHT - BUTTON_MARGIN_BOTTOM)

// Graph configuration
#define GRAPH_MARGIN_LEFT 150
#define GRAPH_MARGIN_TOP 50
#define GRAPH_MARGIN_RIGHT 5
#define GRAPH_MARGIN_BOTTOM 45  // Space for axis labels and time marks

// Graph dimensions (calculated from margins)
#define GRAPH_WIDTH (TFT_WIDTH - GRAPH_MARGIN_LEFT - GRAPH_MARGIN_RIGHT)
#define GRAPH_HEIGHT (TFT_HEIGHT - GRAPH_MARGIN_TOP - GRAPH_MARGIN_BOTTOM)

// Graph position
#define GRAPH_X GRAPH_MARGIN_LEFT
#define GRAPH_Y GRAPH_MARGIN_TOP

// ROR axis text positioning
#define ROR_AXIS_TEXT_X_OFFSET 45  // X offset for ROR axis text labels
#define ROR_AXIS_TEXT_Y_OFFSET 8   // Y offset for text centering (half text height)

// Graph text positioning
#define GRAPH_TITLE_X_OFFSET 150   // X offset for graph title
#define GRAPH_TITLE_Y 15           // Y position for graph title
#define GRAPH_TIME_LABEL_X_OFFSET 8    // X offset for time axis labels
#define GRAPH_TIME_LABEL_Y_OFFSET 10   // Y offset for time axis labels
#define GRAPH_TEMP_LABEL_X_OFFSET 40   // X offset for temperature axis labels
#define GRAPH_TEMP_LABEL_Y_OFFSET 8    // Y offset for temperature axis labels

// Graph data limits
#define GRAPH_TIME_MAX 1200 // 20 minutes in seconds
#define GRAPH_TEMP_MAX 240 // 240°C

// Calculate the Y position for the 20°C mark
#define Y_MARK_20 (GRAPH_Y + GRAPH_HEIGHT - (GRAPH_HEIGHT / 11))

// Color mode
#define USE_RGB565 false // RGB888 for native format

// Backlight control
#define BL_PWM_PIN 45
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY 5000

// AW9523 IO Expander
#define AW9523_ADDR 0x5B
#define AW9523_REG_CONFIG_P0 0x04 // P0 port configuration
#define AW9523_REG_CONFIG_P1 0x05 // P1 port configuration
#define AW9523_REG_OUTPUT_P0 0x02 // P0 output register
#define AW9523_REG_OUTPUT_P1 0x03 // P1 output register
#define AW9523_P10 0x01 // P10 bit in P1 (LCD_RST)
#define AW9523_P11 0x02 // P11 bit in P1 (TP_RST)

// MAX31850 using DallasTemperature library
#define MAX31850_DQ_PIN 2  // GPIO_02 for 1-Wire data line

// GT911 Touch controller
#define TOUCH_I2C_ADDR1 0x5D // Primary address (ADDR pin low)
#define TOUCH_I2C_ADDR2 0x14 // Secondary address (ADDR pin high)

// Data arrays for graphing
#define MAX_DATA_POINTS 900 // 15 minutes * 60 seconds

// Temperature reading and averaging
#define TEMP_READING_INTERVAL 200    // 200ms = 5 readings/second
#define TEMP_AVERAGE_SIZE 25         // 5 readings/sec × 5 seconds = 25 samples
#define CSV_OUTPUT_INTERVAL 1000     // 1 second CSV output (maintains compatibility)

// ROR averaging for display stability
#define ROR_AVERAGE_SIZE 3 // Number of ROR values to average for display

// ROR Calculation Parameters (Artisan-style)
#define ROR_WINDOW_SEC 30 // 30-second window for ROR calculation
#define ROR_SMOOTH_SIZE 5 // Moving average window for ROR smoothing
#define ROR_GRAPH_MIN -10
#define ROR_GRAPH_MAX 100  // Cap at 100°C/min for meaningful roasting control
#define ROR_DISPLAY_MAX 100 // Maximum ROR value to display/store

#endif // CONFIG_H 