#include <Arduino.h>

// ===== PIN CONFIGURATION =====
#define ADC_PIN_1 PA0        // ADC Sensor 1
#define ADC_PIN_2 PA1        // ADC Sensor 2
#define LED_PIN PB1          // LED Indicator

// ===== THRESHOLD & TIMING =====
#define THRESHOLD_ADC1 50    // Threshold for ADC1 (0-4095)
#define THRESHOLD_ADC2 50    // Threshold for ADC2 (0-4095)
#define DEBOUNCE_TIME 100    // Debounce 100ms
#define ADC_UPDATE_INTERVAL 100  // Update display every 100ms

// ===== GLOBAL VARIABLES =====
// Sensor 1 (ADC1)
volatile int lastADC1Value = 0;
volatile bool adc1Changed = false;
unsigned long lastADC1ChangeTime = 0;

// Sensor 2 (ADC2)
volatile int lastADC2Value = 0;
volatile bool adc2Changed = false;
unsigned long lastADC2ChangeTime = 0;

// Response Time
volatile unsigned long responseTime = 0;
int firstSensorTriggered = 0;  // 1=ADC1, 2=ADC2
unsigned long lastDisplayTime = 0;

// ===== INTERRUPT HANDLER ADC1 =====
void handleADC1Change() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastADC1ChangeTime < DEBOUNCE_TIME) {
        return;
    }
    
    int currentValue = analogRead(ADC_PIN_1);
    
    if (abs(currentValue - lastADC1Value) >= THRESHOLD_ADC1) {
        lastADC1Value = currentValue;
        adc1Changed = true;
        lastADC1ChangeTime = currentTime;
        
        if (firstSensorTriggered == 2) {
            responseTime = currentTime - lastADC2ChangeTime;
            firstSensorTriggered = 1;
        } else {
            firstSensorTriggered = 1;
            responseTime = 0;
        }
    }
}

// ===== INTERRUPT HANDLER ADC2 =====
void handleADC2Change() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastADC2ChangeTime < DEBOUNCE_TIME) {
        return;
    }
    
    int currentValue = analogRead(ADC_PIN_2);
    
    if (abs(currentValue - lastADC2Value) >= THRESHOLD_ADC2) {
        lastADC2Value = currentValue;
        adc2Changed = true;
        lastADC2ChangeTime = currentTime;
        
        if (firstSensorTriggered == 1) {
            responseTime = currentTime - lastADC1ChangeTime;
            firstSensorTriggered = 2;
        } else {
            firstSensorTriggered = 2;
            responseTime = 0;
        }
    }
}

// ===== LED CONTROL =====
void updateLED(int adcValue) {
    // Map ADC value to LED brightness
    // 0-1023: LED OFF
    // 1024-2047: LED dim
    // 2048-3071: LED medium
    // 3072-4095: LED bright
    
    if (adcValue < 1024) {
        digitalWrite(LED_PIN, LOW);  // Off
    } else if (adcValue < 2048) {
        analogWrite(LED_PIN, 85);    // Dim (1/3 brightness)
    } else if (adcValue < 3072) {
        analogWrite(LED_PIN, 170);   // Medium (2/3 brightness)
    } else {
        digitalWrite(LED_PIN, HIGH); // Full brightness
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=====================================");
    Serial.println("Dual ADC Sensor System - STM32");
    Serial.println("ADC1: PA0 | ADC2: PA1 | LED: PB1");
    Serial.println("=====================================\n");
    
    // Initialize pins
    analogReadResolution(12);  // 12-bit (0-4095)
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Read initial values
    lastADC1Value = analogRead(ADC_PIN_1);
    lastADC2Value = analogRead(ADC_PIN_2);
    lastADC1ChangeTime = millis();
    lastADC2ChangeTime = millis();
    lastDisplayTime = millis();
    
    Serial.print("Initial ADC1 Value: ");
    Serial.println(lastADC1Value);
    
    Serial.print("Initial ADC2 Value: ");
    Serial.println(lastADC2Value);
    Serial.println("\nMonitoring active...\n");
}

void loop() {
    // Read ADC values
    int currentADC1 = analogRead(ADC_PIN_1);
    int currentADC2 = analogRead(ADC_PIN_2);
    
    unsigned long currentTime = millis();
    
    // ===== CHECK ADC1 CHANGE =====
    if (abs(currentADC1 - lastADC1Value) >= THRESHOLD_ADC1) {
        if (currentTime - lastADC1ChangeTime >= DEBOUNCE_TIME) {
            handleADC1Change();
        }
    }
    
    // ===== CHECK ADC2 CHANGE =====
    if (abs(currentADC2 - lastADC2Value) >= THRESHOLD_ADC2) {
        if (currentTime - lastADC2ChangeTime >= DEBOUNCE_TIME) {
            handleADC2Change();
        }
    }
    
    // ===== UPDATE LED BASED ON ADC1 =====
    updateLED(lastADC1Value);
    
    // ===== DISPLAY DATA WHEN CHANGE DETECTED OR TIME INTERVAL =====
    if (adc1Changed || adc2Changed || (currentTime - lastDisplayTime >= ADC_UPDATE_INTERVAL)) {
        adc1Changed = false;
        adc2Changed = false;
        lastDisplayTime = currentTime;
        
        // Calculate voltages
        float voltage1 = lastADC1Value * 3.3 / 4095.0;
        float voltage2 = lastADC2Value * 3.3 / 4095.0;
        
        // Display data
        Serial.println("\n--- SENSOR DATA ---");
        
        Serial.print("ADC Value 1: ");
        Serial.println(lastADC1Value);
        
        Serial.print("ADC Value 2: ");
        Serial.println(lastADC2Value);
        
        Serial.print("Response Time: ");
        Serial.print(responseTime);
        Serial.println(" ms");
        
        Serial.println("--- END ---\n");
    }
    
    delay(20);  // Sampling interval
}
