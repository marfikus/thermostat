/*
    Дополнительно реализована возможность настройки верхней границы 
    температурного диапазона при помощи подстроечного резистора.
    Но поскольку в ATtiny13 только 2 аналоговых входа, то для неё этот вариант не подходит.

*/

const byte VAR_RESISTOR = A0;
const byte THERMO_RESISTOR = A1;
const byte ADJUST_THERMO_SPAN_RESISTOR = A2;
const byte HEATER_VT = 2;
const byte FAULT_LED = 3;

const int VAR_RESIST_MIN = 0;
const int VAR_RESIST_MAX = 1023;

const int THERMO_RESIST_FAULT = 300; // ниже этого значения считаем терморезистор в обрыве
const int THERMO_RESIST_MIN = 540; // ~27 degrees Celsius

const int ADJUST_THERMO_SPAN_MIN = 700; // ~50 degrees Celsius
const int ADJUST_THERMO_SPAN_MAX = 910; // ~100 degrees Celsius

const byte HYSTERESIS = 10;


long convertSpanMaxToThermoResist(long varResist) {
    long thermoResist = ((varResist - VAR_RESIST_MIN) * (ADJUST_THERMO_SPAN_MAX - ADJUST_THERMO_SPAN_MIN) 
        / (VAR_RESIST_MAX - VAR_RESIST_MIN)) + ADJUST_THERMO_SPAN_MIN;
    return thermoResist;
}

long convertToThermoResist(long varResist) {
    int adjustThermoSpanResist = analogRead(ADJUST_THERMO_SPAN_RESISTOR);
    Serial.print("  adjust: ");
    Serial.print(adjustThermoSpanResist);
    long thermoResistMax = convertSpanMaxToThermoResist(adjustThermoSpanResist);
    Serial.print("  convertedAdjust: ");
    Serial.print(thermoResistMax);

    long thermoResist = ((varResist - VAR_RESIST_MIN) * (thermoResistMax - THERMO_RESIST_MIN) 
        / (VAR_RESIST_MAX - VAR_RESIST_MIN)) + THERMO_RESIST_MIN;
    return thermoResist;
}

void setup() {
    pinMode(VAR_RESISTOR, INPUT);
    pinMode(THERMO_RESISTOR, INPUT);
    pinMode(ADJUST_THERMO_SPAN_RESISTOR, INPUT);
    pinMode(HEATER_VT, OUTPUT);
    pinMode(FAULT_LED, OUTPUT);

    digitalWrite(HEATER_VT, LOW);
    digitalWrite(FAULT_LED, LOW);

    Serial.begin(9600);
    Serial.println("ready");
}

void loop() {
    long thermoResist = analogRead(THERMO_RESISTOR);
    Serial.print("thermo: ");
    Serial.print(thermoResist);

    if (thermoResist < THERMO_RESIST_FAULT) {
        digitalWrite(HEATER_VT, LOW);
        digitalWrite(FAULT_LED, HIGH);
        Serial.print(" Thermo resistor fault!");
    } else {
        digitalWrite(FAULT_LED, LOW);

        long varResist = analogRead(VAR_RESISTOR);
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
    delay(500);
}
