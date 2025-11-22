#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "bms_sensors.h"
#include "bms_data.h"
#include "bms_html.h"

// ============ WiFi Configuration ============
const char* WIFI_SSID = "Wifi 2.4G";
const char* WIFI_PASSWORD = "66668888";

// ============ Web Server ============
WebServer server(80);

// ============ BMS Objects ============
BMSSensors sensors;

// ============ Timing ============
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_READ_INTERVAL = 500;

unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_PRINT_INTERVAL = 5000;

// ============================================
// WIFI CONNECTION (ĐẶT TRƯỚC SETUP)
// ============================================
void connectWiFi() {
    Serial.println("📡 Connecting to WiFi...");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println("❌ WiFi connection failed!");
        Serial.println("⚠️  System will continue without WiFi");
    }
}

// ============================================
// WEB SERVER SETUP
// ============================================
void setupWebServer() {
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", getHTMLPage());
    });
    
    server.on("/bms", HTTP_GET, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "application/json", getBMSJson());
    });
    
    server.on("/info", HTTP_GET, []() {
        String info = "ESP32 BMS System\n";
        info += "Uptime: " + String(millis() / 1000) + "s\n";
        info += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
        info += "WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
        server.send(200, "text/plain", info);
    });
    
    server.onNotFound([]() {
        server.send(404, "text/plain", "404: Not Found");
    });
    
    Serial.println("✅ Web server routes configured");
}

// ============================================
// BMS FUNCTIONS
// ============================================
void readAndUpdateBMS() {
    sensors.readAllSensors();
    
    float cell1 = sensors.getCellVoltage(1);
    float cell2 = sensors.getCellVoltage(2);
    float cell3 = sensors.getCellVoltage(3);
    float cell4 = sensors.getCellVoltage(4);
    float current = sensors.getCurrent();
    float temp = sensors.getTemperature();
    
    updateBMSData(cell1, cell2, cell3, cell4, current, temp);
}

void printBMSStatus() {
    Serial.println("\n========================================");
    Serial.println("📊 BMS STATUS REPORT");
    Serial.println("========================================");
    
    Serial.println("📦 CELLS:");
    for (int i = 0; i < NUM_CELLS; i++) {
        Serial.printf("  Cell %d: %.3fV", i+1, bmsData.cellVoltages[i]);
        if (bmsData.balancingCells[i]) {
            Serial.print(" [BALANCING]");
        }
        Serial.println();
    }
    
    Serial.println("\n⚡ PACK:");
    Serial.printf("  Voltage: %.2fV\n", bmsData.packVoltage);
    Serial.printf("  Current: %.2fA", bmsData.current);
    if (bmsData.isCharging) Serial.print(" [CHARGING]");
    if (bmsData.isDischarging) Serial.print(" [DISCHARGING]");
    Serial.println();
    Serial.printf("  Temperature: %.1f°C\n", bmsData.packTemp);
    
    Serial.println("\n📊 STATE:");
    Serial.printf("  SOC: %.1f%%\n", bmsData.soc);
    Serial.printf("  SOH: %.1f%%\n", bmsData.soh);
    
    Serial.println("\n🛡️ PROTECTION:");
    Serial.printf("  Over Voltage: %s\n", bmsData.overVoltageAlarm ? "ALARM" : "OK");
    Serial.printf("  Under Voltage: %s\n", bmsData.underVoltageAlarm ? "ALARM" : "OK");
    Serial.printf("  Over Current: %s\n", bmsData.overCurrentAlarm ? "ALARM" : "OK");
    Serial.printf("  Over Temperature: %s\n", bmsData.overTempAlarm ? "ALARM" : "OK");
    
    Serial.println("========================================\n");
}

// ============================================
// SETUP (BÂY GIỜ MỚI ĐẾN SETUP)
// ============================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("🔋 ESP32 BMS System Starting...");
    Serial.println("========================================");
    
    initBMSData();
    Serial.println("✅ BMS Data initialized");
    
    sensors.begin();
    
    connectWiFi();
    
    if (MDNS.begin("esp32bms")) {
        Serial.println("✅ mDNS responder started: http://esp32bms.local");
    }
    
    setupWebServer();
    
    server.begin();
    Serial.println("✅ HTTP server started");
    
    Serial.println("========================================");
    Serial.println("🎉 BMS System Ready!");
    Serial.println("========================================");
    Serial.print("📡 Access Dashboard at: http://");
    Serial.println(WiFi.localIP());
    Serial.println("========================================\n");
}

// ============================================
// MAIN LOOP
// ============================================
void loop() {
    server.handleClient();
    
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
        lastSensorRead = millis();
        readAndUpdateBMS();
    }
    
    if (millis() - lastDebugPrint >= DEBUG_PRINT_INTERVAL) {
        lastDebugPrint = millis();
        printBMSStatus();
    }
}