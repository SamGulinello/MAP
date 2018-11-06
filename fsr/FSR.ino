#include <Servo.h>
int emgpin = A1;
int emgValue;

void setup() {
  // put your setup code here, to run once:
  pinMode(emgpin, INPUT);
  Serial.begin(9600);
}

void loop() {
  emgValue = analogRead(emgpin);
  Serial.println(emgValue);
  delay(25);

}
