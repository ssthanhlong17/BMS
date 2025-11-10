#include "BMS_Data.h"   // Gọi file bạn đã viết

// Hàm giả lập dữ liệu đo được từ pin
void simulateBmsData() {
  // Giả lập 4 cell điện áp (V)
  bmsData.cellVoltages[0] = 4.15;
  bmsData.cellVoltages[1] = 4.20;
  bmsData.cellVoltages[2] = 4.10;
  bmsData.cellVoltages[3] = 4.12;

  bmsData.packVoltage = 16.57; // Tổng điện áp pack
  bmsData.current = 12.5;      // Dòng điện (A)
  bmsData.packTemp = 45.3;     // Nhiệt độ (°C)
  bmsData.soc = 85.2;          // State of Charge (%)
  bmsData.soh = 97.6;          // State of Health (%)

  // Trạng thái hệ thống
  bmsData.isCharging = true;
  bmsData.isDischarging = false;
  bmsData.systemActive = true;

  // Balancing
  bmsData.balancingActive = true;
  bmsData.balancingCells[0] = false;
  bmsData.balancingCells[1] = true;
  bmsData.balancingCells[2] = false;
  bmsData.balancingCells[3] = true;

  // Cờ alarm (bạn có thể đổi giá trị này để test)
  bmsData.overVoltageAlarm = false;
  bmsData.underVoltageAlarm = false;
  bmsData.overCurrentAlarm = false;
  bmsData.overTempAlarm = false;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== BMS JSON Monitor ===");
}

void loop() {
  simulateBmsData();      // Tạo dữ liệu giả lập
  String jsonOutput = getBMSJson();  // Gọi hàm trong file .h

  Serial.println(jsonOutput); // In JSON ra Serial Monitor
  Serial.println("-------------------------");

  delay(2000); // 2 giây cập nhật 1 lần
}
