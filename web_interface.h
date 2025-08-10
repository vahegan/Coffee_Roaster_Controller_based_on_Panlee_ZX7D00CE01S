#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "config.h"
#include "data_structures.h"

// Function declarations for web server and file operations
void setup_webserver();
String getRoastFilename();
void saveRoastProfile();

// External web server object
extern WebServer server;

#endif // WEB_INTERFACE_H