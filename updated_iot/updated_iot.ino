#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 2. Define the API Key */
#define API_KEY "AIzaSyBCWhuPBFDChYaqqY-VMmLlFtd4lhZkAWs"

/* 3. Define the RTDB URL */
#define DATABASE_URL "pillgrim-2b6cd-default-rtdb.asia-southeast1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "lmdujari33@gmail.com"
#define USER_PASSWORD "Test123"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif



#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>


#define ledpin 1;
#define trigpin 2;
#define echopin 3;
#define servo_morn 4;
#define servo_noon 5;
#define servo_even 6;


int pins[7] = {0,D0, D1, D2, D3, D4, D8};

#define morn 1;
#define noon 2; 
#define even 3;

String preset_time[4]={" "," "," "," "};
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800); //19800 is utc time offset for india



int button_state;

int whattime = 0; //which motor's time is it to run


const char* ssid = "internet";  // connect to hotspot
const char* password = "12345678";

int update_time; //to track if there is any new update from the server



void setup() {
  // put your setup code here, to run once:

  pinMode(pins[trigpin], OUTPUT);
  pinMode(pins[echopin], INPUT);
  pinMode(pins[ledpin], OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.begin(ssid, password);
//check if connected to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

//  Added : lakshya DUjari
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
 Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

// Code Ended 02/11/2023

timeClient.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  // firebase code
  int temp_update_time;
  int update_val;

  // fetching update time variable from firebase
  Firebase.Firestore.getInt(fbdo, "/update/update_time", &temp_update_time)? Serial.println("successfully fetched update_time value") : Serial.println("unable to fetch update_time variable, ERROR: %s",fbdo.errorReason().c_str());

  // if got zero value //null case
  if(temp_update_time==0)
  {
    Serial.println("update_time variable got a value other than integer data type");
  }
//------------------------------------------------------------------------------------if we get an input from firebase-----------------------------------------------------------------------------------------------------------------
  // if got a new update_time variable, different from the previous one
  else if(update_time != temp_update_time){ //we got a new update
      Serial.println("we got a update from the server!!!");
      update_time = temp_update_time; //updating the global update_time variable

      // fetch update val variable to see which value has been updated
      Firebase.Firestore.getInt(fbdo, "/update/update_val", &update_val) ? Serial.println("successfully fetched update_val value") : Serial.println("unable to fetch update_val variable, ERROR: %s",fbdo.errorReason().c_str());

      if(update_val<4 && update_val>0) // then the command is to set time
      {
        Firebase.Firestore.getString(fbdo, "/preset_time/" + update_val), &preset_time[update_val]) ? Serial.println("successfully fetched preset_time value") : Serial.println("unable to fetch preset_time variable, ERROR: %s",fbdo.errorReason().c_str());
      }

      else if(update_val>3 && update_val <7){ // then the command is to dispense pill
        int day;
        Firebase.Firestore.getInt(fbdo, "/event/day", &day) ? Serial.println("successfully fetched day value") : Serial.println("unable to fetch day variable, ERROR: %s",fbdo.errorReason().c_str());

        Serial.println("droping a pill on command");
        drop_pill(pins[update_val], update_val,  day);
      }

      else{ // we get an undesignated value, error
        Serial.println("update_time variable got a value other than predefined values");
      }

  }
//------------------------------------------------------------------------------------end of firebase input code-----------------------------------------------------------------------------------------------------------------



//  Comment Added : Lakshya Dujari (02/11/2023)

  
//  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
//  {
//    sendDataPrevMillis = millis();
//    Serial.println();
//  }
// Comment Ended : Lakshya Dujari (02/11/2023)
  
  


//----------------------------------------------------------------------------select which motors time is it to run based on current time----------------------------------------------------------------------------------------------

  switch(timeClient.getFormattedTime()) {
    case preset_time[morn]:
      whattime = servo_morn; //4
      break;
    case preset_time[noon]:
      whattime = servo_noon; //5
      break;
    case preset_time[even]:
      whattime = servo_even; //6
      break;
    default:
      break;
  }

//------------------------------------------------------------------------------------------------------end-----------------------------------------------------------------------------------------------------------------------------
  
  timeClient.update();
  button_state = UV_output();
  
  button_state? digitalWrite(pins[ledpin], HIGH) : digitalWrite(pins[ledpin], LOW);

  if(button_state && whattime!= 0){ // button is pressed and it is time to take medicine
    int current_day = timeClient.getDay()
    drop_med(pins[whattime], whattime%3, current_day); // activates whatever servo is required to run and sends data to current day and time
    
    whattime = 0;
    
  }





}

