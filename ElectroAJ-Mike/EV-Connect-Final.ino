#include <HTTPClient.h>
#include <WiFiManager.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>
#include <PZEM004Tv30.h>
#include <Wire.h>
#include "ThingSpeak.h"

HardwareSerial pzemSerial(2); // Create a HardwareSerial object for Serial2
PZEM004Tv30 pzem(pzemSerial, 16, 17);
LiquidCrystal_I2C lcd(0x27, 20, 4);
WiFiClient  client;
WiFiUDP ntpUDP;

const long offsetTime = 25200;
NTPClient timeClient(ntpUDP, "pool.ntp.org", offsetTime);

unsigned long myChannelNumber = ChannelNum; //Input Your ChannelID Number
const char * myWriteAPIKey = "API_KEY"; //Input Your API_Key ThinkSpeak

String GOOGLE_SCRIPT_ID = "ID_SCRIPT";
#define Control_Electric 32
#define Status_Green 33
#define Status_Yellow 25
#define Status_Red 26

String finalresult = "";
String myStatus = "";
String result = "";
int hour;
int minute;
int second;
int count = 0;
float vol;
float amp;

void setupWiFi();
void charging();
void nocharging();
void problemcharging();
void sendThing();

void setup() {
  Serial.begin(115200);
  pzemSerial.begin(9600);
  problemcharging();
  // initialize the LCD
  lcd.begin();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  pinMode(Control_Electric, OUTPUT);
  pinMode(Status_Green, OUTPUT);
  pinMode(Status_Yellow, OUTPUT);
  pinMode(Status_Red, OUTPUT);
  lcd.setCursor(0, 1); // แถวที่ 2
  lcd.print("Please Connect Wifi.");
  lcd.setCursor(4, 2); // แถวที่ 3
  lcd.print("Wait......");
  setupWiFi();
  lcd.clear();
  lcd.setCursor(4, 1); // แถวที่ 2
  lcd.print("Starting Up");
  lcd.setCursor(4, 2); // แถวที่ 3
  lcd.print("The System..");

  ThingSpeak.begin(client);  // Initialize ThingSpeak
  timeClient.begin();
  timeClient.update();
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  second = timeClient.getSeconds();
  Serial.print(F("Time Begin : "));
  Serial.println(timeClient.getFormattedTime());
  delay(5000);
  // lcd.clear();
}

void loop() {
  timeClient.update();
  Serial.println("\n=============================================\n");
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  second = timeClient.getSeconds();
  Serial.println(F("Time Begin : "));
  Serial.println(timeClient.getFormattedTime());
  // sendThing(11, 22, 33, 44); //Test Send Data To Thinkspeak
  //เรียกใช้ฟังชั่น
  getDataFromGoogleSheets();
}


void getDataFromGoogleSheets() {
  // คำอธิบาย: ส่วน getDataFromGoogleSheets ไม่มีการเปลี่ยนแปลง
  if ((WiFi.status() == WL_CONNECTED)) {
    WiFiClientSecure client;
    HTTPClient http;
       //-----------------------------------------------------------------------------------
      client.setInsecure();
      HTTPClient https;
      String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?read";
      Serial.println("Reading Data From Google Sheet.....");
      https.begin(client, url.c_str());
      //-----------------------------------------------------------------------------------
      //Removes the error "302 Moved Temporarily Error"
      https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      //-----------------------------------------------------------------------------------
      //Get the returning HTTP status code
      int httpCode = https.GET();
      Serial.print("HTTP Status Code: ");
      Serial.println(httpCode);
      //-----------------------------------------------------------------------------------
      if(httpCode <= 0){Serial.println("Error on HTTP request"); https.end(); return;}
      //-----------------------------------------------------------------------------------
      //reading data comming from Google Sheet
      String payload = https.getString();
      Serial.println("Payload: " + payload);

      int index1 = payload.indexOf(":");
      if (index1 >= 0) {
      String result = payload.substring(index1 + 1);
      result.trim();

      Serial.print("Data from Sheet : ");
      Serial.println(result);

      // ให้ finalresult มีค่าเป็น result ที่ได้
      finalresult = result;
}
      //-----------------------------------------------------------------------------------
      if(httpCode == 200)
      Serial.print("Data from Sheet : ");
      Serial.println(finalresult);
      worked(finalresult);
      //-------------------------------------------------------------------------------------
      https.end();

  }
}

