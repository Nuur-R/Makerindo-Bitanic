// Author       : Firdaus Nuur Rhamadhan
// Project      : Bitanic 2.0
// Code Version : 1.0
// Last Update  : 
// Description  : Development process of Bitanic 2.0

#include <Arduino.h>
#include <Wire.h> // Library komunikasi I2C 
#include <LiquidCrystal_I2C.h> // Library modul I2C LCD


// Pin Declaration
#define relay1 25
#define relay2 26


LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); 

void setup() {
  Serial.begin(115200);

  // pinMode declaration
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Modul I2C LCD"); 
  lcd.setCursor(0, 1); 
  lcd.print("www.ardutech.com");
}

void loop() {
  delay(1000);       
  Serial.println("Hallo, Guys");
  digitalWrite(25, HIGH);
  delay(1000);
  Serial.println("Ardutech.com");
  digitalWrite(25, LOW);
}
