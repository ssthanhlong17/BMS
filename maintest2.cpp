
#include <WiFi.h>
#include <WebServer.h>
#include "BMS_DATA.h"   

// --- Thay bằng SSID/Pass của bạn ---
const char* ssid = "Wifi 2.4G";
const char* password = "66668888";

WebServer server(80);

void handleRoot() {
  server.sendHeader("Location", "/index.html");
  server.send(302, "text/plain", "");
}

void handleApiBms() {
  // CORS (cho phép mọi nguồn; thay * bằng origin cụ thể khi deploy)
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  String json = getBMSJson(); // từ BMS_DATA_H
  server.send(200, "application/json", json);
}

void handleOptions() {
  // trả về CORS preflight
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nBooting...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(350);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/bms", HTTP_GET, handleApiBms);
  server.on("/api/bms", HTTP_OPTIONS, handleOptions);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // --- Demo: cập nhật bmsData mẫu (bạn thay bằng đọc ADC / cảm biến thật) ---
  // Ví dụ thay đổi ngẫu nhiên để thấy UI cập nhật:
  bmsData.packVoltage = 3.65 * 4.0; // ví dụ
  bmsData.cellVoltages[0] = 3.65;
  bmsData.cellVoltages[1] = 3.63;
  bmsData.cellVoltages[2] = 3.60;
  bmsData.cellVoltages[3] = 3.62;
  bmsData.current = -1.23;
  bmsData.packTemp = 34.5;
  bmsData.soc = 78.4;
  bmsData.soh = 95.1;
  bmsData.isCharging = false;
  bmsData.isDischarging = true;
  bmsData.systemActive = true;
  bmsData.balancingActive = true;
  bmsData.balancingCells[1] = true; // cell2 balancing
  delay(500); 
}