void worked(String minutes) {
  String params = "0";
  int conminutes = minutes.toInt();
  if (conminutes == 0) {
    Serial.print("This Defualt : ");
    Serial.println(minutes);
    Serial.printf("===============================================\n");
    Serial.printf("This Defualt minutes in if : %d \n", conminutes);
    Serial.printf("This Count : %d \n", count);
    nocharging();
    Serial.printf("===============================================\n");
    Serial.printf("Exit Program!!! \n");
  } else {
      if (count < 3) {
        lcd.clear();
        Serial.printf("===============================================\n");
        Serial.printf("This Defualt minutes in else : %d \n", conminutes); count++;
        Serial.printf("This Count : %d \n", count); 
        charging(conminutes);
        Serial.printf("===============================================\n");
        //----------------------------------------
        for (int i = conminutes; i >= 0; i--) {
          Serial.print(F("Countdown = "));
          Serial.print(i);
          Serial.print(F(" Seconds\n"));
          lcd.setCursor(5, 2);
          lcd.print(i);
          lcd.print(" Seconds");
          delay(1000);
        }
        sendDataNull(params);
        nocharging();
      } else {
        Serial.printf("===============================================\n");
        Serial.printf("Count : %d > 3 \n", count+1); count = 0;
        problemcharging();
        int j = 8;
          while (j >= 0) {
            lcd.setCursor(5, 2);
            lcd.print(j);
            lcd.print(" Seconds");
            Serial.printf("Wait %d Seconds \n", j);
            j--;
            delay(1000);
          }
          nocharging();
        Serial.printf("===============================================\n");
      }
    }
}

void charging(int second) {
  lcd.clear();
  digitalWrite(Control_Electric, HIGH);
  digitalWrite(Status_Green, HIGH);
  digitalWrite(Status_Yellow, LOW);
  digitalWrite(Status_Red, LOW);
  sendThing(second);
  delay(1000);
  Serial.println("Charging!");
  lcd.setCursor(5, 0);
  lcd.print("EV Connect");
  lcd.setCursor(4, 1);
  lcd.print("Please Wait..");
  // lcd.setCursor(5, 2);
  // lcd.print("     Seconds");
  lcd.setCursor(3, 3);
  lcd.print("-- Charging --");
}

void nocharging() {
  lcd.clear();
  Serial.println("No Charging!");
  lcd.setCursor(5, 0);
  lcd.print("EV Connect");
  lcd.setCursor(2, 2);
  lcd.print("System Is Ready.");
  // lcd.setCursor(4, 2);
  // lcd.print("Scan QRcode..");
  lcd.setCursor(1, 3);
  lcd.print("-- No Charging --");
  digitalWrite(Control_Electric, LOW);
  digitalWrite(Status_Green, LOW);
  digitalWrite(Status_Yellow, HIGH);
  digitalWrite(Status_Red, LOW);
}

void problemcharging() {
  lcd.clear();
  String params = "0";
  Serial.printf("Problem Charge!! \n");
  digitalWrite(Control_Electric, LOW);
  digitalWrite(Status_Green, LOW);
  digitalWrite(Status_Yellow, LOW);
  digitalWrite(Status_Red, HIGH);
  lcd.setCursor(5, 0);
  lcd.print("EV Connect");
  lcd.setCursor(5, 1);
  lcd.print("Warning!!!");
  // lcd.setCursor(5, 2);
  // lcd.print("     Seconds");
  lcd.setCursor(3, 3);
  lcd.print("-- Problem! --");
  sendDataNull(params);
}

void sendDataNull(String params) {
  WiFiClientSecure client;
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec";
  Serial.print("Making a request to: ");
  Serial.println(url);

  // Add your request data here
  String postData = "params=" + params;
  client.setInsecure();
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpCode);

  http.end();
}

void sendThing(int second){
  Serial.print("\n Sending Data");

  ThingSpeak.setField(1, second);
  // ThingSpeak.setField(2, voltage); 
  // ThingSpeak.setField(3, current);
  // ThingSpeak.setField(4, power);
  // delay(1000);
  ThingSpeak.setStatus(myStatus);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
    Serial.println("\n=============================================");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
    Serial.println("\n=============================================");
  }
}

void setupWiFi() {
    WiFiManager wm;
    bool res;
    res = wm.autoConnect("EV Connect","11110000"); // password protected
    if(!res) {
      Serial.println("Failed to connect");     
    } 
    else {
      Serial.println("connected...:) | Yoohoo!!");
    }
}
