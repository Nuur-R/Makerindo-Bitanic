#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include "DHT.h"
#include <esp_task_wdt.h>

#define LED 2
#define RELAY1 15
#define RELAY2 4
#define BUZZER 13
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DHTPIN1 27
#define DHTPIN2 26
#define DHTTYPE DHT11
#define WDT_TIMEOUT 3

RTC_DS1307 rtc;
WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

int Detik, PortValue, countWifi, count2, State1, GlobalCount, h1, h2, LockTime, CountTime, CountMotor, TimeMotor, CountMinggu, TotalMinggu, OnDay, SaveOn, CountDayOn, countEND;
float t1, t2;
unsigned long GlobalClock;
String DATA[10], INPUTDATA, TimeNOW, DateNOW;
bool Lock1, Lock2, Lock3, Lock4, LockDate, MotorLock, LockMQTT;
String ssid, password;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void setup() {
  Serial.begin(9600);
  EEPROM.begin(512);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  rtc.begin();
  dht1.begin();
  dht2.begin();

  pinMode(LED, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(LED, LOW);
  digitalWrite(BUZZER, LOW);
  LockTime = 0;

  ssid = EEPROM.readString(0);
  password = EEPROM.readString(30);

  wifiMulti.addAP("Setting", "admin1234");
  wifiMulti.addAP(ssid.c_str(), password.c_str());

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  DateTime now = rtc.now();
  while (wifiMulti.run() != WL_CONNECTED) {
    DateTime now = rtc.now();
    digitalWrite(LED, LOW);
    Detik++;
    State1 = 0;
    display.println("  WIFI NOT CONNECTED");
    display.display();
    Serial.println("WIFI NOT CONNECTED");
    delay(1000);
    if (Detik == 5) {
      Lock1 == false;
      break;
    }
  }
  if (wifiMulti.run() == WL_CONNECTED) {
    State1 = 1;
    Lock1 = true;
    display.println("   WIFI CONNECTED");
    display.display();
    digitalWrite(LED, HIGH);
    Serial.println("WIFI CONNECTED");
  }
  client.setServer("broker.hivemq.com", 1883);
  client.setCallback(callback);
  while (!client.connected() && Lock1 == true) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("NUTRIFERADS1")) {
      State1 = 3;
      display.println("    CONNECTION OK");
      display.display();
      Serial.println("MQTT CONNECTED");
    } else {
      State1 = 4;
      display.println("    NO CONNECTION");
      display.display();
      digitalWrite(LED, LOW);
      delay(500);
      digitalWrite(LED, HIGH);
      delay(100);
      Serial.print("failed with state ");
      Serial.print(client.state());
    }
  }
  Detik = 0;

  client.subscribe("ferads/NF01");
}

