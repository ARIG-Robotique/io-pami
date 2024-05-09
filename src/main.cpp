#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <I2C_Anything.h>
#include <FastLED.h>

#define F_CPU 16000000L

#define I2C_ADD 0x55
#define SERVO1 5 // Gauche
#define SERVO2 6 // Droite
#define INPUT1 2 // uS Gauche
#define INPUT2 4 // uS Droite
#define PIN_WS2812 3 // PWM WS2812
#define GP2D1 A0 // Gauche
#define GP2D2 A7 // Centre
#define GP2D3 A3 // Droite

#define NUM_LEDS 3

CRGB leds[NUM_LEDS];

bool alt = true;
bool needLedRefresh = false;

bool val_input1 = false;
bool val_input2 = false;

int val_gp2d1 = 0;
int val_gp2d2 = 0;
int val_gp2d3 = 0;
int val_servo1 = DEFAULT_PULSE_WIDTH;
int val_servo2 = DEFAULT_PULSE_WIDTH;

bool servo_init = false;
Servo servo1;
Servo servo2;
String firmwareVersion;

void setLedColor(uint8_t id, uint8_t colorCode) {

#ifdef DEBUG
    Serial.print("Led ");
    Serial.print(id);
    Serial.print(" -> ");
    Serial.println((char) colorCode);
#endif

    CRGB color;
    switch(colorCode) {
        case 'W': color = CRGB::White;break;
        case 'R': color = CRGB::Red;break;
        case 'G': color = CRGB::Green;break;
        case 'B': color = CRGB::Blue;break;
        case 'Y': color = CRGB::Yellow;break;
        case 'K': color = CRGB::Black;break;
    }

    if (id == 0) {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = color;
        }
    } else {
        leds[id - 1] = color;
    }
    needLedRefresh = true;
}

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
#ifdef DEBUG
            Serial.println("Detach Servo 1 & 2");
#endif
            break;

        case 'F': // Serial only
            if (!wire) {
                processResponse(false);
                break;
            }

        case 'V':
            if (wire) {
                I2C_writeAnything(firmwareVersion);
            }
#ifdef DEBUG
            Serial.print("Version : ");
            Serial.println(firmwareVersion);
#endif
            break;

        case 'L':
            uint8_t id;
            uint8_t colorCode;
            if (wire) {
                I2C_readAnything(id);
                I2C_readAnything(colorCode);
            } else {
#ifdef DEBUG
                while(!Serial.available());
                id = Serial.read() - '0';
                while(!Serial.available());
                colorCode = Serial.read();
#endif
            }

            setLedColor(id, colorCode);
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
int readGP2D(uint8_t pin) {
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
    Serial.println(" * I2C slave configuration ...");
    Serial.print("   -> I2C slave address : 0x");
    Serial.println(I2C_ADD, HEX);
#endif
    Wire.begin(I2C_ADD);
    Wire.onReceive(I2C_RxHandler);
    Wire.onRequest(I2C_TxHandler);

    // Configure FastLED
#ifdef DEBUG
    Serial.println(" * Configure FastLED ...");
#endif
    FastLED.addLeds<NEOPIXEL, PIN_WS2812>(leds, NUM_LEDS);
    setLedColor(0, 'R');delay(100);
    setLedColor(0, 'G');delay(100);
    setLedColor(0, 'B');delay(100);
    setLedColor(0, 'Y');delay(100);
    setLedColor(0, 'W');delay(100);
    setLedColor(0, 'b');delay(100);

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

    EVERY_N_SECONDS(1) {
        digitalWrite(LED_BUILTIN, alt ? HIGH : LOW);
        alt = !alt;

        if (needLedRefresh) {
            needLedRefresh = false;
            FastLED.show();
        }
    }

    // The micro switch is connected on ground.
    // The inputs on controller have a pullup configured by hardware
    val_input1 = digitalRead(INPUT1) == LOW;
    val_input2 = digitalRead(INPUT2) == LOW;

    // Read GP2D sensors
    val_gp2d1 = readGP2D(GP2D1);
    val_gp2d2 = readGP2D(GP2D2);
    val_gp2d3 = readGP2D(GP2D3);

}
