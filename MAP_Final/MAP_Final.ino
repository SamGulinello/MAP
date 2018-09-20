/**
* @author Keith Lim and Sam Gulinello
* The Main file for working the MAP
* Consist of the latest working code
**/
#include <Servo.h>
Servo myservo;
const int bicepPin = A1;
const int servoPin = 4;
int pos = 0;
int bicepValue = 0;
int servoState = 0;
unsigned long StartTime = 0;
unsigned long runningTime = 0;
int runningValue = 2000;
int currentTime = 0;
int pressedTime;
int releasedTime;
int pressLength = 0;
bool gripOpen = false;
int emg = A5;
int emgArray[25] = {0};
int readIndex;
int emgValue;
int rmsValue;
long total;
int getRMSSignal = 0;


void setup() {
  // put your setup code here, to run once:
  myservo.attach(4);
  myservo.write(servoState);
  pinMode(bicepPin,INPUT);
  pinMode(emg, INPUT);
  Serial.begin(115200);
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
  bool signalPeaked;
  int16_t signalFromSensor;
  int16_t maxSignal;
  int16_t minSignal;
  unsigned long currentCounter;
  unsigned long pastCounter;
  int amountOfSeconds;
  int pumpedMuscle;
  int pumpedMuscleCounter;


public:
  MuscleMotor(int16_t, int16_t);                    //done
  void sayHello();
  void readSignal(int16_t);
  int  getAmountOfSeconds();
  bool checkGripPosition(int16_t);

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
  this->signalPeaked = false;
  this->amountOfSeconds = 0;
  this->pumpedMuscleCounter = 0;
  this->pumpedMuscle = false;
}

/**
* @ReadSignal
* Takes in the signal and save it in a field
**/
void MuscleMotor::readSignal(int16_t signal)
{
  this->signalFromSensor = signal;
}

/**
* Outputs the amount of seconds we received the signals
**/
int MuscleMotor::getAmountOfSeconds()
{
  return this->amountOfSeconds;
}

// check to see if the grip should be open or close
// have to make a new function that calculates 2 seconds
bool MuscleMotor::checkGripPosition(int16_t bicepValue)
{

  //A hack to allow the actuator to work. We've to change
  // the amount of seconds to be more than the time for
  // the grip to change. Once it is higher than the time
  // for the grip to change (2000 or 2 seconds). We change
  // the time press back to 0;

  /**new**/
  if (amountOfSeconds >= 1600) {
    amountOfSeconds = 0;
  }

  // if (!openGrip) {


    //If the muscle is squeezed for 2 seconds, Switch the
    //grip, save the grip to the currentGrip and then
    // return the openGrip.
    if (amountOfSeconds >= 1500) {

      openGrip = !openGrip;
      currentGrip = openGrip;
      if (currentGrip) Serial.println("Changing bool to open");
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
  // } else {
  //   // if the muscle has been pumped twice and it is equal or over the timer
  //   // open the hand.
  //   if (pumpedMuscleCounter >= 2 && amountOfSeconds >= 1500) {
  //
  //     openGrip = false;
  //     currentGrip = openGrip;
  //     if (!currentGrip) Serial.println("Changing to close");
  //     amountOfSeconds += 100;
  //     pumpedMuscleCounter = 0;
  //     pumpedMuscle = false;
  //     return openGrip;
  //
  //     // if the signal is low and the timer went over, reset
  //     // everything. The timer here has to be set to the same as the
  //     // previous if statement, because everything happens at that time.
  //   } else if (bicepValue != HIGH && amountOfSeconds >= 1500) {
  //     amountOfSeconds = 0;
  //     pumpedMuscleCounter = 0;
  //
  //     // if the signal is high(there is a pump), and the time is
  //     // not over yet.
  //     // we increase the pumpedMuscleCounter by 1 to keep count.
  //     // we then change PumpedMuscle to true to show that we have pumped.
  //     // if it is pumped twice, we pushed the timer to 1500, so that we
  //     // can open the hand instantly.
  //   } else if (bicepValue == HIGH && amountOfSeconds <= 1600 && !pumpedMuscle) {
  //     pumpedMuscleCounter++;
  //     Serial.println("pumped " + pumpedMuscleCounter);
  //     pumpedMuscle = true;
  //     if (pumpedMuscleCounter == 2) {
  //       amountOfSeconds = 1500;
  //     } else {
  //       delay(100);
  //       amountOfSeconds+= 100;
  //     }
  //
  //     // if the signal is not high and the muscle hasn't triggered
  //     // the machine yet, return the currentGrip.
  //   } else if (bicepValue!=HIGH && pumpedMuscleCounter == 0){
  //     amountOfSeconds = 0;
  //     return currentGrip;
  //
  //     //reset the value of pumped muscle back to false on the
  //     // second round when it comes back around.
  //   } else if (bicepValue!= HIGH && pumpedMuscle) {
  //     pumpedMuscle = false;
  //
  //     // continue pushing the time.
  //   } else {
  //     delay(100);
  //     amountOfSeconds += 100;
  //   }
  //
  //   return currentGrip;
  // }

}



/** ###HARDWARE CONTROL### **/

/**
* func() openCloseActuator
*
* Open or close the actuator based on a boolean
* @parameter bool open: if gripOpen is true, open the hand
* @parameter int pressLength: We use pressLength to measure if it
*     has been 2 seconds. if val >= 2000, then its been 2 seconds.
**/
void openCloseActuator(bool gripOpen, int pressLength) {
  // if we receive a boolean that says, open is true
  // we move the actuator so that it opens.
  // else we close it.

  if (gripOpen) {


    if (pressLength >= 1600) {
      Serial.println("open");
      //writing onto the servo to open it (extend it)
      for (pos = 0; pos < 180; pos = pos + 1){
        myservo.write(pos);
        delay(5);
      }

    }
  } else {
    if (pressLength >= 1600) {

      Serial.println("close");
      //writing onto the servo to close it (retract it)
      for (pos = 180; pos > 1; pos = pos - 1) {
        myservo.write(pos);
        delay(5);
      }
    }

  }
}

int RMS(int emgValue) {
  total = total - emgArray[readIndex];
  emgArray[readIndex] = sq(emgValue)- 254;
  total = total + emgArray[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= 25) {
    readIndex = 0;
  }
  rmsValue = (sqrt(total/25));
  Serial.print(emgValue);
  Serial.print("\t");
  Serial.print(36);
  Serial.print("\t");
  Serial.println(rmsValue);
  
  delay(25);

  return rmsValue;
}

//Instantiate the class.
MuscleMotor* mm = new MuscleMotor(36, 0);

void loop() {
  // put your main code here, to run repeatedly:
  // Rule of thumb for optimization:
  // The code within this box should not be more than 8 lines

  getRMSSignal = RMS(analogRead(emg) - 340);

  bicepValue = digitalRead(bicepPin);
  // this will be used to change gripPosition
  gripOpen = mm->checkGripPosition(getRMSSignal);
  pressLength = mm->getAmountOfSeconds();
  if (pressLength > 0) {
//
//     Serial.print("pressLength =");
//     Serial.print(pressLength);
//     Serial.print(" gripOpen = ");
//     Serial.println(gripOpen);
  }
  openCloseActuator(gripOpen, pressLength);


}