void drop_med(int servo, int time, int current_day){
  Servo s;
  s.attach(servo,500,2500);
  s.write(45);
  s.detach();
 // update the boolean in the database to true
  
  firbase_update(current_day, time); // change the current day and time's vale to true
}


bool UV_output() {
  long duration, distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.0343) / 2;

  if (distance < 7) {
    return true; // hand is less than 7 cm away return true
  } else {
    return false; // hand is 7 cm or farther away return false
  }
}



void firebase_set(bool data){
  Serial.printf("Set bool... %s\n", Firebase.setBool(fbdo, F("/monday/morning/pill_taken"),false) ? "ok" : fbdo.errorReason().c_str());
  
//  Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/test/int"), count) ? "ok" : fbdo.errorReason().c_str());
//  Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/test/float"), count + 10.2) ? "ok" : fbdo.errorReason().c_str());
//  Serial.printf("Set double... %s\n", Firebase.setDouble(fbdo, F("/test/double"), count + 35.517549723765) ? "ok" : fbdo.errorReason().c_str());
//  Serial.printf("Set string... %s\n", Firebase.setString(fbdo, F("/test/string"), "Hello World!") ? "ok" : fbdo.errorReason().c_str());
}
String firebase_get(){
//  Serial.printf("Get bool... %s\n", Firebase.getBool(fbdo, FPSTR("/test/bool")) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());
//  bool bVal;
//  Serial.printf("Get bool ref... %s\n", Firebase.getBool(fbdo, F("/test/bool"), &bVal) ? bVal ? "true" : "false" : fbdo.errorReason().c_str());
//  Serial.printf("Get int... %s\n", Firebase.getInt(fbdo, F("/test/int")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
//  int iVal = 0;
//  Serial.printf("Get int ref... %s\n", Firebase.getInt(fbdo, F("/test/int"), &iVal) ? String(iVal).c_str() : fbdo.errorReason().c_str());
//  Serial.printf("Get float... %s\n", Firebase.getFloat(fbdo, F("/test/float")) ? String(fbdo.to<float>()).c_str() : fbdo.errorReason().c_str());
//  Serial.printf("Get double... %s\n", Firebase.getDouble(fbdo, F("/test/double")) ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());
  Serial.printf("Get string... %s\n", Firebase.getString(fbdo, F("/test/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
}

//  updateNode(FirebaseData &fbdo, T path, FirebaseJson &json)


void firebase_update(int day, int time){
  FirebaseJson json;
  String day = {"Monday", "Tueday", "Wednesday", "Thursday", "Friday", "Saturday"};
  String time = {"Morning", "Afternoon", "Evening"};

  if (count == 0)
  {
    json.set("value/round/" + String(count), F("cool!"));
    json.set(F("vaue/ts/.sv"), F("timestamp"));
    Serial.printf("Set json... %s\n", Firebase.set(fbdo, F("/test/json"), json) ? "ok" : fbdo.errorReason().c_str());
  }
  else
  {
    json.add(String(count), "smart!");
    Serial.printf("Update node... %s\n", Firebase.updateNode(fbdo, F("/test/json/value/round"), json) ? "ok" : fbdo.errorReason().c_str());
  }

}

void refill_mode(){

}