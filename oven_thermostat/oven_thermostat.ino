
#include <util/delay.h>


// Pin config for Arduino (ATmega328P):
#define REGULATOR A0
#define SENSOR A1
#define HEATER 2
#define FAULT_LED 3

/*
// Pin config for ATtiny13A:
#define REGULATOR 3 // ADC3 on ATtiny13 (pin 2)
#define SENSOR 2 // ADC2 on ATtiny13 (pin 3)
#define HEATER 0 // PB0 on ATtiny13 (pin 5)
#define FAULT_LED PB1 // PB1 on ATtiny13 (pin 6)
*/

#define REGULATOR_MIN 0
#define REGULATOR_MAX 1023

#define SENSOR_FAULT 300 // ниже этого значения считаем терморезистор в обрыве
#define SENSOR_MIN 540 // ~27 degrees Celsius
// #define SENSOR_MAX 820 // ~70 degrees Celsius
#define SENSOR_MAX 910 // ~100 degrees Celsius

#define HYSTERESIS 10


long mapToSensorValue(long regulatorValue) {
    long mapped = ((regulatorValue - REGULATOR_MIN) * (SENSOR_MAX - SENSOR_MIN) 
        / (REGULATOR_MAX - REGULATOR_MIN)) + SENSOR_MIN;
    return mapped;
}

void setup() {
    pinMode(REGULATOR, INPUT);
    pinMode(SENSOR, INPUT);
    pinMode(HEATER, OUTPUT);
    pinMode(FAULT_LED, OUTPUT);

    digitalWrite(HEATER, LOW);
    digitalWrite(FAULT_LED, LOW);

    Serial.begin(9600);
    Serial.println("ready");
}

void loop() {
    int sensorValue = analogRead(SENSOR);
    Serial.print("sensor: ");
    Serial.print(sensorValue);

    if (sensorValue < SENSOR_FAULT) {
        digitalWrite(HEATER, LOW);
        digitalWrite(FAULT_LED, HIGH);
        Serial.print(" Temp sensor fault!");
    } else {
        digitalWrite(FAULT_LED, LOW);

        int regulatorValue = analogRead(REGULATOR);
        Serial.print("  regulator: ");
        Serial.print(regulatorValue);

        if (regulatorValue > 0) {
            long mappedRegulatorValue = mapToSensorValue(regulatorValue);
            Serial.print("  mappedRegulator: ");
            Serial.print(mappedRegulatorValue);

            if (sensorValue - HYSTERESIS >= mappedRegulatorValue) {
                digitalWrite(HEATER, LOW);
                Serial.print("  heater is off");
            } else if (sensorValue + HYSTERESIS <= mappedRegulatorValue) {
                digitalWrite(HEATER, HIGH);
                Serial.print("  heater is on");
            }

        } else {
            digitalWrite(HEATER, LOW);
        }
    }

    Serial.println("");
    _delay_ms(500);
}
