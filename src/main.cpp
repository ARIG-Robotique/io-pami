#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <I2C_Anything.h>
#include <HeartBeat.h>

#define I2C_ADD 0x55
#define SERVO1 5 // Gauche
#define SERVO2 6 // Droite
#define INPUT1 2 // uS Gauche
#define INPUT2 4 // uS Droite
#define WS2812 3 // PWM WS2812
#define GP2D1 A0 // Gauche
#define GP2D2 A7 // Centre
#define GP2D3 A3 // Droite

bool val_input1 = false;
bool val_input2 = false;

uint16_t val_gp2d1 = 0;
uint16_t val_gp2d2 = 0;
uint16_t val_gp2d3 = 0;
int val_servo1 = DEFAULT_PULSE_WIDTH;
int val_servo2 = DEFAULT_PULSE_WIDTH;

HeartBeat heartBeat;

bool servo_init = false;
Servo servo1;
Servo servo2;
String firmwareVersion;

void processResponse(bool wire) {
    byte inputs = val_input1 ? 1 : 0;
    inputs += val_input2 ? 2 : 0;
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
        case 0x31: // '1'
        case 0x32: // '2'
            int val_servo;
            if (wire) {
                I2C_readAnything(val_servo);
#ifdef DEBUG
                Serial.print("Servo ");
                Serial.print(c - 0x30, HEX);
                Serial.print(" -> 0x");
                Serial.println(val_servo, HEX);
#endif

            } else {
#ifdef DEBUG
                while(!Serial.available());
                val_servo = Serial.read();
                while(!Serial.available());
                val_servo += Serial.read() << 8;
                Serial.print("Servo ");
                Serial.print(c - 0x30, HEX);
                Serial.print(" -> 0x");
                Serial.println(val_servo, HEX);
#endif
            }
            if (c == 0x31) {
                val_servo1 = val_servo;
                servo1.writeMicroseconds(val_servo1);
            } else {
                val_servo2 = val_servo;
                servo2.writeMicroseconds(val_servo2);
            }

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
uint16_t readGP2D(uint8_t pin) {
    return analogRead(pin);
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

#ifdef DEBUG
    Serial.println(" * Output configuration ...");
#endif
    pinMode(WS2812, OUTPUT);

#ifdef DEBUG
    Serial.println(" * Heart Beat configuration ...");
    Serial.println("   -> Use LED_BUILTIN");
    Serial.println("   -> Duty Cycle 50%");
    Serial.println("   -> Frequency 1Hz");
#endif
    heartBeat.begin(LED_BUILTIN);

#ifdef DEBUG
    Serial.println(" * I2C slave configuration ...");
    Serial.print("   -> I2C slave address : 0x");
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
  // Heart Beat
  heartBeat.beat();

#if defined(DEBUG)
    if (Serial.available()) {
        processRequest(7, false);
    }
#endif

    // The micro switch is connected on ground.
    // The inputs on controller have a pullup configured by hardware
    val_input1 = digitalRead(INPUT1) == LOW;
    val_input2 = digitalRead(INPUT2) == LOW;

    val_gp2d1 = readGP2D(GP2D1);
    val_gp2d2 = readGP2D(GP2D2);
    val_gp2d3 = readGP2D(GP2D3);
}
