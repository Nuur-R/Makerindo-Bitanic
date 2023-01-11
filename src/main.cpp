// Author       : Firdaus Nuur Rhamadhan
// Project      : Bitanic 2.0
// Code Version : 1.0
// Last Update  :
// Description  : Development process of Bitanic 2.0

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <RTClib.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

// WiFi Setting
const char* ssid = "Setting";
const char* password = "admin1234";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Publis Topic
const char* dataTopic = "bitanicv2/data";
// Subscribe Topic
const char* control = "bitanicv2/control";

// Pin Declaration
#define relay1 25
#define relay2 26
#define soilMoisture1 13
#define soilMoisture2 14
#define DHTPIN 15
#define Buzzer 12

// Variable Declaration
int soilMoisture1Value = 0;
int soilMoisture2Value = 0;
float humidity = 0;
float temperature = 0;
float heatIndex = 0;
String timeNow = "";
String dateNow = "";

// LiquidCrystal_I2C Declaration
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

// DHT Declaration
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// RTC Declaration
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu", "Minggu"};

// Class untuk mengeksekusi program secara multiprocessing
class Task
{
public:
  Task(unsigned long interval, void (*callback)())
  {
    this->interval = interval;
    this->callback = callback;
    this->nextRun = millis() + interval;
  }

  void update()
  {
    if (millis() >= nextRun)
    {
      nextRun = millis() + interval;
      callback();
    }
  }

private:
  unsigned long interval;
  unsigned long nextRun;
  void (*callback)();
};


void buzz(int delayTime, int repeat)
{
  for (int i = 0; i < repeat; i++)
  {
    digitalWrite(Buzzer, HIGH);
    delay(delayTime);
    digitalWrite(Buzzer, LOW);
    delay(delayTime);
  }
}

void serialDataPrint()
{
  Serial.println("====================================");
  Serial.println("          Data Bitanic 2.0");
  Serial.println("====================================");
  Serial.println("Waktu : " + timeNow + " | " + dateNow);
  Serial.println("Soil Moisture 1 : " + String(soilMoisture1Value));
  Serial.println("Soil Moisture 2 : " + String(soilMoisture2Value));
  Serial.println("Suhu            : " + String(temperature) + " C");
  Serial.println("Kelembaban      : " + String(humidity) + " %");
  Serial.println("Indeks Panas    : " + String(heatIndex) + " C");
  Serial.println("====================================");
  Serial.println();
}

void lcdPrint(String text1, String text2)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text1);
  lcd.setCursor(0, 1);
  lcd.print(text2);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("arduinoClientABC123")) {
      client.subscribe(control);
    } else {
      delay(1000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Received: " + message);
}

void sendData(const char* topic, const char* payload) {
  client.publish(topic, payload);
  Serial.println("Sent: " + String(payload));
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  // pinMode declaration
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(soilMoisture1, INPUT);
  pinMode(soilMoisture2, INPUT);

  // LCD Initialization
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // DHT Initialization
  dht.begin();

// RTC Initialization
#ifndef ESP8266
  while (!Serial)
    ; // wait for serial port to connect. Needed for native USB
#endif
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
  }
  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcdPrint("Bitaniv V2.0", "Start...");
  buzz(100, 3);
}

// task untuk mencetak hello world pada serial monitor per 10 detik sekali
Task task1(10000, []() {
  Serial.println("Hello World!");
});
Task task2(1000, []() {
  Serial.println("Hello");
});

Task dataUpdate(3000,[](){
  DateTime now = rtc.now(); // get the current time
  // = = = = = = = = = = = = = variable feed = = = = = = = = = = = = = 
  soilMoisture1Value = analogRead(soilMoisture1);
  delay(500);
  soilMoisture2Value = analogRead(soilMoisture2);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  heatIndex = dht.computeHeatIndex(temperature, humidity, false);
  timeNow = String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC);
  dateNow = String(now.day(), DEC) + "/" + String(now.month(), DEC) + "/" + String(now.year(), DEC);
});

Task lcdUpdate(100,[](){
  // = = = = = = = = = = = = = LCD Print = = = = = = = = = = = = =
  lcdPrint("T: " + String(temperature) + "C", "H: " + String(humidity) + "%");
  delay(2000);
  lcdPrint("soil1: " + String(soilMoisture1Value), "soil2: " + String(soilMoisture2Value));
  delay(2000);
  lcdPrint("Time: " + timeNow, "Date: " + dateNow);
  delay(2000);
});

Task mqttUpdate(5000,[](){
  // Kirim data JSON ke MQTT
  DynamicJsonDocument doc(1024);
  doc["soil1"] = soilMoisture1Value;
  doc["soil2"] = soilMoisture2Value;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["heatIndex"] = heatIndex;
  doc["time"] = timeNow;
  doc["date"] = dateNow;
  String output;
  serializeJson(doc, output);
  sendData(dataTopic, output.c_str());
});

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  dataUpdate.update();
  lcdUpdate.update();
  mqttUpdate.update();
}
