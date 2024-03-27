#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

#define DIRA 3
#define PWMA 2
#define DIRB 7
#define PWMB 6

#define PWM_ROUE 8
#define IN1_ROUE 10
#define IN2_ROUE 9

#define PWM_EA 12
#define IN1_EA 14
#define IN2_EA 13

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void testPin(uint8_t pin) {
    Serial.print("Pin ");
    Serial.print(pin);
    Serial.println(" ON");
    pwm.setPin(pin, 4096);
    delay(5000);
    Serial.print("Pin ");
    Serial.print(pin);
    Serial.println(" OFF");
    pwm.setPin(pin, 0);
    delay(5000);
}

void processResponse(bool wire) {

}

void processRequest(int length, boolean wire) {
    char c = wire ? Wire.read() : Serial.read();

    switch (c) {
        case 'M':
            Serial.println("Motor 1 & 2 : Forward");
            pwm.setPin(DIRA, 4096);
            pwm.setPin(DIRB, 4096);
            for (unsigned int i = 0 ; i < 1024 ; i++) {
                Serial.print("Motor 1 & 2 : ");
                Serial.println(i);
                pwm.setPin(PWMA, i);
                pwm.setPin(PWMB, i);
                delay(10);
            }
            Serial.println("Motor 1 & 2 : Stop");
            pwm.setPin(PWMA, 0);
            pwm.setPin(PWMB, 0);
            delay(2000);

            Serial.println("Motor 1 & 2 : Reverse");
            pwm.setPin(DIRA, 0);
            pwm.setPin(DIRB, 0);
            for (unsigned int i = 0 ; i < 1024 ; i++) {
                Serial.print("Motor 1 & 2 : -");
                Serial.println(i);
                pwm.setPin(PWMA, i);
                pwm.setPin(PWMB, i);
                delay(10);
            }
            Serial.println("Motor 1 & 2 : Stop");
            pwm.setPin(PWMA, 0);
            pwm.setPin(PWMB, 0);

            break;

        case 'E':
            Serial.println("Electro Aimant ON");
            pwm.setPin(PWM_EA, 4096);
            delay(2000);
            Serial.println("Electro Aimant OFF");
            pwm.setPin(PWM_EA, 0);

            break;

        case 'R':
            Serial.println("Roue : Forward");
            pwm.setPin(IN1_ROUE, 0);
            pwm.setPin(IN2_ROUE, 4096);
            for (unsigned int i = 0 ; i < 1024 ; i++) {
                Serial.print("Roue : ");
                Serial.println(i);
                pwm.setPin(PWM_ROUE, i);
                delay(10);
            }
            Serial.println("Roue : Stop");
            pwm.setPin(IN1_ROUE, 0);
            pwm.setPin(IN2_ROUE, 0);
            pwm.setPin(PWM_ROUE, 0);
            delay(2000);

            Serial.println("Motor 1 & 2 : Reverse");
            pwm.setPin(IN1_ROUE, 4096);
            pwm.setPin(IN2_ROUE, 0);
            for (unsigned int i = 0 ; i < 1024 ; i++) {
                Serial.print("Roue : -");
                Serial.println(i);
                pwm.setPin(PWM_ROUE, i);
                delay(10);
            }
            Serial.println("Roue : Stop");
            pwm.setPin(IN1_ROUE, 0);
            pwm.setPin(IN2_ROUE, 0);
            pwm.setPin(PWM_ROUE, 0);
            break;

        case 'T':
            testPin(PWMA);
            testPin(DIRA);

            testPin(PWMB);
            testPin(DIRB);

            testPin(PWM_ROUE);
            testPin(IN1_ROUE);
            testPin(IN2_ROUE);

            testPin(PWM_EA);
            testPin(IN1_EA);
            testPin(IN2_EA);

            break;
    }
}

void I2C_RxHandler(int numBytes) {
    processRequest(numBytes, true);
}

void I2C_TxHandler(void) {
    processResponse(true);
}

void setup() {
#ifdef DEBUG
    Serial.begin(115200);
    delay(2000);
    Serial.println("Setup start :");
#endif

#ifdef DEBUG
    Serial.println(" * I2C master configuration ...");
#endif
    Wire.begin();


    if (!pwm.begin()) {
        Serial.println("PCA FAILED !!");
    }
    pwm.setPWMFreq(200);
    delay(100);

    pwm.setPin(IN1_EA, 4096);
    pwm.setPin(IN2_EA, 0);
}

void loop() {
#if defined(DEBUG)
    if (Serial.available()) {
        processRequest(7, false);
    }
#endif
}
