#include <Arduino.h>

#define I2C_ADD 0x55
#define DIR 2
#define PWM 3


void setup() {
#ifdef DEBUG
    Serial.begin(115200);
    delay(2000);
    Serial.println("Setup start :");
#endif

#ifdef DEBUG
    Serial.println(" * Input configuration ...");
#endif
    pinMode(DIR, OUTPUT);
    pinMode(PWM, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
}

bool dir = false;
bool hb = false;
void loop() {
    digitalWrite(DIR, dir ? HIGH : LOW);
    dir = !dir;

    for (int i = 0 ; i < 128 ; i++) {
       analogWrite(PWM, i);
        digitalWrite(LED_BUILTIN, hb ? HIGH : LOW);
        hb = !hb;
       delay(100);
    }

    analogWrite(PWM, 0);
    delay(2000);
}
