#ifndef BMS_SENSORS_H
#define BMS_SENSORS_H

#include <Arduino.h>

class BMSSensors {
private:
    // Giá trị giả lập
    float cellVoltages[4];
    float current;
    float temperature;
    
    // ========== SIMULATION PARAMETERS ==========
    unsigned long startTime;
    float simulatedCapacity;      // Dung lượng giảm theo thời gian
    int cycleCount;               // Số chu kỳ sạc/xả
    float baseCapacity;           // Dung lượng ban đầu
    
    // Trạng thái sạc/xả hiện tại
    enum ChargeState { IDLE, CHARGING, DISCHARGING };
    ChargeState chargeState;
    unsigned long stateChangeTime;
    
public:
    BMSSensors() {
        // Khởi tạo dữ liệu giả lập
        cellVoltages[0] = 3.40;
        cellVoltages[1] = 3.40;
        cellVoltages[2] = 3.40;
        cellVoltages[3] = 3.40;
        current = 0.0;           // Ban đầu idle
        temperature = 25.0;      // °C
        
        startTime = millis();
        baseCapacity = 6.0;      // Dung lượng ban đầu (Ah)
        simulatedCapacity = 6.0;
        cycleCount = 0;
        chargeState = IDLE;
        stateChangeTime = millis();
    }

    void begin() {
        Serial.println("✅ BMS Sensors initialized (simulation mode)");
        Serial.println("📊 Simulating battery degradation over time");
        Serial.println("   - Capacity loss: ~1% per 100 cycles");
        Serial.println("   - Temperature fluctuation: ±5°C");
        Serial.println("   - Charge/Discharge cycling every 120 seconds\n");
    }

    // Cập nhật dữ liệu mô phỏng (thay đổi theo thời gian)
    void readAllSensors() {
        unsigned long elapsed = millis() - startTime;
        unsigned long elapsedSeconds = elapsed / 1000;
        
        // ========== SIMULATION: CHARGE/DISCHARGE CYCLES ==========
        // Chu kỳ 120 giây: 40s sạc + 40s xả + 40s idle
        unsigned long cycleTime = elapsedSeconds % 120;
        
        if (cycleTime < 40) {
            // ===== CHARGING PHASE (0-40s) =====
            chargeState = CHARGING;
            current = 1.5;  // Sạc 1.5A
            
            // Điện áp tăng từ 3.0V đến 3.4V khi sạc
            float chargeProgress = cycleTime / 40.0;  // 0.0 → 1.0
            float baseVoltage = 3.0 + (chargeProgress * 0.4);
            
            // Thêm sự biến thiên nhỏ giữa các cell
            cellVoltages[0] = baseVoltage + 0.01;
            cellVoltages[1] = baseVoltage + 0.005;
            cellVoltages[2] = baseVoltage - 0.005;
            cellVoltages[3] = baseVoltage - 0.01;
            
        } else if (cycleTime < 80) {
            // ===== DISCHARGING PHASE (40-80s) =====
            chargeState = DISCHARGING;
            current = -1.2;  // Xả 1.2A (dòng xả thường cao hơn dòng sạc)
            
            // Điện áp giảm từ 3.4V đến 3.0V khi xả
            float dischargeProgress = (cycleTime - 40) / 40.0;  // 0.0 → 1.0
            float baseVoltage = 3.4 - (dischargeProgress * 0.4);
            
            cellVoltages[0] = baseVoltage + 0.01;
            cellVoltages[1] = baseVoltage + 0.005;
            cellVoltages[2] = baseVoltage - 0.005;
            cellVoltages[3] = baseVoltage - 0.01;
            
        } else {
            // ===== IDLE PHASE (80-120s) =====
            chargeState = IDLE;
            current = 0.0;
            
            // Điện áp ổn định ở ~3.2V
            cellVoltages[0] = 3.20 + 0.01;
            cellVoltages[1] = 3.20 + 0.005;
            cellVoltages[2] = 3.20 - 0.005;
            cellVoltages[3] = 3.20 - 0.01;
        }
        
        // ========== SIMULATION: CAPACITY DEGRADATION ==========
        // Mô phỏng suy giảm dung lượng: 1% mất sau 100 chu kỳ
        // cycleCount tăng mỗi chu kỳ hoàn chỉnh (mỗi 120 giây)
        cycleCount = elapsedSeconds / 120;
        
        // Công thức: Capacity = Capacity_0 * (1 - degradation_rate * cycles)
        float degradationRate = 0.001;  // 0.1% mất per cycle
        simulatedCapacity = baseCapacity * (1.0 - (degradationRate * cycleCount));
        simulatedCapacity = constrain(simulatedCapacity, baseCapacity * 0.7, baseCapacity);  // Min 70%
        
        // ========== SIMULATION: TEMPERATURE FLUCTUATION ==========
        // Nhiệt độ biến đổi: 20-30°C theo thời gian
        float tempVariation = sin(elapsedSeconds * 0.01) * 5.0;  // ±5°C
        temperature = 25.0 + tempVariation;
        
        // Nhiệt độ tăng khi sạc/xả, hạ khi idle
        if (chargeState == CHARGING) {
            temperature += 2.0;  // +2°C khi sạc
        } else if (chargeState == DISCHARGING) {
            temperature += 3.0;  // +3°C khi xả (tỏa nhiệt)
        }
        
        temperature = constrain(temperature, 10.0, 50.0);
    }

