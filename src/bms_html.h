#ifndef BMS_HTML_H
#define BMS_HTML_H

#include <Arduino.h>
#include "bms_html_styles.h"
#include "bms_html_scripts.h"

String getHTMLPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 BMS Dashboard</title>
    <style>
)rawliteral";

  // Nhúng CSS từ bms_html_styles.h
  html += getHTMLStyles();

  html += R"rawliteral(
    </style>
</head>
<body>
    <div class="container">
        <div class="dashboard">
            <div class="header">
                <h1>🔋 BMS Dashboard</h1>
                <div class="header-right">
                    <div class="status-badge" id="chargingStatus">
                        <span class="status-icon">⚡</span>
                        <span class="status-text">Idle</span>
                    </div>
                    <div class="live-indicator">
                        <span class="dot"></span>
                        <span>Live</span>
                    </div>
                </div>
            </div>

            <div id="alertsContainer" class="alerts-container hidden"></div>

            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-icon">⚡</div>
                    <div class="stat-info">
                        <div class="stat-label">Pack Voltage</div>
                        <div class="stat-value" id="packVolt">--</div>
                    </div>
                </div>
                
                <div class="stat-card">
                    <div class="stat-icon">🔋</div>
                    <div class="stat-info">
                        <div class="stat-label">State of Charge</div>
                        <div class="stat-value" id="soc">--</div>
                    </div>
                </div>
                
                <div class="stat-card">
                    <div class="stat-icon">❤️</div>
                    <div class="stat-info">
                        <div class="stat-label">State of Health</div>
                        <div class="stat-value" id="soh">--</div>
                    </div>
                </div>
                
                <div class="stat-card">
                    <div class="stat-icon">⚡</div>
                    <div class="stat-info">
                        <div class="stat-label">Current</div>
                        <div class="stat-value" id="current">--</div>
                    </div>
                </div>

                <div class="stat-card">
                    <div class="stat-icon">🌡️</div>
                    <div class="stat-info">
                        <div class="stat-label">Pack Temperature</div>
                        <div class="stat-value" id="packTemp">--</div>
                    </div>
                </div>

                <div class="stat-card">
                    <div class="stat-icon">⚖️</div>
                    <div class="stat-info">
                        <div class="stat-label">Balancing</div>
                        <div class="stat-value small" id="balancingStatus">Inactive</div>
                    </div>
                </div>
            </div>

            <div class="section">
                <h2>🛡️ Protection Status</h2>
                <div class="protection-grid">
                    <div class="protection-item" id="protectOV">
                        <div class="protection-icon">⚠️</div>
                        <div class="protection-info">
                            <div class="protection-label">Over Voltage</div>
                            <div class="protection-status">Normal</div>
                        </div>
                    </div>

                    <div class="protection-item" id="protectUV">
                        <div class="protection-icon">⚠️</div>
                        <div class="protection-info">
                            <div class="protection-label">Under Voltage</div>
                            <div class="protection-status">Normal</div>
                        </div>
                    </div>

                    <div class="protection-item" id="protectOC">
                        <div class="protection-icon">⚠️</div>
                        <div class="protection-info">
                            <div class="protection-label">Over Current</div>
                            <div class="protection-status">Normal</div>
                        </div>
                    </div>

                    <div class="protection-item" id="protectSC">
                        <div class="protection-icon">⚠️</div>
                        <div class="protection-info">
                            <div class="protection-label">Short Circuit</div>
                            <div class="protection-status">Normal</div>
                        </div>
                    </div>

                    <div class="protection-item" id="protectOT">
                        <div class="protection-icon">⚠️</div>
                        <div class="protection-info">
                            <div class="protection-label">Over Temperature</div>
                            <div class="protection-status">Normal</div>
                        </div>
                    </div>
                </div>
            </div>

            <div class="section">
                <h2>📦 Trạng Thái Các Cell</h2>
                <div id="batteryDisplay" class="battery-grid">
                    <div class="loading">Đang tải dữ liệu...</div>
                </div>
            </div>
        </div>
    </div>

    <script>
)rawliteral";

  // Nhúng JS từ bms_html_scripts.h
  html += getHTMLScripts();

  html += R"rawliteral(
    </script>
</body>
</html>
)rawliteral";

  return html;
}

#endif
