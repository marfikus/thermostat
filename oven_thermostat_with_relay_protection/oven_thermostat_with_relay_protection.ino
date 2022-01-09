
/*
    В данном варианте в цепь управления нагревателем добавлен симистор, параллельно реле. 
    При включении нагревателя сначала открывается симистор, а через некоторое время замыкается реле 
    (то есть оно шунтирует симистор). Отключение происходит в обратном порядке. 
    Это позволяет защитить контакты реле от искрообразования, а также избежать 
    постоянного нагрева симистора в том случае, если бы он один коммутировал нагреватель
    (в текущем виде симистор работает только во время включения и отключения нагревателя).
*/

#include <util/delay.h>

/*
// Pin config for Arduino (ATmega328P):
#define REGULATOR A0
#define SENSOR A1
#define HEATER_RELAY 2
#define HEATER_TRIAC 3
#define FAULT_LED 4
*/

// Pin config for ATtiny13A:
#define REGULATOR 3 // ADC3 on ATtiny13 (pin 2)
#define SENSOR 2 // ADC2 on ATtiny13 (pin 3)
#define HEATER_RELAY PB0 // PB0 on ATtiny13 (pin 5)
#define HEATER_TRIAC PB1 // PB1 on ATtiny13 (pin 6)
#define FAULT_LED PB2 // PB2 on ATtiny13 (pin 7)


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

void turnOnHeater() {
    digitalWrite(HEATER_TRIAC, HIGH);
    _delay_ms(500);
    digitalWrite(HEATER_RELAY, HIGH);
}

void turnOffHeater() {
    digitalWrite(HEATER_RELAY, LOW);
    _delay_ms(500);
    digitalWrite(HEATER_TRIAC, LOW);
}

void setup() {
    pinMode(REGULATOR, INPUT);
    pinMode(SENSOR, INPUT);
    pinMode(HEATER_RELAY, OUTPUT);
    pinMode(HEATER_TRIAC, OUTPUT);
    pinMode(FAULT_LED, OUTPUT);

    digitalWrite(HEATER_RELAY, LOW);
    digitalWrite(HEATER_TRIAC, LOW);
    digitalWrite(FAULT_LED, LOW);

    // Serial.begin(9600);
    // Serial.println("ready");
}

void loop() {
    int sensorValue = analogRead(SENSOR);
    // Serial.print("sensor: ");
    // Serial.print(sensorValue);

    if (sensorValue < SENSOR_FAULT) {
        turnOffHeater();
        digitalWrite(FAULT_LED, HIGH);
        // Serial.print(" Temp sensor fault!");
    } else {
        digitalWrite(FAULT_LED, LOW);

        int regulatorValue = analogRead(REGULATOR);
        // Serial.print("  regulator: ");
        // Serial.print(regulatorValue);

        if (regulatorValue > 0) {
            long mappedRegulatorValue = mapToSensorValue(regulatorValue);
            // Serial.print("  mappedRegulator: ");
            // Serial.print(mappedRegulatorValue);

            if (sensorValue - HYSTERESIS >= mappedRegulatorValue) {
                turnOffHeater();
                // Serial.print("  HEATER is off");
            } else if (sensorValue + HYSTERESIS <= mappedRegulatorValue) {
                turnOnHeater();
                // Serial.print("  HEATER is on");
            }

        } else {
            turnOffHeater();
        }
    }

    // Serial.println("");
    _delay_ms(500);
}
