#include <HTTPClient.h>
#include <WiFiManager.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
WiFiClient  client;
WiFiUDP ntpUDP;

const long offsetTime = 25200;
NTPClient timeClient(ntpUDP, "pool.ntp.org", offsetTime);

unsigned long myChannelNumber = 2364449; //Input Your ChannelID Number
const char * myWriteAPIKey = "UNQ5FDQ8IBTWD3K7"; //Input Your API_Key ThinkSpeak

String GOOGLE_SCRIPT_ID = "AKfycbzNPVabOUt6ZOOCm4OgNpWVbeoLGB1_av_Wkl9KJ-cpr1nf6kSJKG6U0FSOGvgfIh4s";
int Control_Electric = 16;
int Status_Green = 17;
int Status_Yellow = 18;
int Status_Red = 19;

String finalresult = "";
String myStatus = "";
String result = "";
int year;
int month;
int day;
int hour;
int minute;
int second;
int count = 0;
bool set = false;

void setup() {
  Serial.begin(115200);
  pinMode(Control_Electric, OUTPUT);
  pinMode(Status_Green, OUTPUT);
  pinMode(Status_Yellow, OUTPUT);
  pinMode(Status_Red, OUTPUT);
  setupWiFi();

  // initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();

  lcd.setCursor(4, 1); // แถวที่ 2
  lcd.print("Starting Up");

  lcd.setCursor(4, 2); // แถวที่ 3
  lcd.print("The System..");

  timeClient.begin();
  timeClient.update();
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  second = timeClient.getSeconds();
  Serial.print(F("Time Begin : "));
  Serial.println(timeClient.getFormattedTime());
  delay(5000);
  lcd.clear();
}

void loop() {
  timeClient.update();
  Serial.println("\n=============================================\n");
  Serial.print(F("Count : "));
  Serial.println(count);
    if(count > 9){
      count = 0;
      set = true;
    }
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  second = timeClient.getSeconds();
  Serial.println(F("Time Begin : "));
  Serial.println(timeClient.getFormattedTime());
  //เรียกใช้ฟังชั่น
  getDataFromGoogleSheets();
  // sendDataNull("null");
  delay(1000);
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
      Serial.printf("   \n");
      Serial.printf(" Control_Electric = OFF \n");
      Serial.printf(" Status_Green = OFF \n");
      Serial.printf(" Status_Yellow = ON \n");
      Serial.printf(" Status_Red = OFF \n");
      Serial.printf("===============================================\n");
    //----------------------------------------
    digitalWrite(Control_Electric, HIGH);
    digitalWrite(Status_Green, HIGH);
    digitalWrite(Status_Yellow, LOW);
    digitalWrite(Status_Red, HIGH);
    //----------------------------------------
     Serial.printf("Exit Program!!! \n");
  } else {
      Serial.printf("===============================================\n");
      Serial.printf("This Defualt minutes in else : %d \n", conminutes);
      Serial.printf("   \n");
      Serial.printf(" Control_Electric = ON \n");
      Serial.printf(" Status_Green = ON \n");
      Serial.printf(" Status_Yellow = OFF \n");
      Serial.printf(" Status_Red = OFF \n");
      Serial.printf("===============================================\n");
      //----------------------------------------
      digitalWrite(Control_Electric, LOW);
      digitalWrite(Status_Green, LOW);
      digitalWrite(Status_Yellow, HIGH);
      digitalWrite(Status_Red, HIGH);
      //----------------------------------------
      for (int i = conminutes; i >= 0; i--) {
        Serial.print(F("Countdown = "));
        Serial.print(i);
        Serial.print(F(" Seconds\n"));
        lcd.setCursor(3, 1); // แถวที่ 2
        lcd.print("Wait : ");
        lcd.print(i);
        lcd.print(" Seconds");
        delay(1000);
      }
      sendDataNull(params);
    }
  lcd.clear();
  delay(1000);
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



void setupWiFi() {
    WiFiManager wm;
    bool res;
    res = wm.autoConnect("EV Connect","12345678"); // password protected
    if(!res) {
      Serial.println("Failed to connect");     
    } 
    else {
      Serial.println("connected...:) | Yoohoo!!");
    }
}