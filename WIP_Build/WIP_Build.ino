/**
* @author Keith Lim, Sam Gullinello, Keller Martin, Jared Butler
* The Main file for working the MAP
* Consists of the latest working code as of 11/15/18
**/

#include <Servo.h>
#include "Print.h"
#include "Timer.h"
Servo myservo;
Servo myservo2;
int initialState = 0;
const int EMG_ARRAY_LENGTH = 25;
int32_t emgArray[EMG_ARRAY_LENGTH] = {0};
const int TIME_OF_FLEX = 700;
int readIndex;
int32_t emgValue;
int32_t rmsValue;
int32_t total;
int getRMSSignal = 0;
int32_t *maxNum;
int32_t *first;
int32_t *last;
int32_t maxValue = 0;
int32_t emgRead = 0;

// Calibration Sequence Variables
int32_t emgAvg = 0;

// Analog Pins
int fsr = A1;
int BatteryLevelReadBoth = A3;
int BatteryLevelReadBat2 = A2;
int thresholdPot = A4;
int emg = A5;

//Digital Pins
int BatteryLevelLEDR = 2;
int BatteryLevelLEDG = 4;
int BatteryLevelLEDB = 3;
int led = 7;
int servo = 10;
int servo2 = 11;

//--------Below Are functions not in mm class---------------//
int32_t* maxElement(int32_t * first, int32_t * last){
  
  maxNum = first;
  
  while(++first != last){
    if(*first > *maxNum){
      maxNum = first;
    }
  }

  return maxNum;
}


//--------Start of mm Class----------------------------------//
/**
* Start of Muscle Motor class.
*
* Muscle Motor class takes in variables and signals and
* decide on what the output should be. For example
* Takes in a signal and outputs a boolean to signify that
* we should or not move the arm.
*
* ##grip is always open when true and close when false
**/
class MuscleMotor {
private:
  bool currentGrip;
  int16_t maxSignal;
  int amountOfSeconds;
  int16_t fsrReading;
  void openHand();
  void closeHand();
  int servoPos;

public:
  MuscleMotor();                  
  void readSignal(int16_t);
  void checkGripPosition(int32_t);
  void setMaxSignal(int32_t);
  int32_t  rms(int32_t);
  void openCloseActuator();  
  void setFsrReading(int32_t);
  void indicateBatteryLevel();
  void emgCal();         
};

//Instantiate printer class
Monitor* Print = new Monitor();

//Instantiate mm class.
MuscleMotor* mm = new MuscleMotor();

//Instantiate Battery Timer
Timer* Time = new Timer();

/**
* Set the fields to a default value.
**/
MuscleMotor::MuscleMotor()
{
  this->currentGrip = true;
  this->servoPos = 0;
}

void MuscleMotor::setMaxSignal(int32_t maxSignal)
{
  this->maxSignal = maxSignal;
}

void MuscleMotor::setFsrReading(int32_t fsrReading){
  this->fsrReading = fsrReading;
}


void MuscleMotor::checkGripPosition(int32_t rmsVal){
  
  if(!Time->getTime()){
    Time->resetTimer();
    
  } else if(Time->getTime() > TIME_OF_FLEX){
    currentGrip = !currentGrip;

    if(currentGrip){
      closeHand();
    } else{
      openHand();
    }
    
    Time->resetTimer();
    
  } else if(rmsVal < maxSignal){
    Time->resetTimer();
  }
  
}

void MuscleMotor::openHand(){

  digitalWrite(led, HIGH);
  for (; mm->servoPos > 1; mm->servoPos--) {
    myservo2.write(mm->servoPos);
    myservo.write(mm->servoPos);
    delay(5);
    
  }
  
}

void MuscleMotor::closeHand(){
  
  digitalWrite(led, LOW);
  for (; mm->servoPos < 180; mm->servoPos++){
    myservo2.write(mm->servoPos);
    myservo.write(mm->servoPos);
    mm->setFsrReading(analogRead(fsr));
    delay(5);
    
    if(fsrReading > 600){
      break;
    }
  }
  
}


