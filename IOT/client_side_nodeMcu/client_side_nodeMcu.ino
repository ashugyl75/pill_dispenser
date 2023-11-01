#include <Servo.h>

Servo myservo;  // Create a servo object

void setup() {
  myservo.attach(D7,500,2500);  // Attach the servo to pin 9
}

void loop() {
  float pos;

  // Move the servo from 0 to 180 degrees
  for (pos = 0; pos <= 180; pos += 0.5) {
    myservo.write(pos);  // Set the servo position
    delay(5);           // Wait for the servo to reach the desired position
  }

  // Move the servo from 180 to 0 degrees
  for (pos = 180; pos >= 0; pos -= 0.5) {
    myservo.write(pos);  // Set the servo position
    delay(5);           // Wait for the servo to reach the desired position
  }
}
