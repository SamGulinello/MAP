/**
* @author Keith Lim, Sam Gullinello, Keller Martin, Jared _____
* The Main file for working the MAP
* Consists of the latest working code as of 10/4/18
**/

#include <Servo.h>
Servo myservo;
Servo myservo2;
int pos = 0;
int servoState = 0;
bool gripOpen = false;
int thresholdPot = A4;
int emg = A5;
int emgArray[25] = {0};
int readIndex;
int emgValue;
int rmsValue;
long total;
int getRMSSignal = 0;
int *maxNum;
int *first;
int *last;
int maxValue = 0;
int fsr = A0;
int led = 4;
int BatteryLevelReadBoth = A2;
int BatteryLevelReadBat2 = A3;
int BatteryLevelLEDR = D2;
int BatteryLevelLEDG = D3;
int BatteryLavelLEDB = D4;



void setup() {
  // put your setup code here, to run once:
  myservo.attach(10);
  myservo2.attach(11);
  myservo.write(servoState);
  myservo2.write(servoState);
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
}

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
  bool openGrip;
  bool currentGrip;
  int16_t maxSignal;
  int16_t minSignal;
  int amountOfSeconds;
  int16_t fsrReading;


public:
  MuscleMotor(int16_t, int16_t);                  
  void readSignal(int16_t);
  bool checkGripPosition(int16_t);
  void setMaxSignal(int16_t);
  int  rms(int);
  void openCloseActuator();  
  void setFsrReading(int16_t);          
};

/**
* Set the fields to a default value.
**/
MuscleMotor::MuscleMotor(int16_t maxsignal, int16_t minsignal)
{
  this->openGrip = true;
  this->currentGrip = true;
  this->maxSignal = maxsignal;
  this->minSignal = minsignal;
  this->amountOfSeconds = 0;
}

void MuscleMotor::setMaxSignal(int16_t maxSignal)
{
  this->maxSignal = maxSignal;
}

void MuscleMotor::setFsrReading(int16_t fsrReading){
  this->fsrReading = fsrReading;
}


// check to see if the grip should be open or close
// have to make a new function that calculates 2 seconds
bool MuscleMotor::checkGripPosition(int16_t bicepValue)
{

  // A hack to allow the actuator to work. We've to change
  // the amount of seconds to be more than the time for
  // the grip to change. Once it is higher than the time
  // for the grip to change (2000 or 2 seconds). We change
  // the time press back to 0;

  if (amountOfSeconds >= 2000) {
  amountOfSeconds = 0;
  }
  
  //If the muscle is squeezed for 1.5 seconds, Switch the
  //grip, save the grip to the currentGrip and then
  // return the openGrip.
  if (amountOfSeconds >= 1900) {

    openGrip = !openGrip;
    currentGrip = openGrip;
    amountOfSeconds+=100;
    return openGrip;

    //if the value of Bicep and tripcep is not high (not squeezed)
    // return the currentGrip Position.
  } else if (bicepValue < maxSignal) {
    amountOfSeconds = 0;
    return currentGrip;

    //if the muscles is squeezed for less than 2 seconds,
    //delay the system for 1 millisecond, then add 1 millisecond
    // to the total amount of seconds.
  } else {
    delay(100);
    amountOfSeconds += 100;
  }

}

/**
* Calculates Root Mean Square (RMS) of last 25 readings when called, including the newest emgValue reading
* Removes the maximum value, in order to guarentee a more uniform reading
**/
int MuscleMotor::rms(int emgValue) {
  //Updates array with new value from the emg
  total = total - emgArray[readIndex];
  emgArray[readIndex] = sq(emgValue);
  total = total + emgArray[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= 25) {
    readIndex = 0;
  }


  //adds maximum value back in, updates it, and then removes it again
  total += maxValue;                     

  first = emgArray;
  last = emgArray+24;

  maxNum = maxElement(first, last);
  maxValue = *maxNum;

  total -= *maxNum;                       


  //calculates rms
  rmsValue = (sqrt(total/24));

  //Print things to the monitor. Creates the plot
  Serial.print(emgValue);
  Serial.print("\t");
 // Serial.print(threshold);
  Serial.print(this->maxSignal);
  Serial.print("\t");
  Serial.print(this->fsrReading);
  Serial.print("\t");
  Serial.println(rmsValue);
  delay(25);

  return rmsValue;
}

/**
*  Open or close the actuator based on a boolean ___________ and a pressLength int
**/

void MuscleMotor::openCloseActuator() {
  // if we receive a boolean that says, open is true
  // we move the actuator so that it opens.
  // else we close it.

  if (currentGrip) {


    if (amountOfSeconds >= 2000) {                                                         
      //writing onto the servo to open it (extend it)
      digitalWrite(led, HIGH);
      for (pos = 0; pos < 180; pos = pos + 1){
        myservo2.write(pos);
        myservo.write(pos);
        delay(5);
        if(pos == 179){
          digitalWrite(led, LOW);
        }
        if(fsrReading > 500){
          break;
        }
      }

    }
  } else {
    if (amountOfSeconds >= 2000) {
      //writing onto the servo to close it (retract it)
      
      for (pos = 180; pos > 1; pos = pos - 1) {
        myservo2.write(pos);
        myservo.write(pos);
        delay(5);
        
      }
    }

  }
}

int* maxElement(int * first, int * last){
  
  maxNum = first;
  
  while(++first != last){
    if(*first > *maxNum){
      maxNum = first;
    }
  }

  return maxNum;
}


//Instantiate the class. Default threshold set to 25. 
MuscleMotor* mm = new MuscleMotor(25, 0);

void loop() {
  // put your main code here, to run repeatedly:
  // Rule of thumb for optimization:
  // The code within this box should not be more than 8 lines

  //set fsrReading variable
  mm->setFsrReading(analogRead(fsr));

  getRMSSignal = mm->rms(analogRead(emg) - 334);
  //getRMSSignal = mm->rms(analogRead(emg) - 575);

  // Setting variable threshold
  mm->setMaxSignal(analogRead(thresholdPot)/10);
  
  gripOpen = mm->checkGripPosition(getRMSSignal);
  mm->openCloseActuator();

}