    // Getters
    float getCellVoltage(int cellNum) {
        if (cellNum >= 1 && cellNum <= 4)
            return cellVoltages[cellNum - 1];
        return 0.0;
    }

    float getCurrent() {
        return current;
    }

    float getTemperature() {
        return temperature;
    }

    float getPackVoltage() {
        return cellVoltages[0] + cellVoltages[1] + cellVoltages[2] + cellVoltages[3];
    }
    
    // Getter cho dung lượng mô phỏng (để debug)
    float getSimulatedCapacity() {
        return simulatedCapacity;
    }
    
    int getCycleCount() {
        return cycleCount;
    }

    // Debug: in tất cả dữ liệu
    void printDebug() {
        unsigned long elapsed = millis() - startTime;
        unsigned long elapsedSeconds = elapsed / 1000;
        unsigned long minutes = elapsedSeconds / 60;
        unsigned long seconds = elapsedSeconds % 60;
        
        Serial.println("\n========== BMS SENSORS DEBUG (SIMULATION) ==========");
        Serial.printf("⏱️  Elapsed Time: %02lu:%02lu (Cycles: %d)\n", minutes, seconds, cycleCount);
        
        Serial.println("\n📦 CELLS:");
        for (int i = 0; i < 4; i++)
            Serial.printf("  Cell %d: %.3f V\n", i + 1, cellVoltages[i]);
        
        Serial.printf("\n⚡ MEASUREMENTS:");
        Serial.printf("  Pack Voltage: %.2f V\n", getPackVoltage());
        Serial.printf("  Current: %.2f A", current);
        if (current > 0.1) Serial.print(" [CHARGING]");
        else if (current < -0.1) Serial.print(" [DISCHARGING]");
        else Serial.print(" [IDLE]");
        Serial.println();
        
        Serial.printf("  Temperature: %.1f °C\n", temperature);
        
        Serial.println("\n📊 DEGRADATION SIMULATION:");
        Serial.printf("  Base Capacity: %.2f Ah\n", baseCapacity);
        Serial.printf("  Current Capacity: %.2f Ah (%.1f%%)\n", 
                      simulatedCapacity, 
                      (simulatedCapacity / baseCapacity) * 100.0);
        Serial.printf("  Cycles: %d\n", cycleCount);
        
        float degradationPercent = ((baseCapacity - simulatedCapacity) / baseCapacity) * 100.0;
        Serial.printf("  Total Degradation: %.1f%%\n", degradationPercent);
        
        Serial.println("=====================================================\n");
    }
};

#endif