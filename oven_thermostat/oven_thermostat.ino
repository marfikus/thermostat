
#include <util/delay.h>


// Pin config for Arduino (ATmega328P):
#define VAR_RESISTOR A0
#define THERMO_RESISTOR A1
#define HEATER_VT 2
#define FAULT_LED 3

/*
// Pin config for ATtiny13A:
#define VAR_RESISTOR 3 // ADC3 on ATtiny13 (pin 2)
#define THERMO_RESISTOR 2 // ADC2 on ATtiny13 (pin 3)
#define HEATER_VT 0 // PB0 on ATtiny13 (pin 5)
#define FAULT_LED PB1 // PB1 on ATtiny13 (pin 6)
*/

#define VAR_RESIST_MIN 0
#define VAR_RESIST_MAX 1023

#define THERMO_RESIST_FAULT 300 // ниже этого значения считаем терморезистор в обрыве
#define THERMO_RESIST_MIN 540 // ~27 degrees Celsius
// #define THERMO_RESIST_MAX 820 // ~70 degrees Celsius
#define THERMO_RESIST_MAX 910 // ~100 degrees Celsius

#define HYSTERESIS 10


long convertToThermoResist(long varResist) {
    long thermoResist = ((varResist - VAR_RESIST_MIN) * (THERMO_RESIST_MAX - THERMO_RESIST_MIN) 
        / (VAR_RESIST_MAX - VAR_RESIST_MIN)) + THERMO_RESIST_MIN;
    return thermoResist;
}

void setup() {
    pinMode(VAR_RESISTOR, INPUT);
    pinMode(THERMO_RESISTOR, INPUT);
    pinMode(HEATER_VT, OUTPUT);
    pinMode(FAULT_LED, OUTPUT);

    digitalWrite(HEATER_VT, LOW);
    digitalWrite(FAULT_LED, LOW);

    Serial.begin(9600);
    Serial.println("ready");
}

void loop() {
    int thermoResist = analogRead(THERMO_RESISTOR);
    Serial.print("thermo: ");
    Serial.print(thermoResist);

    if (thermoResist < THERMO_RESIST_FAULT) {
        digitalWrite(HEATER_VT, LOW);
        digitalWrite(FAULT_LED, HIGH);
        Serial.print(" Thermo resistor fault!");
    } else {
        digitalWrite(FAULT_LED, LOW);

        int varResist = analogRead(VAR_RESISTOR);
        Serial.print("  var: ");
        Serial.print(varResist);

        if (varResist > 0) {
            long convertedVarResist = convertToThermoResist(varResist);
            Serial.print("  converted: ");
            Serial.print(convertedVarResist);

            if (thermoResist - HYSTERESIS >= convertedVarResist) {
                digitalWrite(HEATER_VT, LOW);
                Serial.print("  heater is off");
            } else if (thermoResist + HYSTERESIS <= convertedVarResist) {
                digitalWrite(HEATER_VT, HIGH);
                Serial.print("  heater is on");
            }

        } else {
            digitalWrite(HEATER_VT, LOW);
        }
    }

    Serial.println("");
    _delay_ms(500);
}
