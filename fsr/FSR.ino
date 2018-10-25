#include <Servo.h>
int emgpin = A5;

void setup() {
  // put your setup code here, to run once:
  pinMode(emgpin, INPUT);
  Serial.begin(9600);
}

void loop() {
  Serial.println(analogRead(emgpin));
  delay(25);

}
