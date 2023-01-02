// Author       : Firdaus Nuur Rhamadhan
// Project      : Bitanic 2.0
// Code Version : 1.0
// Last Update  : 
// Description  : Development process of Bitanic 2.0

#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


// Pin Declaration
#define relay1 25
#define relay2 26
#define soilMoisture1 14
#define soilMoisture2 15
#define DHTPIN 13

// LiquidCrystal_I2C Declaration
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); 

// DHT Declaration
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // pinMode declaration
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(soilMoisture1, INPUT);
  pinMode(soilMoisture2, INPUT);

  // LCD Initialization
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Modul I2C LCD"); 
  lcd.setCursor(0, 1); 
  lcd.print("www.ardutech.com");

  // DHT Initialization
  dht.begin();
}

void loop() {
  // read analog data
  float soil1 = analogRead(soilMoisture1);
  float soil2 = analogRead(soilMoisture2);
  // read data DHT
  float humidity = dht.readHumidity();
  float temperatureCelcius = dht.readTemperature();
  if (isnan(humidity) || isnan(temperatureCelcius)){
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  float indexTemperatureCelcius = dht.computeHeatIndex(temperatureCelcius, humidity, false);


  // print data soil moisture
  Serial.println("Soil Moisture 1 : "+String(soil1));
  Serial.println("Soil Moisture 2 : "+String(soil2));

  Serial.println();

  // print data DHT
  Serial.println("Humidity    : "+String(humidity)+" %");
  Serial.println("Temperature : "+String(temperatureCelcius)+" C");
  Serial.println("Heat Index  : "+String(indexTemperatureCelcius)+" C");

  
  // print data & Execite Relay
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Soil1 : "+String(soil1)+"%");
  lcd.setCursor(0, 1);
  lcd.print("Soil2 : "+String(soil2)+"%");
  delay(2000);
  digitalWrite(25, LOW);
  digitalWrite(26, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humidity : "+String(humidity)+"%");
  lcd.setCursor(0, 1);
  lcd.print("Temp : "+String(temperatureCelcius)+"C");
  delay(2000);   
}
