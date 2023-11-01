/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>

ESP8266WiFiMulti WiFiMulti;
const char* ssid = "sunil";  // connect to hotspot
const char* password = "12345678";
const char* serverUrl = "http://192.168.1.11:5000/receive_data";  // flask server

int button_state;

const int servo = D4; //servo control pin number
const int button = D1; //button pin number
bool booltime = false;  // boolean to track if it is time to take pill
Servo s;               // servo variable
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800); //19800 is utc time offset for india

unsigned long previous;
String preset_time = "14:16:00";  // preset time for taking the medicine


void setup() {
  pinMode(button, INPUT);
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  timeClient.begin();
}

void loop() {
  
  timeClient.update();          // update time
  button_state = digitalRead(button); // read if the button has been pressed
  // Serial.println(button_state);
  // send_message("abc");
  // Serial.println(timeClient.getFormattedTime());

  if(timeClient.getFormattedTime()==preset_time){ // if time is exactly equal to preset time
      send_message("time%20to%20take%20your%20pill", timeClient.getFormattedTime());
      booltime = true;
      previous = millis();
      delay(1000);
    }

    // delay(100000); //debugging purpose
  if(!button_state && booltime){ //if button has not been pressed and time has gone by for taking medicine
      if(((millis()-previous)==10000)){ //if time since sending first message is equal to 5 mins
        send_message("You%20forgot%20to%20take%20your%20pill", timeClient.getFormattedTime());
        previous = millis();
      }
    }
    
    if(button_state && booltime){ // if button is pressed and preset time has gone by
      s.attach(servo);
      // for (int pos = 0; pos <= 180; pos += 1) {
      // // tell servo to go to position in variable 'pos'
      // s.write(pos);
      // // wait 15 ms for servo to reach the position
      // delay(15); // Wait for 15 millisecond(s)
      // }
      s.write(90);
      delay(100);
      s.write(90);
      s.detach();
      booltime = false;
      send_message("you%20took%20your%20pill,%20thank%20you", timeClient.getFormattedTime());
    }

}

   


void send_message(String str, String time){
    if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;
    String url = String(serverUrl) + "?message=" + String(str)+"&time=" + String(time);
    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, url)) {  // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.println("[HTTP] Unable to connect");
    }
  }
}
