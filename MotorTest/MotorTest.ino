#include <Servo.h>
Servo myservo;
Servo myservo2;
int pos = 0;
int servoState = 0;
int input = A0;
int inputVal = 0;
bool oldValGreaterThan600 = false;
bool currentValGreaterThan600 = false;
bool handState = false;

void setup() {
  // put your setup code here, to run once:
  myservo.attach(10);
  myservo2.attach(11);
  myservo.write(servoState);
  myservo2.write(servoState);
  pinMode(input, INPUT);
  Serial.begin(9600);
}


void moveMotors(bool handState){
  if(handState){
    
    for (pos = 180; pos > 1; pos = pos - 1) {
        myservo2.write(pos);
        myservo.write(pos);
        delay(5); 
      }
      
  } else {
    
    for (pos = 0; pos < 180; pos = pos + 1){
        myservo2.write(pos);
        myservo.write(pos);
        delay(5);

        
        /*if(pos == 179){
          digitalWrite(led, LOW);
        }
        if(fsrReading > 500){
          break;
        }*/
    }
    
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  inputVal = analogRead(input);
  Serial.println(inputVal);

  if(inputVal > 600){
    currentValGreaterThan600 = true;
  } else {
    currentValGreaterThan600 = false;
  }

  if(currentValGreaterThan600 != oldValGreaterThan600){
    handState = !handState;
    moveMotors(handState);
  }

  oldValGreaterThan600 = currentValGreaterThan600;

  

  
}
