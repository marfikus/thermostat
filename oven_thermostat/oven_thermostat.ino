

const byte VAR_RESISTOR = A0;
const byte THERMO_RESISTOR = A1;
const byte HEATER_VT = 4;
const byte FAULT_LED = 3;

const int VAR_RESIST_MIN = 0;
const int VAR_RESIST_MAX = 1023;

const int THERMO_RESIST_FAULT = 300; // ниже этого значения считаем терморезистор в обрыве
const int THERMO_RESIST_MIN = 540; // ~27 degrees Celsius
// const int THERMO_RESIST_MAX = 820; // ~70 degrees Celsius
const int THERMO_RESIST_MAX = 910; // ~100 degrees Celsius

const byte HYSTERESIS = 10;


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
