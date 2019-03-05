/**
* @author Keith Lim, Sam Gullinello, Keller Martin, Jared Butler
* The Main file for working the MAP
* Consists of the latest working code as of 3/4/19
**/

/**
 * -------------------Future Changes-------------------
 * LOOK FOR A COMMENT LIKE ^^^^^ TO FIND NECESSARY CHANGES
 * 
 * -------------------Future Changes-------------------
 * Minimize global variables
 */

#include <Servo.h>
#include "Print.h"
#include "Timer.h"

Servo myservo;
Servo myservo2;

// Constants
const int INITIAL_STATE = 0;
extern const int EMG_ARRAY_LENGTH = 25;
const int TIME_OF_FLEX = 700;
const int FSR_THRESHOLD = 600;

// Variables for rms
int32_t emgArray[EMG_ARRAY_LENGTH] = {0};
int readIndex;
int32_t rmsValue;
int32_t total;
int32_t *maxNum;
int32_t *first;
int32_t *last;
int32_t maxValue = 0;

// Variables for Loop
int32_t getRMSSignal = 0;
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
int led = 7;  //---------------------> Testing LED
int servo = 10;
int servo2 = 11;

//-------------------Below Are functions not in mm class-----------------------//

/**
 * This calculates the max element in an array, given a pointer to the start 
 * of the array and a pointer to the end of the same array. This is just a 
 * simple max finder function.
 */
int32_t* maxElement(int32_t * first, int32_t * last){
  
  maxNum = first;
  
  while(++first != last){
    if(*first > *maxNum){
      maxNum = first;
    }
  }

  return maxNum;
}


//--------------------------Start of mm Class----------------------------------//
/**
* Muscle Motor class keeps track of the state of the motors
* and provides functions which control the motors. For example, 
* checkGripPosition() takes in the most recent rms signal and 
* decides whether or not the hand should be opened or closed.
*
* ##grip is always open when false and closed when true
* ^^^^^check if this is true
* 
* -------------------Future Changes-------------------
* Move Emg related functions and data into separate class
* 
* openHand() and closeHand() may need to be made public later
* 
**/
class MuscleMotor {
  private:
    // Variables concerning state of motors/apparatus
    bool currentGrip;
    int16_t maxSignal;
    int16_t fsrReading;
    int servoPos;
  
    // Functions concerning state of motors/apparatus
    void openHand();
    void closeHand();
    
  
  public:
    // Constructor(s)
    MuscleMotor();   

    // Functions which make decisions or control states
    void checkGripPosition(int32_t);
    int32_t  rms(int32_t);
    void indicateBatteryLevel();
    void emgCal(); 

    // Getters and Setters
    void setMaxSignal(int32_t);
    void setFsrReading(int32_t);    
};

//---------------------Instantiate necessary classes---------------------//

MuscleMotor* mm = new MuscleMotor();
Monitor* Print = new Monitor();
Timer* Time = new Timer();

/**
 * MuscleMotor class constructor
 * 
 * Sets initial values
 */
MuscleMotor::MuscleMotor()
{
  this->currentGrip = true;
  this->servoPos = 0;
}

/**
 * Sets hand-trigger theshold level
 */
void MuscleMotor::setMaxSignal(int32_t maxSignal)
{
  this->maxSignal = maxSignal;
}

/**
 * Sets fsr level
 */
void MuscleMotor::setFsrReading(int32_t fsrReading){
  this->fsrReading = fsrReading;
}

/**
 * Main decision making function. This keeps track of how long
 * the muscles are being flexed for, and changes the state of
 * the motors if necessary.
 */
void MuscleMotor::checkGripPosition(int32_t rmsVal){

  // If Timer hasn't been initialized yet, getTime() returns 0
  // This if statement only executes once and never again
  if(!Time->getTime()){

    // This sets the Timer to 0 on first run-through
    Time->resetTimer();

  // Check to see if muscle has been flexed longer than specified time
  // If so, this toggles the state of the motors and sets Timer to 0
  } else if(Time->getTime() > TIME_OF_FLEX){

    // Toggle motor state
    currentGrip = !currentGrip;

    // Move motors based upon new state
    if(currentGrip){
      closeHand();
    } else{
      openHand();
    }

    // Set Timer to 0
    Time->resetTimer();

  // Check to see if muscle has been relaxed below threshold
  // If so, reset Timer to 0
  } else if(rmsVal < maxSignal){

    Time->resetTimer();
  }

  /**
   * If none of the if statements are true, this function does nothing.
   * Next time it is called, the state may be different, so it may then
   * perform an action. If the function does nothing, the Timer continues
   * counting.
   */
  
}

/**
 * This opens the hand when called.
 * It also turns on an LED for testing purposes
 * 
 * -------------------Future Changes-------------------
 * Remove testing LED
 */
void MuscleMotor::openHand(){

  digitalWrite(led, HIGH);
  for (; mm->servoPos > 1; mm->servoPos--) {
    myservo2.write(mm->servoPos);
    myservo.write(mm->servoPos);
    delay(5);
    
  }
}

/**
 * This closes the hand when called
 * It also turns off an LED for testing purposes
 * 
 * If the value of the fsr is larger than a set threshold
 * immediately stop closing the hand. 
 * 
 * -------------------Future Changes-------------------
 * This is part of the MuscleMotor class "mm->" notation
 * is redundant. Remove it
 * 
 * Remove testing LED
 */
void MuscleMotor::closeHand(){
  
  digitalWrite(led, LOW);
  for (; mm->servoPos < 180; mm->servoPos++){
    myservo2.write(mm->servoPos);
    myservo.write(mm->servoPos);
    
    mm->setFsrReading(analogRead(fsr));
    delay(5);
    
    if(fsrReading > FSR_THRESHOLD){
      break;
    }
  } 
}


