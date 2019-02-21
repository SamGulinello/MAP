#include "Print.h"
#include "Timer.h"

int led = 7;
int emg = A5;

int32_t emgAvg = 0;
int32_t *first;
int32_t *last;
int32_t *maxNum;
int32_t total;
const int EMG_ARRAY_LENGTH = 25;
int32_t emgArray[EMG_ARRAY_LENGTH] = {0};
const int TIME_OF_FLEX = 1000;
int readIndex;
int32_t maxValue;
int32_t rmsValue;
int32_t maxSignal;

int32_t rmsVal;
int isOpen = 1;

Monitor* Print = new Monitor();
Timer* Time = new Timer();

int32_t* maxElement(int32_t *first, int32_t *last){
  
  maxNum = first;
  
  while(++first != last){
    if(*first > *maxNum){
      maxNum = first;
    }
  }

  return maxNum;
}

void emgCal(){
  emgAvg = 0;
  
  for(int i = 0; i < 25; i++){
    emgAvg += analogRead(emg);
    delay(25);
  }

  emgAvg = emgAvg / 25;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);
  pinMode(emg, INPUT);

  Serial.begin(9600);
  *maxNum = 0;
  *first = 0;
  *last = 0;

  emgCal();
}

int32_t rms(int32_t emgValue) {
  //Updates array with new value from the emg
  total = total - emgArray[readIndex];
  emgArray[readIndex] = sq(emgValue);
  total = total + emgArray[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= emgArrayLength) {
    readIndex = 0;
  }

  //adds maximum value back in, updates it, and then removes it again
  total += maxValue;                     

  first = emgArray;
  last = emgArray+emgArrayLength;

  maxNum = maxElement(first, last);
  maxValue = *maxNum;

  total -= *maxNum;                       


  //calculates rms
  rmsValue = (sqrt(total/(emgArrayLength - 1)));

  //Print things to the monitor. Creates the plot
  Print->p(emgValue);
  Print->p(maxSignal);
  //Print->p(this->fsrReading);
  Print->pln(rmsValue);
  delay(25);

  return rmsValue;
}

int checkGripPosition(int32_t rmsVal){
  if(!Time->getTime()){
    Time->newTimer();
    
  } else if(Time->getTime() > TIME_OF_FLEX){
    isOpen = !isOpen;

    if(isOpen){
      openHand();
    } else{
      closeHand();
    }
    
    Time->newTimer();
    
  } else if(rmsVal < maxSignal){
    Time->newTimer();
  }


}

void openHand(){
  digitalWrite(led, LOW);
}

void closeHand(){
  digitalWrite(led, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  maxSignal = 25;

  rmsVal = rms(analogRead(emg) - emgAvg);

  checkGripPosition(rmsVal);
}
