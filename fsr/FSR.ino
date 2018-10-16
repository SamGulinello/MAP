#include <Servo.h>
int FSRpin = A5;

void setup() {
  // put your setup code here, to run once:
  pinMode(FSRpin, INPUT);
  Serial.begin(9600);
}

void loop() {
  Serial.print(600);
  Serial.print("\t");
  Serial.println(analogRead(FSRpin));
  delay(25);

}