void loop() {
  OnDay = 0;
  SaveOn = EEPROM.readString(280).toInt();
  if (EEPROM.readString(100) != "00:00:00") {
    OnDay++;
  } if (EEPROM.readString(120) != "00:00:00") {
    OnDay++;
  } if (EEPROM.readString(140) != "00:00:00") {
    OnDay++;
  } if (EEPROM.readString(160) != "00:00:00") {
    OnDay++;
  } if (EEPROM.readString(180) != "00:00:00") {
    OnDay++;
  }
  DateTime now = rtc.now();
  if (now.minute() % 60 == 0) {
    display.dim(true);
  }
  if (now.minute() % 60 == 1) {
    display.dim(false);
  }
  if (now.second() != GlobalClock) {
    CountDayOn = 0;
    //    GlobalCount++;
    if (wifiMulti.run() != WL_CONNECTED) {
      Lock1 = false;
      State1 = 0;
    }
    if (!client.connected()) {
      Lock1 = false;
      State1 = 4;
    }
    if (Lock1 == false) {
      if (digitalRead(RELAY1) != HIGH || digitalRead(RELAY2) != HIGH) {
        digitalWrite(LED, LOW);
        Detik++;
        if (Detik == 5) {
          State1 = 5;
          ESP.restart();
        }
      }
    }
    if (EEPROM.readString(210) == "Sunday") {
      CountDayOn++;
    } if (EEPROM.readString(220) == "Monday") {
      CountDayOn++;
    } if (EEPROM.readString(230) == "Tuesday") {
      CountDayOn++;
    } if (EEPROM.readString(240) == "Wednesday") {
      CountDayOn++;
    } if (EEPROM.readString(250) == "Thursday") {
      CountDayOn++;
    } if (EEPROM.readString(260) == "Friday") {
      CountDayOn++;
    } if (EEPROM.readString(270) == "Saturday") {
      CountDayOn++;
    }
    if (EEPROM.readString(200) != "END") {
      if (EEPROM.readString(310).toInt() == CountDayOn) {
        if(EEPROM.readString(200) != "0"){
        int YY = EEPROM.readString(205).toInt();
        YY++;
        EEPROM.writeString(205, String() + YY);
        }
        EEPROM.writeString(320, "0000000");
        EEPROM.writeString(310, "0");
      }
      if (LockTime > 0 && Lock4 == false) {
        digitalWrite(RELAY1, HIGH);
        digitalWrite(RELAY2, HIGH);
        int KK = EEPROM.readString(280).toInt();
        KK++;
        EEPROM.writeString(280, String() + KK);
        if (EEPROM.readString(280).toInt() == OnDay) {
          EEPROM.writeString(280, "0   ");
          KK = EEPROM.readString(310).toInt();
          KK++;
          EEPROM.writeString(310, String() + KK);
          int LL = 320 + now.dayOfTheWeek();
          EEPROM.writeChar(LL, '1');
        }
        Send();
        Lock4 = true;
      }
    }
    if (now.hour() == 12) {
      int LL = 320 + now.dayOfTheWeek();
      EEPROM.writeChar(LL, '1');
    }
    if (EEPROM.readString(205) == EEPROM.readString(200) && EEPROM.readString(200) != 0) {
      EEPROM.writeString(200, "END");
      EEPROM.writeString(205, "0");
    }
    if (Lock4 == true) {
      TimeMotor++;
    }
    if (TimeMotor >= CountMotor * 60 && Lock4 == true) {
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      Send();
      Lock4 = false;
      LockTime = 0;
      TimeMotor = 0;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    String Load = String() + daysOfTheWeek[now.dayOfTheWeek()] + ", ";
    if (now.day() < 10) {
      Load += String() + "0" + now.day();
    } else {
      Load += now.day(), DEC;
    }
    Load += "/";
    if (now.month() < 10) {
      Load += String() + "0" + now.month();
    } else {
      Load += now.month(), DEC;
    }
    Load += String() + "/" + now.year();
    if (Load.length() >= 21) {
      display.setCursor(0, 0);
    } else if (Load.length() == 20) {
      display.setCursor(3, 0);
    } else if (Load.length() == 19) {
      display.setCursor(5, 0);
    } else if (Load.length() == 18) {
      display.setCursor(8, 0);
    }
    Load += "\n      ";
    if (now.hour() < 10) {
      Load += String() + "0" + now.hour();
    } else {
      Load += now.hour(), DEC;
    }
    Load += ":";
    if (now.minute() < 10) {
      Load += String() + "0" + now.minute();
    } else {
      Load += now.minute(), DEC;
    }
    Load += ":";
    if (now.second() < 10) {
      Load += String() + "0" + now.second();
    } else {
      Load += now.second(), DEC;
    }
    //    Load += "\n\nDHT1   :\nDHT2   :\nMOTOR1 :\nMOTOR2 :\n   WIFI CONNECTED\nA";
    if (now.second() % 10 <= 4) {
      h1 = dht1.readHumidity();
      t1 = dht1.readTemperature();
      h2 = dht2.readHumidity();
      t2 = dht2.readTemperature();

      if (isnan(h1) || isnan(t1)) {
        Serial.println(F("Failed to read from DHT sensor 1!"));
        Load += "\n\nDHT1   :NOT CONNECT\n";
      } else {
        Load += String() + "\n\nDHT1  :" + t1 + "'C " + h1 + "%\n";
      }
      if (isnan(h2) || isnan(t2)) {
        Serial.println(F("Failed to read from DHT sensor 2!"));
        Load += "DHT2   :NOT CONNECT\n";
      } else {
        Load += String() + "DHT2  :" + t2 + "'C " + h2 + "%\n";
      }
      if (digitalRead(RELAY1) == 1) {
        Load += String() + "RELAY1:NYALA\n";
      } else {
        Load += String() + "RELAY1:MATI\n";
      }
      if (digitalRead(RELAY2) == 1) {
        Load += String() + "RELAY2:NYALA\n";
      } else {
        Load += String() + "RELAY2:MATI\n";
      }
      if (State1 == 0) {
        Load += "  WIFI NOT CONNECTED";
      } else if (State1 == 2) {
        Load += "   WIFI CONNECTED";
      } else if (State1 == 3) {
        Load += "    CONNECTION OK";
      } else if (State1 == 4) {
        Load += "    NO CONNECTION";
      } else if (State1 == 5) {
        Load += "    RESTARTING...";
        display.println(Load);
        display.display();
        delay(1000);
        ESP.restart();
      }
    } else {
      Load += "\n\nDEV :NF01";
      Load += String() + "\nSSID:" + ssid;
      Load += "\nSERV:broker.hivemq";
      Load += "\nPORT:1883\n";
      if (State1 == 0) {
        Load += "  WIFI NOT CONNECTED";
      } else if (State1 == 2) {
        Load += "   WIFI CONNECTED";
      } else if (State1 == 3) {
        Load += "    CONNECTION OK";
      } else if (State1 == 4) {
        Load += "    NO CONNECTION";
      } else if (State1 == 5) {
        Load += "    RESTARTING...";
        display.println(Load);
        display.display();
        delay(1000);
        ESP.restart();
      }
    }
    //    Serial.println(Load);
    display.println(Load);
    display.display();
    client.loop();
    GlobalClock = now.second();
  }
  if (digitalRead(RELAY1) == HIGH || digitalRead(RELAY2) == HIGH) {
    MotorLock = true;
  } else {
    MotorLock = false;
  }
  if (now.hour() < 10) {
    TimeNOW = String() + "0" + now.hour();
  } else {
    TimeNOW = now.hour(), DEC;
  }
  TimeNOW += ":";
  if (now.minute() < 10) {
    TimeNOW += String() + "0" + now.minute();
  } else {
    TimeNOW += now.minute(), DEC;
  }
  TimeNOW += ":";
  if (now.second() < 10) {
    TimeNOW += String() + "0" + now.second();
  } else {
    TimeNOW += now.second(), DEC;
  }
  if (EEPROM.readString(200) != "END") {
    TotalMinggu = EEPROM.readString(200).toInt();
    DateNOW = daysOfTheWeek[now.dayOfTheWeek()];
    if (DateNOW == EEPROM.readString(210) || DateNOW == EEPROM.readString(220) || DateNOW == EEPROM.readString(230) || DateNOW == EEPROM.readString(240) || DateNOW == EEPROM.readString(250) || DateNOW == EEPROM.readString(260) || DateNOW == EEPROM.readString(270)) {
      LockDate = true;
    } else {
      LockDate = false;
    }
    if (LockDate == true && CountMinggu <= TotalMinggu && EEPROM.readString(200).toInt() > 0) {
      if (TimeNOW == EEPROM.readString(100) && EEPROM.readString(100) != "00:00:00") {
        LockTime = 1;
      } else if (TimeNOW == EEPROM.readString(120) && EEPROM.readString(120) != "00:00:00") {
        LockTime = 2;
      } else if (TimeNOW == EEPROM.readString(140) && EEPROM.readString(140) != "00:00:00") {
        LockTime = 3;
      } else if (TimeNOW == EEPROM.readString(160) && EEPROM.readString(160) != "00:00:00") {
        LockTime = 4;
      } else if (TimeNOW == EEPROM.readString(180) && EEPROM.readString(180) != "00:00:00") {
        LockTime = 5;
      }
      if (LockTime == 1) {
        CountMotor = EEPROM.readString(110).toInt();
      } else if (LockTime == 2) {
        CountMotor = EEPROM.readString(130).toInt();
      } else if (LockTime == 3) {
        CountMotor = EEPROM.readString(150).toInt();
      } else if (LockTime == 4) {
        CountMotor = EEPROM.readString(170).toInt();
      } else if (LockTime == 5) {
        CountMotor = EEPROM.readString(190).toInt();
      }
    }
    countEND = 0;
  }
  if (EEPROM.readString(200) == "END") {
    EEPROM.writeString(200, "END");
    EEPROM.writeString(205, "0");
    EEPROM.writeString(280, "0");
    EEPROM.writeString(310, "0");
    EEPROM.writeString(320, "0000000");
  }
  //  Serial.println(String() + "DATA SET ON 1: " + EEPROM.readString(100) + " MINUTE : " + EEPROM.readString(110));
  //  Serial.println(String() + "DATA SET ON 2: " + EEPROM.readString(120) + " MINUTE : " + EEPROM.readString(130));
  //  Serial.println(String() + "DATA SET ON 3: " + EEPROM.readString(140) + " MINUTE : " + EEPROM.readString(150));
  //  Serial.println(String() + "DATA SET ON 4: " + EEPROM.readString(160) + " MINUTE : " + EEPROM.readString(170));
  //  Serial.println(String() + "DATA SET ON 5: " + EEPROM.readString(180) + " MINUTE : " + EEPROM.readString(190));
  //  Serial.println(String() + "DATA MINGGU : " + EEPROM.readString(200));
  //  Serial.println(String() + "DATA CURRENT MINGGU: " + EEPROM.readString(205));
  //  Serial.println(String() + "DATA MINGGU: " + EEPROM.readString(210));
  //  Serial.println(String() + "DATA SENIN: " + EEPROM.readString(220));
  //  Serial.println(String() + "DATA SELASA: " + EEPROM.readString(230));
  //  Serial.println(String() + "DATA RABU: " + EEPROM.readString(240));
  //  Serial.println(String() + "DATA KAMIS: " + EEPROM.readString(250));
  //  Serial.println(String() + "DATA JUMAT: " + EEPROM.readString(260));
  //  Serial.println(String() + "DATA SABTU: " + EEPROM.readString(270));
  //  Serial.println(String() + "ONDAY: " + OnDay);
  //  Serial.println(String() + "SAVEON: " + EEPROM.readString(280));
  //  Serial.println(String() + "COUNT DAY ON: " + EEPROM.readString(310));
  //  Serial.println(String() + "STRING DAY ON: " + EEPROM.readString(320));
  //  Serial.println("\n\n");
  EEPROM.commit();
  CountDayOn = 0;

}

void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    if (c == ',') {
      INPUTDATA.trim();
      DATA[count2] = INPUTDATA;
      count2++;
      INPUTDATA = "";
    } else if (c == '*') {
      display.dim(false);
      digitalWrite(BUZZER, HIGH);
      if (DATA[0] == "SETSSID") {
        EEPROM.writeString(0, DATA[1]);
        Serial.println("NEW SSID SET");
      } else if (DATA[0] == "SETPASSWORD") {
        EEPROM.writeString(30, DATA[1]);
        Serial.println("NEW PASSWORD SET");
      } else if (DATA[0] == "MOTOR1") {
        if (DATA[1] == "1") {
          digitalWrite(RELAY1, HIGH);
        } else {
          digitalWrite(RELAY1, LOW);
        }
        Serial.print("RELAY1 = ");
        Serial.println(digitalRead(RELAY1));
      } else if (DATA[0] == "MOTOR2") {
        if (DATA[1] == "1") {
          digitalWrite(RELAY2, HIGH);
        } else {
          digitalWrite(RELAY2, LOW);
        }
        Serial.print("RELAY2 = ");
        Serial.println(digitalRead(RELAY2));
      } else if (DATA[0] == "GETDATA") {
        //        GlobalCount = 0;
        Serial.println(EEPROM.readString(0));
        Serial.println(EEPROM.readString(30));
        Send();
      } else if (DATA[0] == "SETRTC") {
        rtc.adjust(DateTime(DATA[1].toInt(), DATA[2].toInt(), DATA[3].toInt(), DATA[4].toInt(), DATA[5].toInt(), DATA[6].toInt()));
        Serial.println("RTC UPDATED");
      } else if (DATA[0] == "RESET") {
        Serial.println("RESTARTING ESP");
        ESP.restart();
      } else if (DATA[0] == "SETONTIME1") {
        EEPROM.writeString(100, DATA[1]);
        EEPROM.writeString(110, DATA[2]);
        Serial.println("SETONTIME1 OK");
      } else if (DATA[0] == "SETONTIME2") {
        EEPROM.writeString(120, DATA[1]);
        EEPROM.writeString(130, DATA[2]);
        Serial.println("SETONTIME2 OK");
      } else if (DATA[0] == "SETONTIME3") {
        EEPROM.writeString(140, DATA[1]);
        EEPROM.writeString(150, DATA[2]);
        Serial.println("SETONTIME3 OK");
      } else if (DATA[0] == "SETONTIME4") {
        EEPROM.writeString(160, DATA[1]);
        EEPROM.writeString(170, DATA[2]);
        Serial.println("SETONTIME4 OK");
      } else if (DATA[0] == "SETONTIME5") {
        EEPROM.writeString(180, DATA[1]);
        EEPROM.writeString(190, DATA[2]);
        Serial.println("SETONTIME5 OK");
      } else if (DATA[0] == "SETMINGGU") {
        countEND = 0;
        EEPROM.writeString(200, DATA[1]);
        EEPROM.writeString(205, "0");
        Serial.println("SETMINGGU OK");
      } else if (DATA[0] == "SETHARI") {
        String TT;
        if (DATA[1] == "1") {
          EEPROM.writeString(210, "Sunday");
          TT = "1";
        } else {
          EEPROM.writeString(210, "NONE");
          TT = "0";
        } if (DATA[2] == "1") {
          EEPROM.writeString(220, "Monday");
          TT += "1";
        } else {
          EEPROM.writeString(220, "NONE");
          TT += "0";
        } if (DATA[3] == "1") {
          EEPROM.writeString(230, "Tuesday");
          TT += "1";
        } else {
          EEPROM.writeString(230, "NONE");
          TT += "0";
        }  if (DATA[4] == "1") {
          EEPROM.writeString(240, "Wednesday");
          TT += "1";
        } else {
          EEPROM.writeString(240, "NONE");
          TT += "0";
        }  if (DATA[5] == "1") {
          EEPROM.writeString(250, "Thursday");
          TT += "1";
        } else {
          EEPROM.writeString(250, "NONE");
          TT += "0";
        }  if (DATA[6] == "1") {
          EEPROM.writeString(260, "Friday");
          TT += "1";
        } else {
          EEPROM.writeString(260, "NONE");
          TT += "0";
        }  if (DATA[7] == "1") {
          EEPROM.writeString(270, "Saturday");
          TT += "1";
        } else {
          EEPROM.writeString(270, "NONE");
          TT += "0";
        }
        EEPROM.writeString(300, TT);
        Serial.println("SETHARI OK");
      } else if (DATA[0] == "RESETALL") {
        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, LOW);
        EEPROM.writeString(100, "00:00:00");
        EEPROM.writeString(110, "0");
        EEPROM.writeString(120, "00:00:00");
        EEPROM.writeString(130, "0");
        EEPROM.writeString(140, "00:00:00");
        EEPROM.writeString(150, "0");
        EEPROM.writeString(160, "00:00:00");
        EEPROM.writeString(170, "0");
        EEPROM.writeString(180, "00:00:00");
        EEPROM.writeString(190, "0");
        EEPROM.writeString(200, "0");
        EEPROM.writeString(205, "0");
        EEPROM.writeString(210, "NONE");
        EEPROM.writeString(220, "NONE");
        EEPROM.writeString(230, "NONE");
        EEPROM.writeString(240, "NONE");
        EEPROM.writeString(250, "NONE");
        EEPROM.writeString(260, "NONE");
        EEPROM.writeString(270, "NONE");
        EEPROM.writeString(280, "0");
        EEPROM.writeString(300, "0000000");
        EEPROM.writeString(310, "0");
        EEPROM.writeString(320, "0000000");
        Serial.println("RESETALL OK");
      } else if (DATA[0] == "DIM") {
        if (DATA[1] == "1") {
          display.dim(true);
        } else {
          display.dim(false);
        }
      } else {
        Serial.println("COMMAND UNDEFINED");
      }
      count2 = 0;
      INPUTDATA = "";
    } else {
      INPUTDATA += c;
    }
  }
  EEPROM.commit();
  delay(200);
  digitalWrite(BUZZER, LOW);
}

