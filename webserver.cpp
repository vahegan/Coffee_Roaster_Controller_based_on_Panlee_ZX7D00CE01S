
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "web_interface.h"

// Web server on port 80
WebServer server(80);

void setup_webserver() {
  // Initialize SPIFFS
  Serial.println("Mounting SPIFFS...");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    while (1);
  }
  Serial.println("SPIFFS mounted.");

  // Connect to WiFi (with timeout)
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long wifi_start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifi_start < 10000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi connect timeout. Continuing without network.");
  }

  // Set up time (for timestamps, with timeout)
  Serial.println("Syncing NTP time...");
  configTime(TIME_ZONE*3600, 0, "pool.ntp.org");
  struct tm timeinfo;
  unsigned long ntp_start = millis();
  while (!getLocalTime(&timeinfo) && millis() - ntp_start < 10000) {
    Serial.println("Waiting for NTP time sync...");
    delay(1000);
  }
  if (!getLocalTime(&timeinfo)) {
    Serial.println("NTP sync failed. Timestamps may be invalid.");
  } else {
    Serial.println("NTP time synced.");
  }

  // Set up web server endpoints
  Serial.println("Starting web server...");
  server.on("/", HTTP_GET, [&]() {
    String html = "<h1>Roast Profiles</h1><ul>";
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
      String fname = String(file.name());
      if (!fname.startsWith("/")) fname = "/" + fname;
       html += "<li>" + fname + " <a href=\"/download?file=" + fname + "\">Download</a> <a href=\"/delete?file=" + fname + "\" onclick=\"return confirm('Delete?');\">Delete</a></li>";
      file = root.openNextFile();
    }
    html += "</ul>";
    server.send(200, "text/html", html);
  });
  server.on("/download", HTTP_GET, [&]() {
    if (!server.hasArg("file")) { server.send(400, "text/plain", "Missing file param"); return; }
    String fname = server.arg("file");
    File file = SPIFFS.open(fname, FILE_READ);
    if (!file) { server.send(404, "text/plain", "File not found"); return; }
    server.sendHeader("Content-Disposition", "attachment; filename=\"" + String(fname.substring(1)) + "\"");
    server.streamFile(file, "text/csv");
    file.close();
  });
  server.on("/delete", HTTP_GET, [&]() {
    if (!server.hasArg("file")) { server.send(400, "text/plain", "Missing file param"); return; }
    String fname = server.arg("file");
    if (!fname.startsWith("/")) fname = "/" + fname;
    Serial.print("Requested file: ");
    Serial.println(fname);
    Serial.print("Exists: ");
    Serial.println(SPIFFS.exists(fname));
    if (SPIFFS.remove(fname)) {
      server.sendHeader("Location", "/");
      server.send(303);
    } else {
      server.send(500, "text/plain", "Delete failed");
    }
  });
  server.on("/test", HTTP_GET, [&]() {
    server.send(200, "text/plain", "Web server is working!");
  });
  Serial.println("Test page available at /test");
  server.begin();
  
  Serial.println("Web server started.");
}

String getRoastFilename() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "/roast_unknown.csv";
  }
  char buf[32];
  strftime(buf, sizeof(buf), "/roast_%Y%m%d_%H%M%S.csv", &timeinfo);
  return String(buf);
}

void saveRoastProfile() {
  String filename = getRoastFilename();
  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  // Write Artisan CSV header (example, adjust as needed)
  file.printf("Date:%02d.%02d.%d\tUnit:C\tCHARGE:%02d:%02d\tTP:\tDRYe:\tFCs:%02d:%02d\tFCe:%02d:%02d\tSCs:%02d:%02d\tSCe:%02d:%02d\tDROP:%02d:%02d\tCOOL:\tTime:",timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_year + 1900,charge_time/60,charge_time%60,crack_times[0]/60,crack_times[0]%60,crack_times[1]/60,crack_times[1]%60,crack_times[2]/60,crack_times[2]%60,crack_times[3]/60,crack_times[3]%60,roast_data[data_count-1].timestamp/60,roast_data[data_count-1].timestamp%60);
  file.println("\nTime1\tTime2\tET\tBT\tEvent");

  // Helper to convert seconds to mm:ss
  auto sec_to_mmss = [](uint32_t sec, char* buf) {
    sprintf(buf, "%02d:%02d", sec / 60, sec % 60);
  };

  // Write roast data
  for (int i = 0; i < data_count; i++) {
    char t1[8], t2[8];
    sec_to_mmss(roast_data[i].timestamp, t1);
    if (roast_data[i].timestamp > charge_time)
      sec_to_mmss(roast_data[i].timestamp - charge_time, t2);
    else
      strcpy(t2, "0");
    float et_c = roast_data[i].bean_temp;
    float bt_c = roast_data[i].bean_temp; // Use same for ET/BT for now
    String event = "";
    if (charge_time > 0 && roast_data[i].timestamp == charge_time) event = "CHARGE";
    if (crack_count > 0 && roast_data[i].timestamp == crack_times[0]) event = "1stCrackStart";
    if (crack_count > 1 && roast_data[i].timestamp == crack_times[1]) event = "1stCrackEnd";
    if (crack_count > 2 && roast_data[i].timestamp == crack_times[2]) event = "2ndCrackStart";
    if (crack_count > 3 && roast_data[i].timestamp == crack_times[3]) event = "2ndCrackEnd";
    if (roast_active == false && i == data_count-1) event = "DROP";
    file.printf("%s\t%s\t%.4f\t%.4f\t%s\n", t1, t2, 26.5, bt_c, event.c_str());
  }
  file.close();
  Serial.print("Roast profile saved: ");
  Serial.println(filename);
} 