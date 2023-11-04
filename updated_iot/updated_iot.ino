
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

#include <Wire.h>  // This library is already built in to the Arduino IDE
#include <LiquidCrystal_I2C.h> //This library you can add via Include Library > Manage Library > 
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "internet"
#define WIFI_PASSWORD "12345678"

/* 2. Define the API Key */
#define API_KEY "AIzaSyDBdbu-84whBLw97Al4DYEk5uTNJEswKv4"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "pillgrim-1de80"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "lmdujari33@gmail.com"
#define USER_PASSWORD "Test123"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

// bool taskCompleted = false;

unsigned long dataMillis = 0;




#include <addons/RTDBHelper.h>



/* 3. Define the RTDB URL */
// #define DATABASE_URL "pillgrim-2b6cd-default-rtdb.asia-southeast1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app





#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>


#define buzzer 1 //D6
#define trigpin 2 //D7
#define echopin 3 //D8
#define servo_morn 4 //D0
#define servo_noon 5 // D1
#define servo_even 6 // D2
#define SDA 7 // D4
#define SCL 8  //D3



int pins[9] = {0,D6,D7,D8,D0,D1,D2,D4,D3}; // try D2 -- SDA and D1 -- SCL for correct lcd work
Servo s;
#define morn 1
#define noon 2
#define even 3

String preset_time[4]={" ", "09:30:00","13:30:00","20:30:00"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800);  //19800 is utc time offset for india



int button_state;

int whattime = 0; //which motor's time is it to run

String DAYS[7] = {"Sunday","Monday","Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String TIME[3] = {"Morning", "Afternoon", "Evening"};


String update_time = ""; //to track if there is any new update from the server



void setup() {

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//check if connected to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
//-------------------------------------------------------------IOT code setup-------------------------------------------------------------------------------
  pinMode(pins[trigpin], OUTPUT);
  pinMode(pins[echopin], INPUT);
  pinMode(pins[buzzer], OUTPUT);

  pinMode(pins[servo_morn], OUTPUT);
  pinMode(pins[servo_noon], OUTPUT);
  pinMode(pins[servo_even], OUTPUT);


  Serial.println();
  Serial.println();
  Serial.println();

  Wire.begin(pins[SDA],pins[SCL]);
  lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight

  lcd.print("Welcome :D");
  delay(2000);
  lcd.clear();
  lcd.print("Team:Innovators");
  lcd.clear();
  lcd.print("PillGrim ON");
  // ------------------------------------------------------------------IOT code end-----------------------------------------------------------------------------------------
  //------------------------------------------------------------------firbase configuration----------------------------------------------------------------------------------------
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);
  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
 Firebase.begin(&config, &auth);
  //------------------------------------------------------------------firebase config code end----------------------------------------------------------------------------------------

  // Firebase.setDoubleDigits(5);

// Code Ended 02/11/2023

timeClient.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  // firebase code
  String temp_update_time ="";
  String update_val;

  // fetching update time variable from firebase

  if(firebase_get_string("/Update/Update_chk","update_time", temp_update_time, "timestamp")){
    Serial.print("succesfully fetched update time value: ");
    Serial.println(temp_update_time);
  }
  else
    Serial.println("unable to fetch update time value");

  

//------------------------------------------------------------------------------------if we get an input from firebase-----------------------------------------------------------------------------------------------------------------
  // if got a new update_time variable, different from the previous one
  if(update_time != temp_update_time){ //we got a new update
      Serial.println("we got a update from the server!!!");
      update_time = temp_update_time; //updating the global update_time variable

      // fetch update val variable to see which value has been updated
      //Firebase.Firestore.getInt(fbdo, "/update/update_val", &update_val) ? Serial.println("successfully fetched update_val value") : Serial.println("unable to fetch update_val variable, ERROR: %s",fbdo.errorReason().c_str());

      if(firebase_get_string("/Update/Update_chk","update_val", update_val, "integer"))
        Serial.println("succesfully fetched update val");
      else
        Serial.println("unable to fetch update val");
      int update_val_int = update_val.toInt();

      if(update_val_int == 123) // then the command is to set time
      {
        //Firebase.Firestore.getString(fbdo, "/preset_time/" + update_val), &preset_time[update_val]) ? Serial.println("successfully fetched preset_time value") : Serial.println("unable to fetch preset_time variable, ERROR: %s",fbdo.errorReason().c_str());
        if(firebase_get_string("/Schedule/Monday","Morning", preset_time[1], "string") && firebase_get_string("/Schedule/Monday","Afternoon", preset_time[2], "string") && firebase_get_string("/Schedule/Monday","Evening", preset_time[3], "string")){
          Serial.println("succesfully updated preset time array");
          lcd.clear();
          lcd.print("Morn: ");
          lcd.print(preset_time[1]);
          delay(2000);
          lcd.clear();
          lcd.print("Noon: ");
          lcd.print(preset_time[2]);
          delay(2000);
          lcd.clear();
          lcd.print("Even: ");
          lcd.print(preset_time[3]);
          delay(2000);
          lcd.clear();
          lcd.print("PillGrim ON");
        }  
        else
          Serial.println("unable to set preset time array");
      }

      else if(update_val_int>3 && update_val_int <7){ // then the command is to dispense pill
        String day;
        // Firebase.Firestore.getInt(fbdo, "/event/day", &day) ? Serial.println("successfully fetched day value") : Serial.println("unable to fetch day variable, ERROR: %s",fbdo.errorReason().c_str());
        if(firebase_get_string("/Update/Event","Day", day, "integer"))
          Serial.println("succesfully fetched Day variable");
        else
          Serial.println("unable to fetch day variable");

        Serial.println("droping a pill on command");
        drop_med(pins[update_val_int], update_val_int,  day.toInt());
      }

      else if(update_val_int==7){
        refill_mode();
      }
      else{ // we get an undesignated value, error
        Serial.print("update_time variable got a value other than predefined values: "); // other than 123, 4,5,6
        Serial.println(update_val_int);
      }

  }
  else{
    Serial.println("no new update from the server");
  }
//------------------------------------------------------------------------------------end of firebase input code-----------------------------------------------------------------------------------------------------------------



//  Comment Added : Lakshya Dujari (02/11/2023)

  
//  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
//  {
//    sendDataPrevMillis = millis();
//    Serial.println();
//  }
// Comment Ended : Lakshya Dujari (02/11/2023)
  
  


//------------------------------------------------select which motors time is it to run based on current time---------------------------------------------------------------------------------------
  String current_time = timeClient.getFormattedTime();

  if(current_time==preset_time[morn])
    whattime = servo_morn; //4
  else if(current_time==preset_time[noon])
    whattime = servo_noon; //5
  else if(current_time==preset_time[noon])
    whattime = servo_even; //6


//------------------------------------------------------------------------------------------------------end-----------------------------------------------------------------------------------------------------------------------------
  
  timeClient.update();
  button_state = UV_output();
  Serial.print("UV ON/OFF :");
  Serial.println(button_state);
  if(button_state){
    Buzzer(4);
  }

  if(button_state && whattime!= 0){ // button is pressed and it is time to take medicine
    int current_day = timeClient.getDay();
    drop_med(pins[whattime], whattime%3, current_day); // activates whatever servo is required to run and sends data to current day and time
    
    whattime = 0;
    
  }



delay(1000);

}

