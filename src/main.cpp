#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <I2C_Anything.h>

#define I2C_ADD 0x55
#define SERVO1 2
#define SERVO2 3
#define INPUT1 4
#define INPUT2 5
#define INPUT3 6
#define GP2D1 A0
#define GP2D2 A2
#define GP2D3 A7

bool val_input1 = false;
bool val_input2 = false;
bool val_input3 = false;
uint8_t val_gp2d1 = 0;
uint8_t val_gp2d2 = 0;
uint8_t val_gp2d3 = 0;
int val_servo1 = DEFAULT_PULSE_WIDTH;
int val_servo2 = DEFAULT_PULSE_WIDTH;

bool servo_init = false;
Servo servo1;
Servo servo2;
String firmwareVersion;

void processResponse(bool wire) {
    byte inputs = val_input1 ? 1 : 0;
    inputs += val_input2 ? 2 : 0;
    inputs += val_input3 ? 4 : 0;
    if (wire) {
        I2C_writeAnything(inputs);
        I2C_writeAnything(val_gp2d1);
        I2C_writeAnything(val_gp2d2);
        I2C_writeAnything(val_gp2d3);
    } else {
#ifdef DEBUG
        Serial.println("Raw inputs :");
        Serial.println(val_input1);
        Serial.println(val_input2);
        Serial.println(val_input3);

        Serial.println("Inputs computed :");
        Serial.println(inputs);

        Serial.println("GP2Ds");
        Serial.println(val_gp2d1);
        Serial.println(val_gp2d2);
        Serial.println(val_gp2d3);
#endif
    }
}

void processRequest(int length, boolean wire) {
    char c = wire ? Wire.read() : Serial.read();

    switch (c) {
        case 'S':
            if (wire) {
                I2C_readAnything(val_servo1);
                I2C_readAnything(val_servo2);
            } else {
#ifdef DEBUG
                val_servo1 = Serial.parseInt(SKIP_ALL, '\n');
                val_servo2 = Serial.parseInt(SKIP_ALL, '\n');
#endif
            }

            servo1.writeMicroseconds(val_servo1);
            servo2.writeMicroseconds(val_servo2);

            if (!servo_init) {
                servo1.attach(SERVO1, 500, 2500);
                servo2.attach(SERVO2, 500, 2500);
                servo_init = true;
            }
            break;

        case 'D':
            servo1.detach();
            servo2.detach();
            servo_init = false;
            break;

        case 'F': // Serial only
            if (!wire) {
                processResponse(false);
                break;
            }

        case 'V':
            if (wire) {
                I2C_writeAnything(firmwareVersion);
            } else {
#ifdef DEBUG
                Serial.print("Version : ");
                Serial.println(firmwareVersion);
#endif
            }
            break;

        default:
#ifdef DEBUG
            Serial.print(F("Requete inconnue "));
            Serial.println(c);
#endif
            break;
    }
}

void I2C_RxHandler(int numBytes) {
    processRequest(numBytes, true);
}

void I2C_TxHandler(void) {
    processResponse(true);
}

// https://swanrobotics.com/projects/gp2d12_project/
uint8_t readGP2D(uint8_t pin) {
    int val = analogRead(pin);
    return round(6787.0 / (val - 3.0)) - 4.0;
}

void setup() {
#ifdef DEBUG
    Serial.begin(115200);
    delay(2000);
    Serial.println("Setup start :");
#endif

#ifdef DEBUG
    Serial.println(" * Input configuration ...");
#endif
    pinMode(INPUT1, INPUT_PULLUP);
    pinMode(INPUT2, INPUT_PULLUP);
    pinMode(INPUT3, INPUT_PULLUP);

#ifdef DEBUG
    Serial.println(" * I2C master configuration ...");
    Serial.print("   -> I2C master address : 0x");
    Serial.println(I2C_ADD, HEX);
#endif
    Wire.begin(I2C_ADD);
    Wire.onReceive(I2C_RxHandler);
    Wire.onRequest(I2C_TxHandler);

    // Compute version String
#ifdef DEBUG
    Serial.println(" * Compute firmware version ...");
#endif
    firmwareVersion = String(TIMESTAMP) + "-" + String(COMMIT_HASH);
#ifdef DEBUG
    Serial.print("   -> Version : ");
    Serial.println(firmwareVersion);
#endif
}

void loop() {
#if defined(DEBUG)
    if (Serial.available()) {
        processRequest(7, false);
    }
#endif

    // The micro switch is connected on ground.
    // The inputs on controller have a pullup configured by hardware
    val_input1 = digitalRead(INPUT1) == LOW;
    val_input2 = digitalRead(INPUT2) == LOW;
    val_input3 = digitalRead(INPUT3) == LOW;

    val_gp2d1 = readGP2D(GP2D1);
    val_gp2d2 = readGP2D(GP2D2);
    val_gp2d3 = readGP2D(GP2D3);
}
