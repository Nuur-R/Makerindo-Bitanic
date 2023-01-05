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



// Panel Setup
const char* panelName     = "Bitanic 2.0";
const char* panelVersion  = "1.0";
const char* panelID       = "BT01"; // Berubah Tiap Panel

// network setup
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

// MQTT Topic
const char* topic = "bitanicv2/data";

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
  Serial.println("Soil Moisture 2 : " + String(soilMoisture2Value));
  Serial.println("Soil Moisture 1 : " + String(soilMoisture1Value));
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

// MQTT Callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// variable json dengan struktur {"ID": "BT01","Soil1": "2980.12","Soil2": "2980.12"} untuk di kirimkan ke server MQTT melalui fungsi publish
void publishData() {
  StaticJsonDocument<200> doc;
  doc["ID"] = panelID;
  doc["Soil1"] = soilMoisture1Value;
  doc["Soil2"] = soilMoisture2Value;
  doc["Suhu"] = temperature;
  doc["Kelembaban"] = humidity;
  doc["IndeksPanas"] = heatIndex;
  doc["Waktu"] = timeNow;
  doc["Tanggal"] = dateNow;
  char json[200];
  serializeJson(doc, json);
  client.publish(topic, json);
}

void setup()
{
  Serial.begin(115200);
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  lcdPrint("Bitanic V2.0", "Start...");
  buzz(100, 3);
  lcdPrint("Setting WiFi", "192.168.4.1");
  buzz(100, 3);
  // WiFi Manager Setting
  WiFiManager wm;
    bool res;
    res = wm.autoConnect("Bitanic V2");
    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        Serial.println("connected...yeey :)");
    }
  
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
}

void rtcUpdate()
{
  DateTime now = rtc.now(); // get the current time
  timeNow = String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC);
  dateNow = String(now.day(), DEC) + "/" + String(now.month(), DEC) + "/" + String(now.year(), DEC);
}

int sensorStartTime = 0;
void sensorUpdate()
{
  // = = = = = = = = = = = = = Sensor Update = = = = = = = = = = = = =
  if (sensorStartTime == 0)
  {
    sensorStartTime = millis();
  }
  if (millis() - sensorStartTime >= 5000)
  {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      lcdPrint("Failed to read", "from DHT sensor!");
      return;
    }
    soilMoisture1Value = analogRead(soilMoisture1);
    delay(1000);
    soilMoisture2Value = analogRead(soilMoisture2);
    sensorStartTime = 0;
  }
}

void loop()
{
  rtcUpdate();
  sensorUpdate();
  serialDataPrint();
  if (!client.connected()) {
    reconnect();
  }
  // = = = = = = = = = = = = = LCD Print = = = = = = = = = = = = =
  // lcdPrint("T: " + String(temperature) + "C", "H: " + String(humidity) + "%");
  // delay(2000);
  // lcdPrint("soil1: " + String(soilMoisture1Value), "soil2: " + String(soilMoisture2Value));
  // delay(200);
  
  lcdPrint("T: " + String(temperature) + "C", "H: " + String(humidity) + "%");
  delay(3000);
  publishData();
  lcdPrint("Data send...", timeNow + "||" + dateNow);
  delay(2000);
}