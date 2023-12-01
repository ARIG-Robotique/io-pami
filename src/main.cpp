#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

#define DEBUG
#define I2C_ADD 0x55
#define SERVO1 12
#define SERVO2 11
#define INPUT_D1 9
#define INPUT_D2 8
#define INPUT_D3 7
#define INPUT_A1 A0
#define INPUT_A2 A1
#define INPUT_A3 A2

bool val_d1 = 0;
bool val_d2 = 0;
bool val_d3 = 0;
int val_a1 = 0;
int val_a2 = 0;
int val_a3 = 0;
int val_servo1 = DEFAULT_PULSE_WIDTH;
int val_servo2 = DEFAULT_PULSE_WIDTH;

bool servo_init = false;
Servo servo1;
Servo servo2;

// https://github.com/nickgammon/I2C_Anything
template <typename T>
unsigned int I2C_writeAnything(const T &value)
{
  return Wire.write((byte *)&value, sizeof(value));
}

template <typename T>
unsigned int I2C_readAnything(T &value)
{
  byte *p = (byte *)&value;
  unsigned int i;
  for (i = 0; i < sizeof value; i++)
    *p++ = Wire.read();
  return i;
}

void processRequest(int length, boolean wire)
{
  char c = wire ? Wire.read() : Serial.read();

  switch (c)
  {
  case 'S':
    if (wire)
    {
      I2C_readAnything(val_servo1);
      I2C_readAnything(val_servo2);
    }
    else
    {
#ifdef DEBUG
      Serial.println(F("Pas implémenté"));
#endif
    }

    servo1.writeMicroseconds(val_servo1);
    servo2.writeMicroseconds(val_servo2);

    if (!servo_init)
    {
      servo1.attach(SERVO1);
      servo2.attach(SERVO2);
      servo_init = true;
    }
    break;

  case 'D':
    servo1.detach(SERVO1);
    servo2.detach(SERVO2);
    servo_init = false;
    break;

  case 'F': // Serial only
    if (!wire)
    {
      processResponse(false);
      break;
    }

  default:
#ifdef DEBUG
    Serial.print(F("Requete inconnue "));
    Serial.println(c);
#endif
    break;
  }
}

void processResponse(bool wire)
{
  if (wire)
  {
    I2C_writeAnything(val_d1);
    I2C_writeAnything(val_d2);
    I2C_writeAnything(val_d3);
    I2C_writeAnything(val_a1);
    I2C_writeAnything(val_a2);
    I2C_writeAnything(val_a3);
  }
  else
  {
#ifdef DEBUG
    Serial.println(val_d1);
    Serial.println(val_d2);
    Serial.println(val_d3);
    Serial.println(val_a1);
    Serial.println(val_a2);
    Serial.println(val_a3);
#endif
  }
}

void I2C_RxHandler(int numBytes)
{
  processRequest(numBytes, true);
}

void I2C_TxHandler(void)
{
  processResponse(true);
}

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif

  pinMode(INPUT_D1, INPUT_PULLUP);
  pinMode(INPUT_D2, INPUT_PULLUP);
  pinMode(INPUT_D3, INPUT_PULLUP);

  Wire.begin(I2C_ADD);
  Wire.onReceive(I2C_RxHandler);
  Wire.onRequest(I2C_TxHandler);
}

void loop()
{
#if defined(DEBUG)
  if (Serial.available())
  {
    processRequest(7, false);
  }
#endif

  val_d1 = digitalRead(INPUT_D1);
  val_d2 = digitalRead(INPUT_D2);
  val_d3 = digitalRead(INPUT_D3);

  val_a1 = analogRead(INPUT_A1);
  val_a2 = analogRead(INPUT_A2);
  val_a3 = analogRead(INPUT_A3);
}
