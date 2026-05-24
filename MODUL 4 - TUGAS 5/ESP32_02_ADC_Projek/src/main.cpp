#include <Arduino.h>
#include <DHT.h>

// ===== PIN CONFIGURATION =====
#define ADC_PIN_1 34        // GPIO 34 - Potentiometer
#define DHT_PIN 27          // GPIO 27 - DHT22 Data Pin
#define DHT_TYPE DHT22

// ===== THRESHOLD & TIMING =====
#define THRESHOLD_ADC1 30   // Threshold for Potentiometer (0-4095) - REDUCED
#define THRESHOLD_DHT 1     // Threshold for DHT (temperature change in °C) - REDUCED
#define DEBOUNCE_TIME 50    // Debounce 50ms - REDUCED from 100ms
#define DHT_READ_INTERVAL 2000  // Read DHT every 2 seconds (DHT22 is slow)

// ===== GLOBAL VARIABLES =====
DHT dht(DHT_PIN, DHT_TYPE);

// Sensor 1 (Potentiometer)
volatile int lastADC1Value = 0;
volatile bool adc1Changed = false;
unsigned long lastADC1ChangeTime = 0;

// Sensor 2 (DHT22)
volatile float lastDHTTemp = 0;
volatile bool dht2Changed = false;
unsigned long lastDHT2ChangeTime = 0;

// Response Time
volatile unsigned long responseTime = 0;
int firstSensorTriggered = 0;  // 1=ADC1, 2=DHT
unsigned long lastDHTReadTime = 0;  // Track DHT read interval

// ===== INTERRUPT HANDLER UNTUK ADC1 =====
void IRAM_ATTR handleADC1Change() {
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
            responseTime = currentTime - lastDHT2ChangeTime;
            firstSensorTriggered = 1;
        } else {
            firstSensorTriggered = 1;
            responseTime = 0;
        }
    }
}

// ===== INTERRUPT HANDLER UNTUK DHT22 =====
void IRAM_ATTR handleDHT2Change() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastDHT2ChangeTime < DEBOUNCE_TIME) {
        return;
    }
    
    float currentTemp = dht.readTemperature();
    
    if (isnan(currentTemp)) {
        return;
    }
    
    if (abs(currentTemp - lastDHTTemp) >= THRESHOLD_DHT) {
        lastDHTTemp = currentTemp;
        dht2Changed = true;
        lastDHT2ChangeTime = currentTime;
        
        if (firstSensorTriggered == 1) {
            responseTime = currentTime - lastADC1ChangeTime;
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
    
    Serial.println("\n=====================================");
    Serial.println("Dual Sensor Monitoring System");
    Serial.println("Sensor 1: Potentiometer");
    Serial.println("Sensor 2: DHT22 (Temperature)");
    Serial.println("=====================================\n");
    
    // Initialize Potentiometer ADC
    analogReadResolution(12);  // 12-bit (0-4095)
    analogSetAttenuation(ADC_11db);
    
    // Initialize DHT22
    dht.begin();
    delay(500);
    
    // Read initial values
    lastADC1Value = analogRead(ADC_PIN_1);
    lastDHTTemp = dht.readTemperature();
    lastADC1ChangeTime = millis();
    lastDHT2ChangeTime = millis();
    lastDHTReadTime = millis();  // Initialize DHT read time
    
    Serial.print("Initial Potentiometer Value: ");
    Serial.println(lastADC1Value);
    
    Serial.print("Initial Temperature: ");
    Serial.print(lastDHTTemp);
    Serial.println(" C\n");
    
    Serial.println("Monitoring active...\n");
}

void loop() {
    // Read ADC continuously (fast sampling)
    int currentADC1 = analogRead(ADC_PIN_1);
    
    unsigned long currentTime = millis();
    
    // ===== CHECK POTENTIOMETER CHANGE =====
    if (abs(currentADC1 - lastADC1Value) >= THRESHOLD_ADC1) {
        if (currentTime - lastADC1ChangeTime >= DEBOUNCE_TIME) {
            handleADC1Change();
        }
    }
    
    // ===== CHECK DHT22 CHANGE (Only read every 2 seconds to save time) =====
    if (currentTime - lastDHTReadTime >= DHT_READ_INTERVAL) {
        lastDHTReadTime = currentTime;
        
        float currentTemp = dht.readTemperature();
        
        if (!isnan(currentTemp)) {
            if (abs(currentTemp - lastDHTTemp) >= THRESHOLD_DHT) {
                if (currentTime - lastDHT2ChangeTime >= DEBOUNCE_TIME) {
                    handleDHT2Change();
                }
            }
        }
    }
    
    // ===== DISPLAY DATA WHEN CHANGE DETECTED =====
    if (adc1Changed || dht2Changed) {
        adc1Changed = false;
        dht2Changed = false;
        
        // Calculate voltage for ADC1
        float voltage1 = lastADC1Value * 3.3 / 4095.0;
        
        // Display header
        Serial.println("\n--- SENSOR DATA UPDATE ---");
        
        // Display ADC1 Data
        Serial.print("ADC Value 1: ");
        Serial.println(lastADC1Value);
        
        // Display DHT22 Data
        Serial.print("ADC Value 2: ");
        Serial.println((int)lastDHTTemp);  // Convert to integer for ADC-like format
        
        // Display Response Time
        Serial.print("Response Time: ");
        Serial.print(responseTime);
        Serial.println(" ms");
        
        Serial.println("--- END ---\n");
    }
    
    delay(20);  // Faster sampling interval - REDUCED from 50ms
}
