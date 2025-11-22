#ifndef SOC_ESTIMATOR_H
#define SOC_ESTIMATOR_H

#include <Arduino.h>

/*
 * SOC ESTIMATOR - Simplified Version (Bỏ Peukert)
 * Phù hợp với pack 4S 6Ah
 * - Coulomb Counting cơ bản
 * - Hiệu chỉnh nhiệt độ
 * - OCV calibration khi pin nghỉ
 */

class SOCEstimator {
private:
    float batteryCapacity;      // 6.0 Ah (tính cho từng cell)
    float currentSOC;
    float chargeAccumulated;
    unsigned long lastUpdateTime;
    
    float chargeEfficiency;     // 0.97 khi sạc
    float referenceTemperature; // 25°C
    float temperatureCoefficient; // 0.6 %/°C
    
    // Thống kê
    float totalChargeIn;
    float totalChargeOut;
    int cycleCount;
    
    // OCV Lookup Table cho LiFePO4 (SOC% -> Voltage per cell)
    const float ocvTable[11][2] = {
        {0,   2.50}, {10,  2.90}, {20,  3.00}, {30,  3.10},
        {40,  3.15}, {50,  3.20}, {60,  3.25}, {70,  3.28},
        {80,  3.30}, {90,  3.35}, {100, 3.40}
    };
    
    // Nội suy tuyến tính từ OCV table
    float interpolateOCV(float soc) {
        soc = constrain(soc, 0, 100);
        for (int i = 0; i < 10; i++) {
            if (soc >= ocvTable[i][0] && soc <= ocvTable[i + 1][0]) {
                float soc1 = ocvTable[i][0];
                float soc2 = ocvTable[i + 1][0];
                float v1 = ocvTable[i][1];
                float v2 = ocvTable[i + 1][1];
                return v1 + (v2 - v1) * (soc - soc1) / (soc2 - soc1);
            }
        }
        return 3.20;
    }
    
    // Chuyển đổi điện áp thành SOC
    float socFromOCV(float voltage) {
        voltage = constrain(voltage, 2.5, 3.6);
        for (int i = 0; i < 10; i++) {
            if (voltage >= ocvTable[i][1] && voltage <= ocvTable[i + 1][1]) {
                float v1 = ocvTable[i][1];
                float v2 = ocvTable[i + 1][1];
                float soc1 = ocvTable[i][0];
                float soc2 = ocvTable[i + 1][0];
                return soc1 + (soc2 - soc1) * (voltage - v1) / (v2 - v1);
            }
        }
        return 50.0;
    }
    
public:
    SOCEstimator(float capacity = 6.0, float initialSOC = 100.0) {
        batteryCapacity = capacity;
        currentSOC = initialSOC;
        chargeAccumulated = (initialSOC / 100.0) * capacity;
        lastUpdateTime = millis();
        
        chargeEfficiency = 0.97;
        referenceTemperature = 25.0;
        temperatureCoefficient = 0.6; // 0.6%/°C
        
        totalChargeIn = 0;
        totalChargeOut = 0;
        cycleCount = 0;
    }
    
    // Cập nhật SOC với dòng và nhiệt độ
    void update(float current, float temperature = 25.0) {
        unsigned long now = millis();
        float deltaTime = (now - lastUpdateTime) / 3600000.0; // Convert to hours
        lastUpdateTime = now;
        
        if (deltaTime == 0 || deltaTime > 1.0) return;
        
        // Tính toán ảnh hưởng nhiệt độ
        float tempDiff = temperature - referenceTemperature;
        float tempFactor = 1.0 + (temperatureCoefficient * tempDiff / 100.0);
        tempFactor = constrain(tempFactor, 0.8, 1.2);
        
        if (current > 0) {
            // Sạc: áp dụng hiệu suất
            float effectiveCharge = current * deltaTime * chargeEfficiency;
            chargeAccumulated += effectiveCharge;
            totalChargeIn += current * deltaTime;
            
        } else if (current < 0) {
            // Xả: Coulomb counting + ảnh hưởng nhiệt độ
            float discharge = abs(current) * deltaTime * tempFactor;
            chargeAccumulated -= discharge;
            totalChargeOut += abs(current) * deltaTime;
        }
        
        chargeAccumulated = constrain(chargeAccumulated, 0, batteryCapacity);
        currentSOC = (chargeAccumulated / batteryCapacity) * 100.0;
    }
    
    // Hiệu chỉnh SOC dựa trên điện áp OCV
    // Chỉ nên gọi khi pin nghỉ (restTime > 30 phút)
    void calibrateWithVoltage(float avgCellVoltage, float restTime = 0) {
        if (restTime < 1800) { // < 30 phút
            Serial.println("⚠️ Warning: Battery not rested enough for OCV calibration");
            Serial.println("   Recommended rest time: >= 30 minutes");
            return;
        }
        
        float socFromVoltage = socFromOCV(avgCellVoltage);
        
        // Weighted average: 70% Coulomb, 30% OCV
        float calibratedSOC = currentSOC * 0.7 + socFromVoltage * 0.3;
        
        Serial.println("=== SOC CALIBRATION ===");
        Serial.printf("  Coulomb SOC: %.2f%%\n", currentSOC);
        Serial.printf("  OCV SOC: %.2f%% (from %.3fV)\n", socFromVoltage, avgCellVoltage);
        Serial.printf("  Calibrated SOC: %.2f%%\n", calibratedSOC);
        Serial.println("=======================");
        
        currentSOC = calibratedSOC;
        chargeAccumulated = (currentSOC / 100.0) * batteryCapacity;
    }
    
    // Getters
    float getSOC() { 
        return currentSOC; 
    }
    
    float getRemainingCapacity() { 
        return chargeAccumulated; 
    }
    
    float getExpectedVoltage() { 
        return interpolateOCV(currentSOC); 
    }
    
    int getCycleCount() {
        return cycleCount;
    }
    
    // Ước tính sức khỏe pin (%)
    // Giả định mất 20% sau 2000 chu kỳ
    float getCapacityHealth() {
        float degradation = (cycleCount / 2000.0) * 20.0;
        return constrain(100.0 - degradation, 50.0, 100.0);
    }
    
    // Reset SOC
    void reset(float newSOC = 100.0) {
        currentSOC = newSOC;
        chargeAccumulated = (newSOC / 100.0) * batteryCapacity;
        lastUpdateTime = millis();
    }
    
    // In thông tin debug
    void printDebug(float avgCellVoltage, float current, float temperature) {
        Serial.println("===== SOC ESTIMATOR DEBUG =====");
        Serial.printf("🔋 Battery: %.1f Ah @ %.1f°C\n", batteryCapacity, temperature);
        Serial.printf("⚡ Current: %.3f A (%.2fC rate)\n", 
                      current, abs(current) / batteryCapacity);
        Serial.println("-------------------------------");
        Serial.printf("📊 SOC (Coulomb): %.2f%%\n", currentSOC);
        Serial.printf("📍 SOC (OCV): %.2f%% (from %.3fV)\n", 
                      socFromOCV(avgCellVoltage), avgCellVoltage);
        Serial.printf("💾 Remaining: %.3f Ah\n", chargeAccumulated);
        Serial.printf("📈 Expected OCV: %.3f V\n", getExpectedVoltage());
        Serial.printf("📥 Total In: %.3f Ah\n", totalChargeIn);
        Serial.printf("📤 Total Out: %.3f Ah\n", totalChargeOut);
        Serial.println("===============================\n");
    }
};

#endif