/* Timer Class */

#include "Arduino.h"

class Timer {
  private:
    unsigned long beginTime;
    unsigned long prevRead;
    int allowGetTime;

  public:
    Timer();
    unsigned long getTotalTime();
    void resetTimer();
    unsigned long getTime();
};

Timer::Timer(){
  this->beginTime = millis();
  this->prevRead = 0;
  this->allowGetTime = 0;
}

unsigned long Timer::getTotalTime(){
  return millis() - beginTime;
}

unsigned long Timer::getTime(){
  if(allowGetTime){
    return millis() - prevRead; 
  } else {
    return 0;
  }
}

void Timer::resetTimer(){
  prevRead = millis();
  allowGetTime = 1;
}
