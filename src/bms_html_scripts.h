#ifndef BMS_HTML_SCRIPTS_H
#define BMS_HTML_SCRIPTS_H

#include <Arduino.h>

String getHTMLScripts() {
    return R"rawliteral(
const UPDATE_INTERVAL = 2000;
let updateTimer = null;
let isConnected = false;

const elements = {
    packVolt: document.getElementById('packVolt'),
    soc: document.getElementById('soc'),
    soh: document.getElementById('soh'),
    current: document.getElementById('current'),
    packTemp: document.getElementById('packTemp'),
    balancingStatus: document.getElementById('balancingStatus'),
    chargingStatus: document.getElementById('chargingStatus'),
    protectOV: document.getElementById('protectOV'),
    protectUV: document.getElementById('protectUV'),
    protectOC: document.getElementById('protectOC'),
    protectSC: document.getElementById('protectSC'),
    protectOT: document.getElementById('protectOT'),
    batteryDisplay: document.getElementById('batteryDisplay'),
    alertsContainer: document.getElementById('alertsContainer')
};

function init() {
    console.log('BMS Dashboard Starting...');
    startAutoUpdate();
}

function startAutoUpdate() {
    fetchBMSData();
    updateTimer = setInterval(fetchBMSData, UPDATE_INTERVAL);
}

async function fetchBMSData() {
    try {
        const response = await fetch('/bms');
        if (!response.ok) throw new Error('HTTP ' + response.status);
        
        const data = await response.json();
        if (!isConnected) {
            isConnected = true;
            console.log('Connected to ESP32');
        }
        updateDashboard(data);
    } catch (error) {
        console.error('Connection Error:', error);
        handleConnectionError();
    }
}

function handleConnectionError() {
    if (isConnected) {
        isConnected = false;
        elements.batteryDisplay.innerHTML = '<div class="loading" style="color: #f44336;">Connection Lost</div>';
    }
}

function updateDashboard(data) {
    updateStats(data);
    updateProtectionStatus(data);
    updateBatteryCells(data);
    updateAlerts(data);
}

function updateStats(data) {
    const { measurement, calculation, status } = data;
    
    if (elements.packVolt) elements.packVolt.textContent = measurement.packVoltage + 'V';
    if (elements.soc) elements.soc.textContent = calculation.soc + '%';
    if (elements.soh) elements.soh.textContent = calculation.soh + '%';
    
    if (elements.current) {
        const curr = parseFloat(measurement.current);
        elements.current.textContent = (curr >= 0 ? '+' : '') + measurement.current + 'A';
    }
    
    if (elements.packTemp) elements.packTemp.textContent = measurement.packTemperature + '°C';
    
    if (elements.balancingStatus) {
        if (status.balancing.active) {
            elements.balancingStatus.textContent = 'Active (' + status.balancing.cells.length + ' cells)';
            elements.balancingStatus.style.color = '#ff9800';
        } else {
            elements.balancingStatus.textContent = 'Inactive';
            elements.balancingStatus.style.color = '#4caf50';
        }
    }
    
    updateChargingStatus(status.charging);
}

function updateChargingStatus(status) {
    if (!elements.chargingStatus) return;
    
    const statusIcon = elements.chargingStatus.querySelector('.status-icon');
    const statusText = elements.chargingStatus.querySelector('.status-text');
    
    elements.chargingStatus.classList.remove('charging', 'discharging', 'idle');
    
    if (status === 'charging') {
        elements.chargingStatus.classList.add('charging');
        statusIcon.textContent = '⚡';
        statusText.textContent = 'Charging';
    } else if (status === 'discharging') {
        elements.chargingStatus.classList.add('discharging');
        statusIcon.textContent = '🔋';
        statusText.textContent = 'Discharging';
    } else {
        elements.chargingStatus.classList.add('idle');
        statusIcon.textContent = '⏸️';
        statusText.textContent = 'Idle';
    }
}

function updateProtectionStatus(data) {
    const { protection } = data;
    updateProtectionItem(elements.protectOV, protection.overVoltage);
    updateProtectionItem(elements.protectUV, protection.underVoltage);
    updateProtectionItem(elements.protectOC, protection.overCurrent);
    updateProtectionItem(elements.protectOT, protection.overTemperature);
    updateProtectionItem(elements.protectSC, protection.shortCircuit);
}

function updateProtectionItem(element, status) {
    if (!element) return;
    const statusEl = element.querySelector('.protection-status');
    element.classList.remove('alert', 'alarm');
    
    if (status === 'alarm') {
        element.classList.add('alarm');
        statusEl.textContent = 'ALARM';
        statusEl.style.color = '#d32f2f';
    } else {
        statusEl.textContent = 'Normal';
        statusEl.style.color = '#4caf50';
    }
}

function updateBatteryCells(data) {
    if (!elements.batteryDisplay) return;
    
    const { measurement, status } = data;
    const cells = measurement.cellVoltages;
    
    let html = '';
    cells.forEach(cell => {
        const voltage = parseFloat(cell.voltage);
        const percentage = Math.max(0, Math.min(100, ((voltage - 3.0) / 1.2) * 100));
        
        let levelClass = 'critical';
        if (percentage >= 80) levelClass = 'full';
        else if (percentage >= 60) levelClass = 'good';
        else if (percentage >= 40) levelClass = 'medium';
        else if (percentage >= 20) levelClass = 'low';
        
        const isBalancing = status.balancing.active && status.balancing.cells.includes(cell.cell);
        
        html += '<div class="battery-cell' + (isBalancing ? ' balancing' : '') + '">';
        html += '<div class="cell-label">Cell ' + cell.cell + '</div>';
        html += '<div class="battery-icon-container">';
        html += '<div class="battery-head"></div>';
        html += '<div class="battery-body">';
        html += '<div class="battery-level ' + levelClass + '" style="height: ' + percentage + '%;"></div>';
        html += '</div></div>';
        html += '<div class="voltage-value">' + voltage.toFixed(3) + 'V</div>';
        html += '<div class="percentage">' + percentage.toFixed(0) + '%</div>';
        html += '</div>';
    });
    
    elements.batteryDisplay.innerHTML = html;
}

function updateAlerts(data) {
    if (!elements.alertsContainer) return;
    
    const alerts = data.alerts || [];
    if (alerts.length === 0) {
        elements.alertsContainer.classList.add('hidden');
        return;
    }
    
    elements.alertsContainer.classList.remove('hidden');
    
    let html = '';
    alerts.forEach(alert => {
        const icon = alert.severity === 'critical' ? '🚨' : '⚠️';
        html += '<div class="alert ' + alert.severity + '">';
        html += '<div class="alert-icon">' + icon + '</div>';
        html += '<div class="alert-message">' + alert.message + '</div>';
        html += '</div>';
    });
    
    elements.alertsContainer.innerHTML = html;
}

window.addEventListener('DOMContentLoaded', init);
)rawliteral";
}

#endif