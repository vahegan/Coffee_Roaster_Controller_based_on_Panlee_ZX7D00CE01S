#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "config.h"
#include "data_structures.h"

// Function declarations for temperature and ROR functions
void max31850_init();
float max31850_read_temp(bool use_raw = false);
void reset_temp_averaging();
float get_averaged_ror(float current_ror);
float calculate_artisan_ror(uint16_t current_index);

#endif // TEMPERATURE_H 