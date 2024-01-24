#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiManager.h> 
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <HTTPClient.h>

#define Relay_1 32
#define Relay_2 33
#define Relay_3 25
#define Relay_4 26

WiFiClient  client;
WiFiUDP ntpUDP;
const long offsetTime = 25200; // หน่วยเป็นวินาที จะได้ 7*60*60 = 25200
NTPClient timeClient(ntpUDP, "pool.ntp.org", offsetTime);
LiquidCrystal_I2C lcd(0x27, 16, 2);

String myStatus = "";
int hour;
int minute;
int second;
int count = 0;
bool set = false;

char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <body>
    <center>
      <h1>EV Connect</h1>
        <h3> Charge 5 Seconds </h3>
        <button onclick="window.location = 'http://'+location.hostname+'/relay_1/on5s'">On</button>
        <h3> Charge 10 Seconds </h3>
        <button onclick="window.location = 'http://'+location.hostname+'/relay_1/on10s'">On</button>
        <h3> Charge 15 Seconds </h3>
        <button onclick="window.location = 'http://'+location.hostname+'/relay_1/on15s'">On</button>
        <h3> Stop Charge! </h3>
        <button onclick="window.location = 'http://'+location.hostname+'/relay_1/off'">Off</button>
    </center>
  </body>
</html>
)=====";

void setupWiFi();
void charging();
void nocharging();
void problemcharging();

#include <ESPAsyncWebServer.h>
AsyncWebServer server(80); // server port 80

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Page Not found");
}


void setup(void) {
  Serial.begin(115200);
  pinMode(Relay_1, OUTPUT); //220V Charge
  pinMode(Relay_2, OUTPUT); //Green
  pinMode(Relay_3, OUTPUT); //Yellow
  pinMode(Relay_4, OUTPUT); //Red
  delay(2000);
  Serial.println("Connecting Wifi. Please Wait!!");
  setupWiFi();
  delay(2000);
  Serial.println("Hi everyone!!");
  nocharging();
  if (MDNS.begin("ESP2")) { //esp2.local/
    Serial.println("MDNS responder started");
  }
  server.on("/", [](AsyncWebServerRequest * request) { request->send_P(200, "text/html", webpage); });

  nocharging();
  //==============================เริ่มคำสั่ง request ต่างๆ==============================
  server.begin();
  server.on("/relay_1/on5s", HTTP_GET, [](AsyncWebServerRequest * request) { 
    //Start Process
    Serial.print("===========================\n");
    if (count < 3) {
      count++;
      Serial.printf("Count Of Charge : %d \n", count);
      charging();
    } else {
      Serial.println("Problem Charge! Count > 3");
      delay(1000);
      count = 0;
      problemcharging();
    }
    //End Process
    request->send_P(200, "text/html", webpage);
  });

  server.on("/relay_1/on10s", HTTP_GET, [](AsyncWebServerRequest * request) { 
    //Start Process
    Serial.print("===========================\n");
    if (count < 3) {
      count++;
      Serial.printf("Count Of Charge : %d \n", count);
      charging();
    } else {
      Serial.println("Problem Charge! Count > 3");
      delay(1000);
      count = 0;
      problemcharging();
    }
    //End Process
    request->send_P(200, "text/html", webpage);
  });

  server.on("/relay_1/on15s", HTTP_GET, [](AsyncWebServerRequest * request) { 
    //Start Process
    Serial.print("===========================\n");
    if (count < 3) {
      count++;
      Serial.printf("Count Of Charge : %d \n", count);
      charging();
    } else {
      Serial.println("Problem Charge! Count > 3");
      delay(1000);
      count = 0;
      problemcharging();
    }
    //End Process
    request->send_P(200, "text/html", webpage);
  });

  server.on("/relay_1/off", HTTP_GET, [](AsyncWebServerRequest * request) { 
    //Start Process
    Serial.print("===========================\n");
    Serial.printf("Count Of Charge : %d \n", count);
    nocharging();
    //End Process
    request->send_P(200, "text/html", webpage);
  });

  server.on("/relay_1/problem", HTTP_GET, [](AsyncWebServerRequest * request) { 
    //Start Process
    Serial.print("===========================\n");
    problemcharging();
    //End Process
    request->send_P(200, "text/html", webpage);
  });
  //==============================จบคำสั่ง request ต่างๆ==============================

  server.onNotFound(notFound);
  server.begin();  // it will start webserver
}


void loop(void) {

}

void setupWiFi() {
    WiFiManager wm;
    bool res;
    res = wm.autoConnect("EV Connect","10101010"); // password protected ap
    if(!res) {
      Serial.println("Failed to connect");     
    } else {
      Serial.print("connected...:) ");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
}

void charging() {
  Serial.println("Charging!");
  digitalWrite(Relay_1, HIGH);
  digitalWrite(Relay_2, HIGH);
  digitalWrite(Relay_3, LOW);
  digitalWrite(Relay_4, LOW);
}

void nocharging() {
  digitalWrite(Relay_1, LOW);
  digitalWrite(Relay_2, LOW);
  digitalWrite(Relay_3, HIGH);
  digitalWrite(Relay_4, LOW);
  Serial.println("No Charging!");
}

void problemcharging() {
  int countdown = 5; //wait 5 seconds
  digitalWrite(Relay_1, LOW);
  digitalWrite(Relay_2, LOW);
  digitalWrite(Relay_3, LOW);
  digitalWrite(Relay_4, HIGH);
  for (int i = countdown; i >= 0; i--) {
    Serial.printf("Wait %d Seconds \n", i);
    delay(1000);
  }
  delay(2000);
  nocharging();
}