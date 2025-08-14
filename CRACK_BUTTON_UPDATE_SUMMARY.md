# Crack Button Update Summary

## Overview
The crack button has been updated to implement a multi-stage behavior that tracks both the start and end of 1st and 2nd crack events, with corresponding visual indicators and CSV logging.

## Changes Made

### 1. Data Structures (`data_structures.h`)
- Added new `CrackStage` enum with values:
  - `CRACK_IDLE` - Initial state
  - `FIRST_CRACK_START` - After charge, ready for 1st crack start
  - `FIRST_CRACK_END` - After 1st crack start, ready for 1st crack end
  - `SECOND_CRACK_START` - After 1st crack end, ready for 2nd crack start
  - `SECOND_CRACK_END` - After 2nd crack start, ready for 2nd crack end
- Expanded `crack_times` array from 2 to 4 elements to store all crack events
- Added `current_crack_stage` variable to track the current stage

### 2. Global Variables (`globals.cpp`)
- Updated `crack_times` array initialization to 4 elements
- Added `current_crack_stage` initialization to `CRACK_IDLE`

### 3. Main Logic (`Coffee_Roaster_Controller_Multifile.ino`)
- Updated roast start to initialize all 4 crack time slots and reset crack stage
- Modified charge button logic to set initial crack button text to "1st Crack\nStart"
- Completely rewrote crack button logic to implement state machine:
  - **1st Crack Start**: Records time, changes text to "1st Crack\nEnd"
  - **1st Crack End**: Records time, changes text to "2nd Crack\nStart"
  - **2nd Crack Start**: Records time, changes text to "2nd Crack\nEnd"
  - **2nd Crack End**: Records time, resets text to "CRACK"
- Each stage records the event time and updates the button text accordingly

### 4. Graphics (`graphics.cpp`)
- Updated `draw_crack_lines()` function to handle 4 crack events with proper labels:
  - Event 0: "1stCrackStart" 
  - Event 1: "1stCrackEnd"
  - Event 2: "2ndCrackStart"
  - Event 3: "2ndCrackEnd"
- Enhanced `draw_button()` function to support multi-line text using `\n` separator
- Added support for proper text centering and positioning for two-line button text

### 5. CSV Export (`webserver.cpp`)
- Updated CSV header to include all 4 crack events:
  - `FCs` (1st Crack Start)
  - `FCe` (1st Crack End) 
  - `SCs` (2nd Crack Start)
  - `SCe` (2nd Crack End)
- Modified event detection to properly label all crack events in the data rows

## Button Text Progression
1. **Initial**: "CRACK"
2. **After Charge**: "1st Crack\nStart"
3. **After 1st Click**: "1st Crack\nEnd" 
4. **After 2nd Click**: "2nd Crack\nStart"
5. **After 3rd Click**: "2nd Crack\nEnd"
6. **After 4th Click**: "CRACK" (returns to initial state)

## Visual Indicators
- Each crack event draws a vertical green line on the graph
- Lines are labeled with event type, time, and temperature
- All 4 events can be displayed simultaneously

## CSV Logging
- All crack events are recorded with timestamps
- Events are properly labeled in the CSV output
- Maintains compatibility with existing Artisan-style CSV format

## Technical Notes
- Uses state machine pattern for clean, maintainable code
- Button text supports newline characters for multi-line display
- All existing functionality preserved while adding new features
- Memory usage increased slightly due to larger crack_times array
