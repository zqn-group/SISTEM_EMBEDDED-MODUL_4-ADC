#include <Arduino.h>

// ===== DEFINISI PIN ADC =====
#define ADC_PIN_1 34  // GPIO 34 - Potentiometer
#define ADC_PIN_2 35  // GPIO 35 - LDR (Light Dependent Resistor)

#define THRESHOLD 50  // Threshold perubahan (dari 4095)
#define DEBOUNCE_TIME 100  // Debounce 100ms

// ===== VARIABEL GLOBAL =====
// Sensor 1 (Potentiometer)
volatile int lastADC1Value = 0;
volatile bool adc1Changed = false;
unsigned long lastADC1ChangeTime = 0;

// Sensor 2 (LDR)
volatile int lastADC2Value = 0;
volatile bool adc2Changed = false;
unsigned long lastADC2ChangeTime = 0;

// Response Time
volatile unsigned long responseTime = 0;
volatile bool responseUpdated = false;
int firstSensorTriggered = 0;  // 1 untuk ADC1, 2 untuk ADC2

// ===== FUNGSI INTERRUPT HANDLER ADC1 (POTENTIOMETER) =====
void IRAM_ATTR handleADC1Change() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastADC1ChangeTime < DEBOUNCE_TIME) {
        return;
    }
    
    int currentValue = analogRead(ADC_PIN_1);
    
    if (abs(currentValue - lastADC1Value) >= THRESHOLD) {
        lastADC1Value = currentValue;
        adc1Changed = true;
        lastADC1ChangeTime = currentTime;
        
        // Hitung response time jika ADC2 sudah berubah sebelumnya
        if (firstSensorTriggered == 2) {
            responseTime = currentTime - lastADC2ChangeTime;
            responseUpdated = true;
            firstSensorTriggered = 1;
        } else {
            firstSensorTriggered = 1;
            responseTime = 0;
        }
    }
}

// ===== FUNGSI INTERRUPT HANDLER ADC2 (LDR) =====
void IRAM_ATTR handleADC2Change() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastADC2ChangeTime < DEBOUNCE_TIME) {
        return;
    }
    
    int currentValue = analogRead(ADC_PIN_2);
    
    if (abs(currentValue - lastADC2Value) >= THRESHOLD) {
        lastADC2Value = currentValue;
        adc2Changed = true;
        lastADC2ChangeTime = currentTime;
        
        // Hitung response time jika ADC1 sudah berubah sebelumnya
        if (firstSensorTriggered == 1) {
            responseTime = currentTime - lastADC1ChangeTime;
            responseUpdated = true;
            firstSensorTriggered = 2;
        } else {
            firstSensorTriggered = 2;
            responseTime = 0;
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n===============================================");
    Serial.println(" Dual ADC Sensor with Response Time Monitor");
    Serial.println(" Sensor 1: Potentiometer (GPIO34)");
    Serial.println(" Sensor 2: LDR (GPIO35)");
    Serial.println("===============================================\n");
    
    // Konfigurasi ADC
    analogReadResolution(12);  // 12-bit (0-4095)
    analogSetAttenuation(ADC_11db);  // 0-3.3V
    
    // Baca nilai awal
    lastADC1Value = analogRead(ADC_PIN_1);
    lastADC2Value = analogRead(ADC_PIN_2);
    lastADC1ChangeTime = millis();
    lastADC2ChangeTime = millis();
    
    Serial.printf("Sensor 1 (Potentiometer) Initial: %d\n", lastADC1Value);
    Serial.printf("Sensor 2 (LDR) Initial: %d\n\n", lastADC2Value);
    Serial.println("Monitoring active... Waiting for sensor changes...\n");
}

void loop() {
    // Membaca nilai ADC terbaru
    int currentADC1 = analogRead(ADC_PIN_1);
    int currentADC2 = analogRead(ADC_PIN_2);
    
    unsigned long currentTime = millis();
    
    // ===== CEK PERUBAHAN SENSOR 1 =====
    if (abs(currentADC1 - lastADC1Value) >= THRESHOLD) {
        if (currentTime - lastADC1ChangeTime >= DEBOUNCE_TIME) {
            handleADC1Change();
        }
    }
    
    // ===== CEK PERUBAHAN SENSOR 2 =====
    if (abs(currentADC2 - lastADC2Value) >= THRESHOLD) {
        if (currentTime - lastADC2ChangeTime >= DEBOUNCE_TIME) {
            handleADC2Change();
        }
    }
    
    // ===== TAMPILKAN DATA KETIKA ADA PERUBAHAN =====
    if (adc1Changed || adc2Changed) {
        adc1Changed = false;
        adc2Changed = false;
        
        // Header
        Serial.println("\n╔════════════════════════════════════════════════╗");
        Serial.println("║     📊 SENSOR DATA & RESPONSE TIME MONITOR    ║");
        Serial.println("╚════════════════════════════════════════════════╝");
        
        // Data ADC1 (Potentiometer)
        float voltage1 = lastADC1Value * 3.3 / 4095.0;
        float percentage1 = (lastADC1Value / 4095.0) * 100.0;
        Serial.printf("│ Sensor 1 (Potentiometer)\n");
        Serial.printf("│   ADC Value: %4d / 4095\n", lastADC1Value);
        Serial.printf("│   Voltage:   %.2f V\n", voltage1);
        Serial.printf("│   Level:     %.1f %%\n", percentage1);
        
        // Data ADC2 (LDR)
        float voltage2 = lastADC2Value * 3.3 / 4095.0;
        float percentage2 = (lastADC2Value / 4095.0) * 100.0;
        Serial.printf("│\n");
        Serial.printf("│ Sensor 2 (LDR)\n");
        Serial.printf("│   ADC Value: %4d / 4095\n", lastADC2Value);
        Serial.printf("│   Voltage:   %.2f V\n", voltage2);
        Serial.printf("│   Level:     %.1f %%\n", percentage2);
        
        // Response Time
        Serial.printf("│\n");
        Serial.printf("│ ⚡ RESPONSE TIME: %lu ms\n", responseTime);
        Serial.printf("│   Triggered by: Sensor %d\n", firstSensorTriggered);
        Serial.println("╚════════════════════════════════════════════════╝");
        
        responseUpdated = false;
    }
    
    delay(50);  // Sampling interval
}
