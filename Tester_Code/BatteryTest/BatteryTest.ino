/*  Code to open and close the hand repeatedly to test
 *  the battery
 */

#include <Servo.h>
Servo myservo;
Servo myservo2;
int pos = 0;
int servoState = 0; //this means the hand starts opened
bool handState = false; //false indicates that hand is open
unsigned int beginTime;
unsigned int startTime;

void setup() {
  // put your setup code here, to run once:
  myservo.attach(10);
  myservo2.attach(11);
  myservo.write(servoState);
  myservo2.write(servoState);
  Serial.begin(9600);

  unsigned int beginTime = millis();
  unsigned int startTime = millis();
}


void moveMotors(){
  if(handState){
    
    //opens hand
    for (/*pos = 180*/; pos > 1; pos = pos - 1) {
        myservo2.write(pos);
        myservo.write(pos);
        delay(5); 

    }  

    handState = !handState;
  } else {
    
    //closes hand
    for (/*pos = 0*/; pos < 180; pos = pos + 1){
        myservo2.write(pos);
        myservo.write(pos);
        delay(5);

    }

    handState = !handState;
  }
}


void loop() {
  // put your main code here, to run repeatedly:

  if(millis() >= beginTime + 2000){
    moveMotors();

    beginTime = millis();

    Serial.println((millis() - startTime) / 1000);
  }

  
}
