/*
    Дополнительно реализована возможность настройки верхней границы 
    температурного диапазона при помощи подстроечного резистора (UP_LIMIT_ADJUSTER).

*/

#include <util/delay.h>


// Pin config for Arduino (ATmega328P):
#define REGULATOR A0
#define SENSOR A1
#define UP_LIMIT_ADJUSTER A2
#define HEATER 2
#define FAULT_LED 3

/*
// Pin config for ATtiny13A:
#define REGULATOR 3 // ADC3 on ATtiny13 (pin 2)
#define SENSOR 2 // ADC2 on ATtiny13 (pin 3)
#define UP_LIMIT_ADJUSTER 1 // ADC1 on ATtiny13 (pin 7)
#define HEATER 0 // PB0 on ATtiny13 (pin 5)
#define FAULT_LED PB1 // PB1 on ATtiny13 (pin 6)
*/

// диапазон значений переменного резистора:
#define VAR_RESISTOR_MIN 0
#define VAR_RESISTOR_MAX 1023

#define SENSOR_FAULT 300 // ниже этого значения считаем терморезистор в обрыве
#define SENSOR_MIN 540 // ~27 degrees Celsius

// диапазон настройки верхней границы:
#define UP_LIMIT_MIN 700 // ~50 degrees Celsius
#define UP_LIMIT_MAX 910 // ~100 degrees Celsius

#define HYSTERESIS 10


long mapAdjusterToSensorValue(long adjusterValue) {
    long mapped = ((adjusterValue - VAR_RESISTOR_MIN) * (UP_LIMIT_MAX - UP_LIMIT_MIN) 
        / (VAR_RESISTOR_MAX - VAR_RESISTOR_MIN)) + UP_LIMIT_MIN;
    return mapped;
}

long mapToSensorValue(long regulatorValue) {
    int adjusterValue = analogRead(UP_LIMIT_ADJUSTER);
    Serial.print("  adjuster: ");
    Serial.print(adjusterValue);
    long sensorMax = mapAdjusterToSensorValue(adjusterValue);
    Serial.print("  sensorMax: ");
    Serial.print(sensorMax);

    long mapped = ((regulatorValue - VAR_RESISTOR_MIN) * (sensorMax - SENSOR_MIN) 
        / (VAR_RESISTOR_MAX - VAR_RESISTOR_MIN)) + SENSOR_MIN;
    return mapped;
}

void setup() {
    pinMode(REGULATOR, INPUT);
    pinMode(SENSOR, INPUT);
    pinMode(UP_LIMIT_ADJUSTER, INPUT);
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
