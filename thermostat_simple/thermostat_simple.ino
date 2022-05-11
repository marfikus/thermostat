
#include <util/delay.h>


// Pin config for Arduino (ATmega328P):
#define REGULATOR A0
#define SENSOR A1
#define HEATER 2
#define LED 3

/*
// Pin config for ATtiny13A:
#define REGULATOR 3 // ADC3 on ATtiny13 (pin 2)
#define SENSOR 2 // ADC2 on ATtiny13 (pin 3)
#define HEATER 0 // PB0 on ATtiny13 (pin 5)
#define LED PB1 // PB1 on ATtiny13 (pin 6)
*/

#define REGULATOR_MIN 0
#define REGULATOR_MAX 1023

#define SENSOR_FAULT 300 // ниже этого значения считаем терморезистор в обрыве
#define SENSOR_MIN 540 // ~27 degrees Celsius
// #define SENSOR_MAX 820 // ~70 degrees Celsius
#define SENSOR_MAX 910 // ~100 degrees Celsius

#define HYSTERESIS 0
#define TRIGGER_COUNTER_MAX 5 // количество срабатываний по достижению пороговой температуры (защита от ложных)

#define MAIN_LOOP_DELAY_MS 500

bool ledIsOn = false;
byte upTriggerCounter = 0;
byte downTriggerCounter = 0;


void turnOnHeater() {
    digitalWrite(HEATER, HIGH); // for Arduino
    // PORTB |= (1 << HEATER); // for ATtiny13A
}

void turnOffHeater() {
    digitalWrite(HEATER, LOW); // for Arduino
    // PORTB &= ~(1 << HEATER); // for ATtiny13A
}

void turnOnLed() {
    digitalWrite(LED, HIGH); // for Arduino
    // PORTB |= (1 << LED); // for ATtiny13A
    ledIsOn = true;
}

void turnOffLed() {
    digitalWrite(LED, LOW); // for Arduino
    // PORTB &= ~(1 << LED); // for ATtiny13A
    ledIsOn = false;
}

void toggleLed() {
    if (ledIsOn) {
        turnOffLed();
    } else {
        turnOnLed();
    }
}

long mapToSensorValue(long regulatorValue) {
    long mapped = ((regulatorValue - REGULATOR_MIN) * (SENSOR_MAX - SENSOR_MIN) 
        / (REGULATOR_MAX - REGULATOR_MIN)) + SENSOR_MIN;
    return mapped;
}

void setup() {

    // for Arduino:
    pinMode(REGULATOR, INPUT);
    pinMode(SENSOR, INPUT);
    pinMode(HEATER, OUTPUT);
    pinMode(LED, OUTPUT);

    digitalWrite(HEATER, LOW);
    digitalWrite(LED, LOW);

/*
    // for ATtiny13A:
    DDRB &= ~(1 << REGULATOR);
    DDRB &= ~(1 << SENSOR);
    DDRB |= (1 << HEATER);
    DDRB |= (1 << LED);

    PORTB &= ~(1 << HEATER);
    PORTB &= ~(1 << LED);
*/

    ledIsOn = false;
    upTriggerCounter = 0;
    downTriggerCounter = 0;

    Serial.begin(9600);
    Serial.println("ready");
}

void loop() {
    int sensorValue = analogRead(SENSOR);
    Serial.print("sensor: ");
    Serial.print(sensorValue);

    if (sensorValue < SENSOR_FAULT) {
        turnOffHeater();
        toggleLed();
        upTriggerCounter = 0;
        downTriggerCounter = 0;
        Serial.print(" Temp sensor fault!");
    } else {

        int regulatorValue = analogRead(REGULATOR);
        Serial.print("  regulator: ");
        Serial.print(regulatorValue);

        if (regulatorValue > 0) {
            long mappedRegulatorValue = mapToSensorValue(regulatorValue);
            Serial.print("  mappedRegulator: ");
            Serial.print(mappedRegulatorValue);

            if (sensorValue - HYSTERESIS > mappedRegulatorValue) {
                downTriggerCounter = 0;
                upTriggerCounter++;

                if (upTriggerCounter >= TRIGGER_COUNTER_MAX) {
                    turnOffHeater();
                    turnOffLed();
                    upTriggerCounter = 0;
                    Serial.print("  heater is off");
                }

            } else if (sensorValue + HYSTERESIS < mappedRegulatorValue) {
                upTriggerCounter = 0;
                downTriggerCounter++;

                if (downTriggerCounter >= TRIGGER_COUNTER_MAX) {
                    turnOnHeater();
                    turnOnLed();
                    downTriggerCounter = 0;
                    Serial.print("  heater is on");
                }

            } else { // если в зоне гистерезиса
                upTriggerCounter = 0;
                downTriggerCounter = 0;
            }

        } else {
            turnOffHeater();
            turnOffLed();
            upTriggerCounter = 0;
            downTriggerCounter = 0;
        }
    }

    Serial.println("");
    _delay_ms(MAIN_LOOP_DELAY_MS);
}
