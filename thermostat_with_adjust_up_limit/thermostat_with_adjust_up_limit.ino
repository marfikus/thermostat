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
#define LED 3

/*
// Pin config for ATtiny13A:
#define REGULATOR 3 // ADC3 on ATtiny13 (pin 2)
#define SENSOR 2 // ADC2 on ATtiny13 (pin 3)
#define UP_LIMIT_ADJUSTER 1 // ADC1 on ATtiny13 (pin 7)
#define HEATER 0 // PB0 on ATtiny13 (pin 5)
#define LED PB1 // PB1 on ATtiny13 (pin 6)
*/

// диапазон значений переменного резистора:
#define VAR_RESISTOR_MIN 0
#define VAR_RESISTOR_MAX 1023

#define SENSOR_FAULT 300 // ниже этого значения считаем терморезистор в обрыве
#define SENSOR_MIN 540 // ~27 degrees Celsius

// диапазон настройки верхней границы:
#define UP_LIMIT_MIN 700 // ~50 degrees Celsius
#define UP_LIMIT_MAX 910 // ~100 degrees Celsius

#define HYSTERESIS 0
#define TRIGGER_COUNTER_MAX 2 // количество срабатываний по достижению пороговой температуры (защита от ложных)

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

    // for Arduino:
    pinMode(REGULATOR, INPUT);
    pinMode(SENSOR, INPUT);
    pinMode(UP_LIMIT_ADJUSTER, INPUT);
    pinMode(HEATER, OUTPUT);
    pinMode(LED, OUTPUT);

    digitalWrite(HEATER, LOW);
    digitalWrite(LED, LOW);

/*
    // for ATtiny13A:
    DDRB &= ~(1 << REGULATOR);
    DDRB &= ~(1 << SENSOR);
    DDRB &= ~(1 << UP_LIMIT_ADJUSTER);
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
    _delay_ms(500);
}