void Send() {
  DateTime now = rtc.now();
  DynamicJsonDocument doc(1023);
  doc["ID"] = "NF01";
  doc["SSID"] = ssid;
  doc["PASSWORD"] = password;
  doc["STATUS_MOTOR1"] = digitalRead(RELAY1);
  doc["STATUS_MOTOR2"] = digitalRead(RELAY2);
  doc["DATE"] = String() + now.day() + "-" + now.month() + "-" + now.year();
  doc["TIME"] = String() + now.hour() + ":" + now.minute() + ":" + now.second();
  doc["DHT1Temp"] = t1;
  doc["DHT1Hum"] = h1;
  doc["DHT2Temp"] = t2;
  doc["DHT2Hum"] = h2;
  doc["ONTIME1"] = String() + EEPROM.readString(100) + "  " + EEPROM.readString(110) + "Menit";
  doc["ONTIME2"] = String() + EEPROM.readString(120) + "  " + EEPROM.readString(130) + "Menit";
  doc["ONTIME3"] = String() + EEPROM.readString(140) + "  " + EEPROM.readString(150) + "Menit";
  doc["ONTIME4"] = String() + EEPROM.readString(160) + "  " + EEPROM.readString(170) + "Menit";
  doc["ONTIME5"] = String() + EEPROM.readString(180) + "  " + EEPROM.readString(190) + "Menit";
  doc["TOTALPEKAN"] = EEPROM.readString(200);
  doc["CURRENTPEKAN"] = EEPROM.readString(205);
  doc["DET_HARI"] = EEPROM.readString(300);
  doc["HISTORY"] = EEPROM.readString(320);
  String data = "";
  serializeJson(doc, data);
  serializeJson(doc, Serial);
  Serial.println("Sending message to MQTT topic..");
  client.beginPublish("ferads", data.length(), false);
  client.print(data);
  client.endPublish();
}
