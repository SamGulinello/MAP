/**
* @author Keith Lim, Sam Gullinello, Keller Martin, Jared Butler
* The Main file for working the MAP
* Consists of the latest working code as of 11/15/18
**/

/*
 * We want to add a function to center the EMG values around zero
 * automatically, so the RMS function works as smoothly as possible
 * 
 * We should try to make the code as battery-efficient as possible
 * 
 * Consider making the RMS array length (25) smaller, so we have a 
 * faster response time to changes in the EMG reading
 * 
 * Make sure FSR code is working
 * 
 * Test if removing the max value actually does anything useful
 * 
 * Change print values to a scale of 0V to 5V
 */

#include <Servo.h>

Servo myservo;
Servo myservo2;
int pos = 0;
int servoState = 0;
bool gripOpen = false;
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

// Analog Pins
int fsr = A5;
int BatteryLevelReadBoth = A3;
int BatteryLevelReadBat2 = A2;
int thresholdPot = A4;
int emg = A1;

//Digital Pins
int BatteryLevelLEDR = 2;
int BatteryLevelLEDG = 4;
int BatteryLevelLEDB = 3;
int led = 7;
int servo = 10;
int servo2 = 11;



void setup() {
  // put your setup code here, to run once:
  myservo.attach(servo);
  myservo2.attach(servo2);
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
  int amountOfSeconds;
  int16_t fsrReading;


public:
  MuscleMotor();                  
  void readSignal(int16_t);
  bool checkGripPosition(int16_t);
  void setMaxSignal(int16_t);
  int  rms(int);
  void openCloseActuator();  
  void setFsrReading(int16_t);
  void indicateBatteryLevel();          
};

/*
 * New class to make printing easier
 */
class Monitor {
private:
  double factor;
  int precision;
  
public:
  Monitor();
  void p(int);
  void pln(int);
};

Monitor* Print = new Monitor();


//Instantiate the class. Default threshold set to 25. 
MuscleMotor* mm = new MuscleMotor();

Monitor::Monitor(){
  this->factor = .00488759;
  this->precision = 3;
}

void Monitor::p(int in){
  Serial.print(in * factor, precision);
  Serial.print("\t");
}

void Monitor::pln(int in){
  Serial.println(in * factor, precision);
}

/**
* Set the fields to a default value.
**/
MuscleMotor::MuscleMotor()
{
  this->openGrip = true;
  this->currentGrip = true;
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
  Serial.print(this->maxSignal);
  Serial.print("\t");
  Serial.print(this->fsrReading);
  Serial.print("\t");
  Serial.println(rmsValue);
  //Serial.print("\t");
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
      //writing onto the servo to close it
      digitalWrite(led, HIGH);
      for (; pos < 180; pos = pos + 1){
        myservo2.write(pos);
        myservo.write(pos);
        mm->setFsrReading(analogRead(fsr));
        delay(5);
        
        if(fsrReading > 600){
          //digitalWrite(led, LOW);
          break;
        }
      }
    }
    
  } else {
    if (amountOfSeconds >= 2000) {
      //writing onto the servo to open it
      digitalWrite(led, LOW);
      for (; pos > 1; pos = pos - 1) {
        myservo2.write(pos);
        myservo.write(pos);
        delay(5);
        
      }
    }
    
  }

  
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


int* maxElement(int * first, int * last){
  
  maxNum = first;
  
  while(++first != last){
    if(*first > *maxNum){
      maxNum = first;
    }
  }

  return maxNum;
}



void loop() {
  // put your main code here, to run repeatedly:
  // Rule of thumb for optimization:
  // The code within this box should not be more than 8 lines

  mm->indicateBatteryLevel();

  //set fsrReading variable
  mm->setFsrReading(analogRead(fsr));

  //getRMSSignal = mm->rms(analogRead(emg) - 334);
  getRMSSignal = mm->rms(analogRead(emg) - 500);

  // Setting variable threshold
  mm->setMaxSignal(analogRead(thresholdPot));
  
  gripOpen = mm->checkGripPosition(getRMSSignal);
  mm->openCloseActuator();

}
