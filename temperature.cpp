#include "temperature.h"
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// MAX31850 using DallasTemperature library
OneWire oneWire(MAX31850_DQ_PIN);
DallasTemperature sensors(&oneWire);

void max31850_init() {
  Serial.printf("[%lu] Initializing MAX31850 on GPIO_%d\n", millis(), MAX31850_DQ_PIN);
  
  sensors.begin();
  
  // Search for devices
  Serial.printf("[%lu] Searching for 1-Wire devices...\n", millis());
  sensors.begin();
  int deviceCount = sensors.getDeviceCount();
  Serial.printf("[%lu] Found %d 1-Wire device(s)\n", millis(), deviceCount);
  
  if (deviceCount == 0) {
    Serial.printf("[%lu] No MAX31850 found!\n", millis());
  } else {
    Serial.printf("[%lu] MAX31850 found and initialized\n", millis());
    
    // Print device addresses
    for (int i = 0; i < deviceCount; i++) {
      DeviceAddress tempAddress;
      sensors.getAddress(tempAddress, i);
      Serial.printf("[%lu] Device %d address: ", millis(), i);
      for (uint8_t j = 0; j < 8; j++) {
        Serial.printf("%02X", tempAddress[j]);
        if (j < 7) Serial.print(":");
      }
      Serial.println();
    }
  }
}

float max31850_read_temp(bool use_raw) {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  // Check for errors
  if (temp == DEVICE_DISCONNECTED_C) {
    Serial.println("MAX31850: Device disconnected");
    return -999.0;
  } else {
    temp = temp + TEMP_OFFSET; // Apply the temperature offset
  }
  
  // Add to averaging buffer
  temp_readings[temp_index] = temp;
  temp_index = (temp_index + 1) % TEMP_AVERAGE_SIZE;
  if (temp_index == 0) {
    temp_buffer_filled = true;
  }
  
  // Calculate average
  float avg_temp = 0;
  uint8_t count = temp_buffer_filled ? TEMP_AVERAGE_SIZE : temp_index;
  
  for (uint8_t i = 0; i < count; i++) {
    avg_temp += temp_readings[i];
  }
  avg_temp /= count;
  
  Serial.printf("MAX31850: raw=%.2f°C, avg=%.2f°C (from %d readings)\n", temp, avg_temp, count);
  if (use_raw) {
    return temp;
  } else {
    return avg_temp;
  }
}

void reset_temp_averaging() {
  temp_index = 0;
  temp_buffer_filled = false;
  for (uint8_t i = 0; i < TEMP_AVERAGE_SIZE; i++) {
    temp_readings[i] = 0;
  }
  // Also reset ROR averaging
  ror_index = 0;
  ror_buffer_filled = false;
  for (uint8_t i = 0; i < ROR_AVERAGE_SIZE; i++) {
    ror_readings[i] = 0;
  }
  // Reset ROR smoothing buffer
  ror_smooth_index = 0;
  ror_smooth_filled = false;
  for (uint8_t i = 0; i < ROR_SMOOTH_SIZE; i++) {
    ror_smooth_buffer[i] = 0;
  }
  Serial.printf("[%lu] Temperature and ROR averaging buffers reset\n", millis());
}

float get_averaged_ror(float current_ror) {
  // Add to ROR averaging buffer
  ror_readings[ror_index] = current_ror;
  ror_index = (ror_index + 1) % ROR_AVERAGE_SIZE;
  if (ror_index == 0) {
    ror_buffer_filled = true;
  }
  
  // Calculate average
  float avg_ror = 0;
  uint8_t count = ror_buffer_filled ? ROR_AVERAGE_SIZE : ror_index;
  
  for (uint8_t i = 0; i < count; i++) {
    avg_ror += ror_readings[i];
  }
  avg_ror /= count;
  
  return avg_ror;
}

float calculate_artisan_ror(uint16_t current_index) {
  if (data_count < 2) return 0;
  uint32_t t_now = roast_data[current_index].timestamp;
  float temp_now = roast_data[current_index].bean_temp;
  // Find the index N seconds ago (or the oldest available)
  int idx_past = current_index;
  while (idx_past > 0 && (t_now - roast_data[idx_past].timestamp) < ROR_WINDOW_SEC) {
    idx_past--;
  }
  if (idx_past == current_index) {
    // Not enough time difference, fallback to previous point
    if (current_index > 0) idx_past = current_index - 1;
    else return 0;
  }
  float temp_past = roast_data[idx_past].bean_temp;
  float dt_min = (t_now - roast_data[idx_past].timestamp) / 60.0;
  if (dt_min <= 0) return 0;
  float ror = (temp_now - temp_past) / dt_min;
  // Add to smoothing buffer
  ror_smooth_buffer[ror_smooth_index] = ror;
  ror_smooth_index = (ror_smooth_index + 1) % ROR_SMOOTH_SIZE;
  if (ror_smooth_index == 0) ror_smooth_filled = true;
  // Calculate smoothed ROR
  float ror_sum = 0;
  uint8_t count = ror_smooth_filled ? ROR_SMOOTH_SIZE : ror_smooth_index;
  for (uint8_t i = 0; i < count; i++) {
    ror_sum += ror_smooth_buffer[i];
  }
  float ror_smoothed = ror_sum / count;
  
  // Clamp ROR to meaningful range for roasting control
  if (ror_smoothed > ROR_DISPLAY_MAX) {
    ror_smoothed = ROR_DISPLAY_MAX;
  } else if (ror_smoothed < -ROR_DISPLAY_MAX) {
    ror_smoothed = -ROR_DISPLAY_MAX;
  }
  
  // Additional filtering for initial charge spike (first 30 seconds)
  if (current_index < 30 && ror_smoothed > 50) {
    // During initial charge, limit ROR to prevent extreme spikes
    ror_smoothed = 50;
  }
  
  return ror_smoothed;
} 