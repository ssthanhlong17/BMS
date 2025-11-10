#ifndef BMS_DATA_H
#define BMS_DATA_H

#include <ArduinoJson.h>

const int NUM_CELLS = 4;

struct BMSData {
  // Đo lường cơ bản
  float cellVoltages[NUM_CELLS];
  float packVoltage;
  float current;
  float packTemp;
  float soc;
  float soh;
  
  // Protection status (bool: false=normal, true=alarm)
  bool overVoltageAlarm;
  bool underVoltageAlarm;
  bool overCurrentAlarm;
  bool overTempAlarm;
  
  // Balancing
  bool balancingActive;
  bool balancingCells[NUM_CELLS]; // Cell nào đang balance
  
  // Charging status
  bool isCharging;
  bool isDischarging;
  bool systemActive;
};

BMSData bmsData;

// Helper function: Convert bool to status string
const char* statusToString(bool alarm) {
  return alarm ? "alarm" : "normal";
}

String getBMSJson() {
  StaticJsonDocument<2048> doc;
  
  // ============ MEASUREMENT ============
  JsonObject measurement = doc.createNestedObject("measurement");
  
  // Cell voltages
  JsonArray cells = measurement.createNestedArray("cellVoltages");
  for (int i = 0; i < NUM_CELLS; i++) {
    JsonObject cell = cells.createNestedObject();
    cell["cell"] = i + 1;
    cell["voltage"] = String(bmsData.cellVoltages[i], 3);
  }
  
  measurement["packVoltage"] = String(bmsData.packVoltage, 2);
  measurement["current"] = String(bmsData.current, 2);
  measurement["packTemperature"] = String(bmsData.packTemp, 1);
  
  // ============ CALCULATION ============
  JsonObject calculation = doc.createNestedObject("calculation");
  calculation["soc"] = String(bmsData.soc, 1);
  calculation["soh"] = String(bmsData.soh, 1);
  
  // ============ STATUS ============
  JsonObject status = doc.createNestedObject("status");
  
  // Charging status as string
  if (bmsData.isCharging) {
    status["charging"] = "charging";
  } else if (bmsData.isDischarging) {
    status["charging"] = "discharging";
  } else {
    status["charging"] = "idle";
  }
  
  // Balancing
  JsonObject balancing = status.createNestedObject("balancing");
  balancing["active"] = bmsData.balancingActive;
  
  JsonArray balancingCellsArray = balancing.createNestedArray("cells");
  if (bmsData.balancingActive) {
    for (int i = 0; i < NUM_CELLS; i++) {
      if (bmsData.balancingCells[i]) {
        balancingCellsArray.add(i + 1);
      }
    }
  }
  
  // ============ PROTECTION ============
  JsonObject protection = doc.createNestedObject("protection");
  protection["overVoltage"] = statusToString(bmsData.overVoltageAlarm);
  protection["underVoltage"] = statusToString(bmsData.underVoltageAlarm);
  protection["overCurrent"] = statusToString(bmsData.overCurrentAlarm);
  protection["overTemperature"] = statusToString(bmsData.overTempAlarm);
  
  // ============ ALERTS ============
  JsonArray alerts = doc.createNestedArray("alerts");
  
  // Tự động tạo alerts từ protection status
  if (bmsData.overVoltageAlarm) {
    JsonObject alert = alerts.createNestedObject();
    alert["severity"] = "critical";
    alert["message"] = "Over Voltage ALARM!";
  }
  
  if (bmsData.underVoltageAlarm) {
    JsonObject alert = alerts.createNestedObject();
    alert["severity"] = "critical";
    alert["message"] = "Under Voltage ALARM!";
  }
  
  if (bmsData.overCurrentAlarm) {
    JsonObject alert = alerts.createNestedObject();
    alert["severity"] = "critical";
    alert["message"] = "Over Current ALARM!";
  }
  
  if (bmsData.overTempAlarm) {
    JsonObject alert = alerts.createNestedObject();
    alert["severity"] = "critical";
    alert["message"] = "Over Temperature ALARM!";
  }
  
  // Alert nếu cell imbalance
  if (bmsData.balancingActive) {
    float maxV = bmsData.cellVoltages[0];
    float minV = bmsData.cellVoltages[0];
    for (int i = 1; i < NUM_CELLS; i++) {
      if (bmsData.cellVoltages[i] > maxV) maxV = bmsData.cellVoltages[i];
      if (bmsData.cellVoltages[i] < minV) minV = bmsData.cellVoltages[i];
    }
    if ((maxV - minV) > 0.05) { // Chênh lệch > 50mV
      JsonObject alert = alerts.createNestedObject();
      alert["severity"] = "warning";
      alert["message"] = "Cell voltage imbalance detected";
    }
  }
  
  String output;
  serializeJson(doc, output);
  return output;
}

// ============ HELPER FUNCTIONS ============

// Set protection alarm status
void setProtectionAlarm(bool &alarm, bool status) {
  alarm = status;
}

#endif