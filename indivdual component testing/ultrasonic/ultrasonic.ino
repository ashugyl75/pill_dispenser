void setup() {
  // put your setup code here, to run once:
  pinMode(D7, OUTPUT);
  pinMode(D8, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  double duration, distance;

  digitalWrite(D7, LOW);
  delayMicroseconds(2);

  digitalWrite(D7, HIGH);
  delayMicroseconds(10);
  digitalWrite(D7, LOW);

  duration = pulseIn(d8, HIGH);
  distance = (duration * 0.0343) / 2.0;
  Serial.print("duration from UV sensor");
  Serial.println(duration);
  Serial.print("distance from UV sensor: ");
  Serial.println(distance);

}

// bool UV_output() {
  
//   if ((distance>2) && (distance<15)) {
//     return true; // hand is less than 7 cm away return true
//   } else {
//     return false; // hand is 7 cm or farther away return false
//   }
// }
