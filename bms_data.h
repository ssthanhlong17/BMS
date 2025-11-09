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
  
  // Protection status (3 mức: 0=normal, 1=warning, 2=alarm)
  uint8_t overVoltageLevel;      // 0, 1, 2
  uint8_t underVoltageLevel;     // 0, 1, 2
  uint8_t overCurrentLevel;      // 0, 1, 2
  uint8_t overTempLevel;         // 0, 1, 2
  uint8_t shortCircuitLevel;     // 0, 1, 2
  
  // Balancing
  bool balancingActive;
  bool balancingCells[NUM_CELLS]; // Cell nào đang balance
  
  // Charging status
  bool isCharging;
  bool isDischarging;
  bool systemActive;
};

BMSData bmsData;

// Helper function: Convert level to string
const char* levelToString(uint8_t level) {
  switch(level) {
    case 2: return "alarm";
    case 1: return "warning";
    default: return "normal";
  }
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
  protection["overVoltage"] = levelToString(bmsData.overVoltageLevel);
  protection["underVoltage"] = levelToString(bmsData.underVoltageLevel);
  protection["overCurrent"] = levelToString(bmsData.overCurrentLevel);
  protection["overTemperature"] = levelToString(bmsData.overTempLevel);
  protection["shortCircuit"] = levelToString(bmsData.shortCircuitLevel);
  
  // ============ ALERTS ============
  JsonArray alerts = doc.createNestedArray("alerts");
  
  // ---- Thay logic Over Voltage bằng SOC > 100% ----
  // Nếu muốn chỉ cảnh báo khi đang sạc, thêm điều kiện bmsData.isCharging && ...
  if (bmsData.soc > 100.0) {
    JsonObject alert = alerts.createNestedObject();
    
    if (bmsData.soc > 105.0) {
      alert["severity"] = "critical";
      alert["message"] = "SOC exceeds 105% - possible overcharge condition!";
    } else {
      alert["severity"] = "warning";
      alert["message"] = "SOC above 100% - battery fully charged.";
    }
    
    alert["soc"] = String(bmsData.soc, 1); // thêm thông tin SOC
  }
  
  // Under Voltage (giữ theo mức hiện tại: dùng underVoltageLevel)
  if (bmsData.underVoltageLevel >= 1) {
    JsonObject alert = alerts.createNestedObject();
    alert["severity"] = bmsData.underVoltageLevel == 2 ? "critical" : "warning";
    alert["message"] = bmsData.underVoltageLevel == 2 ? "Under Voltage ALARM!" : "Under Voltage Warning";
  }
  
  // Over Current
  if (bmsData.overCurrentLevel >= 1) {
    JsonObject alert = alerts.createNestedObject();
    alert["severity"] = bmsData.overCurrentLevel == 2 ? "critical" : "warning";
    alert["message"] = bmsData.overCurrentLevel == 2 ? "Over Current ALARM!" : "Over Current Warning";
  }
  
  // Over Temperature
  if (bmsData.overTempLevel >= 1) {
    JsonObject alert = alerts.createNestedObject();
    alert["severity"] = bmsData.overTempLevel == 2 ? "critical" : "warning";
    alert["message"] = bmsData.overTempLevel == 2 ? "Over Temperature ALARM!" : "High Temperature Warning";
  }
  
  // ==== BỎ cảnh báo Short Circuit: (đã loại) ====
  // Trước đây code có: if (bmsData.shortCircuitLevel >= 1) { ... }
  // Bây giờ không tạo alert cho short circuit (vẫn giữ protection["shortCircuit"] để báo trạng thái nếu cần)
  
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

// Set protection level và tự động cập nhật alerts
void setProtectionLevel(uint8_t &level, uint8_t newLevel) {
  level = constrain(newLevel, 0, 2);
}

#endif
