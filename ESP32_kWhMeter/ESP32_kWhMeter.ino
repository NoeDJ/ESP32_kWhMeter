#include <LiquidCrystal_I2C.h>
#include <PZEM004Tv30.h>
#include "EEPROM.h"
#define I2CADDR 0x20 // Set the Address of the PCF8574

#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

String IdKamar = "1.G";

int eeAddress = 0; // eeprom eddress
#define EEPROM_SIZE 12
// int boardId = 18;

int relayPinOut = 2;
// int contact = 0;
int decimalValue = 0;
float inputInt;

// String passWordString = "123456";
// String inputString = "";
// String inputString2 = "";
// String inputString3 = "";
// String androidInput = "";
// String androidInput2 = "";
String inData;

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long startMillis;
unsigned long currentMillis;
unsigned long period = 0.1; //0.001
// int getNextkey = 0;

// PZEM004Tv30 pzem(14, 12);
PZEM004Tv30 pzem(&Serial2);
float energy; // kWh
float power;  // Watt
// float token_spent; // satuan kWh
float token_remaining = 0.00; //  token Value satuan kWh
float token_remaining2;
// float pricePerkWh = 0.00; // price per kWh

// BUZZER
int pinBuzzer = 13;

void setup()
{
  //  delay(1000);
  Serial.begin(115200);
  SerialBT.begin(IdKamar); // Bluetooth device name
  while (!Serial)
    ;
  // set up the LCD's number of columns and rows:
  //  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Kamar : " + IdKamar);
  // relaysetup
  pinMode(relayPinOut, OUTPUT);
  digitalWrite(relayPinOut, HIGH);

  startMillis = millis();

  // Load eeprom token
  EEPROM.begin(EEPROM_SIZE);
  float token_saveddata;
  token_saveddata = EEPROM.readFloat(eeAddress);
  token_remaining = token_saveddata;
  // Serial.println(token_remaining, 4);

  // pinBuzzer
  pinMode(pinBuzzer, OUTPUT);
}

void loop()
{
  currentMillis = millis();
  Serial.print("token remaining: ");
  Serial.println(token_remaining);
  energy = pzem.energy();
  power = pzem.power();
  int powerDecimalValue = getDecimalValue(power);
  int energyDecimalValue = getDecimalValue(power);
  int token_remainingDecimalValue = getDecimalValue(token_remaining);
  if (currentMillis - startMillis >= period)
  {
    if (!isnan(energy))
    {
      bluetoothKey();
      token_remaining2 = token_remaining - energy;
      // Serial.print("Read TOken: ");
      // Serial.println(token_remaining2);
      if (token_remaining2 <= 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("                     ");
        lcd.setCursor(0, 1);
        lcd.print("token habis");
        delay(100);
      }
      else if ((token_remaining2 <= 5) && (token_remaining2 > 0)) // harus disesuaikan lagi
      {
        digitalWrite(pinBuzzer, HIGH);
        delay(1000);
        digitalWrite(pinBuzzer, LOW);
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("               ");
        lcd.setCursor(0, 1);
        lcd.print("Token :");
        lcd.setCursor(8, 1);
        lcd.print(token_remaining2, token_remainingDecimalValue);
        lcd.print("kWh");
      }
      else
      {
        lcd.setCursor(0, 1);
        lcd.print("               ");
        lcd.setCursor(0, 1);
        lcd.print("Token :");
        lcd.setCursor(8, 1);
        lcd.print(token_remaining2, token_remainingDecimalValue);
        lcd.print("kWh");
        delay(100);
      }
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("                     ");
      lcd.setCursor(0, 1);
      lcd.print("Error! No Power");
      delay(100);
    }
    startMillis = currentMillis;
  }
  if (token_remaining2 <= 0)
  {
    pzem.resetEnergy();
    digitalWrite(relayPinOut, LOW);
  }
  else
  {
    digitalWrite(relayPinOut, HIGH);
  }
  // bluetoothKey();
}

float getDecimalValue(float power)
{
  // float power = pzem.power() * 1000;
  if (power < 10)
  {
    decimalValue = 3;
  }
  else if (10 <= power && power < 100) // x < 5 &&  x < 10
  {
    decimalValue = 2;
  }
  else if (100 <= power && power < 1000) // x < 5 &&  x < 10
  {
    decimalValue = 1;
  }
  else
  {
    decimalValue = 0;
  }
  return decimalValue;
}

void bluetoothKey()
{

  if (SerialBT.available())
  {
    char recieved = SerialBT.read();
    inData += recieved;
    if (recieved == '\n')
    {
      inputInt = inData.toFloat();
      if (inputInt == 0)
      {
        token_remaining = 0.00;
        token_remaining2 = token_remaining;
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.writeFloat(eeAddress, token_remaining); // EEPROM.put(address, param);
        EEPROM.commit();
      }
      else
      {
        token_remaining = token_remaining + inputInt;
        // token_remaining2 = token_remaining;
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.writeFloat(eeAddress, token_remaining); // EEPROM.put(address, param);
        EEPROM.commit();
      }
      inData = "";
    }
  }
}