void drop_med(int servo, int time, int current_day){
  // 45--1, 90--2, 135--3, 180--4
  if(current_day==5){ // if the day is friday return, it is time to refill
    lcd.clear();
    lcd.print("out of pills, please Refill");
    delay(2000);
    return;
  }
  s.attach(servo,500,2500);
  lcd.clear();
  lcd.print(".........");
  lcd.clear();
  delay(500);
  lcd.print("dispatching pill.....");
  delay(500);
  Buzzer(3); // Buzz for 3 seconds
  s.write(45*current_day);
  s.detach(); // detaching the servo
  delay(1000);
  lcd.clear();
  lcd.print("thank you for ");
  lcd.setCursor(0,1);
  lcd.print("taking your pill");
  delay(3000);
  lcd.clear();
  lcd.print("PillGrim is ON");
 // update the boolean in the database to true
  
  // firbase_update(current_day, time); // change the current day and time's vale to true
}


bool UV_output() {
  double duration, distance;

  digitalWrite(pins[trigpin], LOW);
  delayMicroseconds(2);

  digitalWrite(pins[trigpin], HIGH);
  delayMicroseconds(10);
  digitalWrite(pins[trigpin], LOW);

  duration = pulseIn(pins[echopin], HIGH);
  distance = (duration * 0.0343) / 2.0;
  Serial.print("duration from UV sensor");
  Serial.println(duration);
  Serial.print("distance from UV sensor: ");
  Serial.println(distance);
  if ((distance>2) && (distance<15)) {
    return true; // hand is less than 7 cm away return true
  } else {
    return false; // hand is 7 cm or farther away return false
  }
}



bool firebase_get_string(String path, String mask, String& temp_update_time, String datatype){
  FirebaseJsonData result;
  FirebaseJson json;
  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", path, mask)){
    // Serial.println(fbdo.payload().c_str()); //debug printf
    json.setJsonData(fbdo.payload().c_str());
    if (json.get(result, "fields/" + mask + "/"+ datatype +"Value")) {
      if (result.type == "string") {
        temp_update_time = result.to<String>();
        return true;
      } else {
          Serial.println(mask + " is not a string.");
          return false;
      }
    } else {
        Serial.println(mask + " field not found.");
        return false;
    }

  }  
  else
    Serial.println(fbdo.errorReason());
  return false;
}

//  updateNode(FirebaseData &fbdo, T path, FirebaseJson &json)


// void firebase_update(int day, int time){
//   FirebaseJson json;  

  

// }

void Buzzer(int time){
  int sec = 0;
  while(sec<=time){
    digitalWrite(pins[buzzer], HIGH);
    delay(200);
    digitalWrite(pins[buzzer], LOW);
    delay(200);  
    sec++;
  }
}

void refill_mode(){
  // // 45--1, 90--2, 135--3, 180--4
  // lcd.clear();
  // lcd.print("Refill Mode On...");
  // delay(500);
  // for(int servo=4; servo<=6; servo++){
  //   s.attach(pins[servo],500,2500);
  //   for(int day = 4; day>0; day--){
  //     s.write(day*45);
  //     lcd.clear();
  //     lcd.print("Refill " + DAYS[day]);
  //     delay(2000);
  //     lcd.setCursor(0,1);
  //     lcd.print(TIME[servo-4] + " pill");
  //     Buzzer(1);
  //     for(int temp=20; temp>=0; temp--){
  //       lcd.clear();
  //       // lcd.print("Time Left: ");
  //       // lcd.print(itoa(temp));
  //       delay(1000);// one sec delay
  //     }
  //     Buzzer(1);
  //   }
  //   s.write(0);
  //   s.detach();
  // }
  // // lcd.clear();
  // // lcd.print("Refill Complete!!!");
  // // lcd.setCursor(0,1);
  // // lcd.print("Thank You!");
  // // delay(2000);
  // lcd.clear();
  // lcd.print("PillGrim is ON");
}