/**
* Calculates Root Mean Square (RMS) of "n" most recent readings (number specified in EMG_ARRAY_LENGTH) 
* when called, including the newest emgValue reading. 
* 
* Removes the maximum value before calculation. This is to reduce the effect of noise.
**/
int32_t MuscleMotor::rms(int32_t emgValue) {
  
  // Updates array with square of new value from the emg
  // Updates total to include this new value
  total = total - emgArray[readIndex];
  emgArray[readIndex] = sq(emgValue);
  total = total + emgArray[readIndex];

  // Increments readIndex. Sets to 0 if end of emgArray is reached
  readIndex = readIndex + 1;
  if (readIndex >= EMG_ARRAY_LENGTH) {
    readIndex = 0;
  }

  // Adds maximum value back in to total
  total += maxValue;                     

  // Updates maximum value based on new emgArray value
  first = emgArray;
  last = emgArray + EMG_ARRAY_LENGTH;

  maxNum = maxElement(first, last);
  maxValue = *maxNum;

  // Removees maximum value from total
  total -= *maxNum;                       


  // Calculates rms
  rmsValue = (sqrt(total/(EMG_ARRAY_LENGTH - 1)));

  // Print things to the monitor. Creates the plot
  Print->p(emgValue);
  Print->p(this->maxSignal);
  //Print->p(this->fsrReading);
  Print->pln(rmsValue);

  return rmsValue;
}


/**
*  Light RGB LED to different colors to signal the battery level.
*  
*  -------------------Future Changes-------------------
*  This needs updating to prevent possible flickering
**/

void MuscleMotor::indicateBatteryLevel() {

  // Read battery levels
  int bat2Level = analogRead(BatteryLevelReadBat2);
  int bat1Level = analogRead(BatteryLevelReadBoth);

  /**-------------------Future Changes-------------------
   * Make thresholds constants
   */
  int greenThreshold = 818; // 4V*1023/5V
  int redThreshold = 655; // 3.2V*1023/5V

  // What does this do?
  digitalWrite(BatteryLevelLEDB, LOW);

  // This is be true if Battery is charged within working capacity
  if((bat2Level > greenThreshold)/* && (bat1Level > greenThreshold)*/) {
    digitalWrite(BatteryLevelLEDR, LOW);
    digitalWrite(BatteryLevelLEDB, LOW);
    digitalWrite(BatteryLevelLEDG, HIGH);
  }

  // This is true if battery is beginning to run low
  else if((bat2Level > redThreshold)/* && (bat1Level > redThreshold)*/) {
    digitalWrite(BatteryLevelLEDR, LOW);
    digitalWrite(BatteryLevelLEDG, LOW);
    digitalWrite(BatteryLevelLEDB, HIGH);
  }

  // This is true if battery is too low to run the hand
  else if((bat2Level <= redThreshold)/* || (bat1Level <= redThreshold)*/){
    digitalWrite(BatteryLevelLEDR, HIGH);
    digitalWrite(BatteryLevelLEDG, LOW);
    digitalWrite(BatteryLevelLEDB, LOW);
  }
}

/**
 * Calibrates emgs so that the average emg reading may be set to 0
 * This improves accuracy of the rms function
 */
void MuscleMotor::emgCal(){
  emgAvg = 0;
  
  for(int i = 0; i < 25; i++){
    emgAvg += analogRead(emg);
    delay(25);
  }

  // Finds average of 25 initial readings to calibrate
  emgAvg = emgAvg / 25;
}

/**
 * This runs once upon startup
 */
void setup() {

  // Initialize servos
  myservo.attach(servo);
  myservo2.attach(servo2);
  myservo.write(INITIAL_STATE);
  myservo2.write(INITIAL_STATE);

  // Initialize pins
  pinMode(thresholdPot, INPUT);
  pinMode(emg, INPUT);
  pinMode(fsr, INPUT);
  pinMode(BatteryLevelReadBoth, INPUT);
  pinMode(BatteryLevelReadBat2, INPUT);
  pinMode(led, OUTPUT);
  pinMode(BatteryLevelLEDR, OUTPUT);
  pinMode(BatteryLevelLEDG, OUTPUT);
  pinMode(BatteryLevelLEDB, OUTPUT);

  // Initialize Serial monitor/plotter
  Serial.begin(9600);

  /**-------------------Future Changes-------------------
   * Are these variables necessary to be set here?
   */
  *maxNum = 0;
  *first = 0;
  *last = 0;

  // Calibrate emgs upon startup
  mm->emgCal();
}

/**
 * This code runs repeatedly until shut off.
 * Try to minimize amount of operations performed in the loop
 */
void loop() {

  // Find emg value
  emgRead = analogRead(emg);

  // Update Battery indicators
  mm->indicateBatteryLevel();

  /**-------------------Future Changes-------------------
   * This fsr update may not be necessary. It is also updated
   * inside of the closeHand() function.
   */
  // Set fsrReading variable
  mm->setFsrReading(analogRead(fsr));

  // Calculate rms value
  //getRMSSignal = mm->rms(analogRead(emg) - 334);
  getRMSSignal = mm->rms(emgRead - emgAvg);

  // Setting variable threshold
  //mm->setMaxSignal(analogRead(thresholdPot));
  mm->setMaxSignal(25);

  // Decide if grip state should change and if so, change it
  mm->checkGripPosition(getRMSSignal);

  // This delay is important to keep the amount of loop executions smaller
  // This is important to save battery. 
  delay(25);
}