/**
* Calculates Root Mean Square (RMS) of last 25 readings when called, including the newest emgValue reading
* Removes the maximum value, in order to guarentee a more uniform reading
**/
int32_t MuscleMotor::rms(int32_t emgValue) {
  //Updates array with new value from the emg
  total = total - emgArray[readIndex];
  emgArray[readIndex] = sq(emgValue);
  total = total + emgArray[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= EMG_ARRAY_LENGTH) {
    readIndex = 0;
  }

  //adds maximum value back in, updates it, and then removes it again
  total += maxValue;                     

  first = emgArray;
  last = emgArray+EMG_ARRAY_LENGTH;

  maxNum = maxElement(first, last);
  maxValue = *maxNum;

  total -= *maxNum;                       


  //calculates rms
  rmsValue = (sqrt(total/(EMG_ARRAY_LENGTH - 1)));

  //Print things to the monitor. Creates the plot
  Print->p(emgValue);
  Print->p(this->maxSignal);
  //Print->p(this->fsrReading);
  Print->pln(rmsValue);
  delay(25);

  return rmsValue;
}


/**
*  Light RGB LED to different colors to signal the battery level.
**/

void MuscleMotor::indicateBatteryLevel() {
  
  int bat2Level = analogRead(BatteryLevelReadBat2);
  int bat1Level = analogRead(BatteryLevelReadBoth);
  int greenThreshold = 818; // 4V*1023/5V
  int redThreshold = 655; // 3.2V*1023/5V
  //Serial.print(bat2Level);
  //Serial.print("\t");
  //Serial.println(bat1Level);

    digitalWrite(BatteryLevelLEDB, LOW);

    if((bat2Level > greenThreshold)/* && (bat1Level > greenThreshold)*/) {
    digitalWrite(BatteryLevelLEDR, LOW);
    digitalWrite(BatteryLevelLEDB, LOW);
    digitalWrite(BatteryLevelLEDG, HIGH);
    }
    
    else if((bat2Level > redThreshold)/* && (bat1Level > redThreshold)*/) {
      digitalWrite(BatteryLevelLEDR, LOW);
      digitalWrite(BatteryLevelLEDG, LOW);
      digitalWrite(BatteryLevelLEDB, HIGH);
    }
    
    else if((bat2Level <= redThreshold)/* || (bat1Level <= redThreshold)*/){
      digitalWrite(BatteryLevelLEDR, HIGH);
      digitalWrite(BatteryLevelLEDG, LOW);
      digitalWrite(BatteryLevelLEDB, LOW);
    }
    
}

void MuscleMotor::emgCal(){
  emgAvg = 0;
  
  for(int i = 0; i < 25; i++){
    emgAvg += analogRead(emg);
    delay(25);
  }

  emgAvg = emgAvg / 25;
}


void setup() {
  // put your setup code here, to run once:
  myservo.attach(servo);
  myservo2.attach(servo2);
  myservo.write(initialState);
  myservo2.write(initialState);
  pinMode(thresholdPot, INPUT);
  pinMode(emg, INPUT);
  pinMode(fsr, INPUT);
  pinMode(BatteryLevelReadBoth, INPUT);
  pinMode(BatteryLevelReadBat2, INPUT);
  pinMode(led, OUTPUT);
  pinMode(BatteryLevelLEDR, OUTPUT);
  pinMode(BatteryLevelLEDG, OUTPUT);
  pinMode(BatteryLevelLEDB, OUTPUT);

  Serial.begin(9600);
  *maxNum = 0;
  *first = 0;
  *last = 0;

  mm->emgCal();
}


void loop() {
  // put your main code here, to run repeatedly:
  // Rule of thumb for optimization:
  // The code within this box should not be more than 8 lines
  emgRead = analogRead(emg);

  mm->indicateBatteryLevel();

  //set fsrReading variable
  mm->setFsrReading(analogRead(fsr));

  //getRMSSignal = mm->rms(analogRead(emg) - 334);
  getRMSSignal = mm->rms(emgRead - emgAvg);

  // Setting variable threshold
  //mm->setMaxSignal(analogRead(thresholdPot));
  mm->setMaxSignal(25);
  
  mm->checkGripPosition(getRMSSignal);
}
