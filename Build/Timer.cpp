/**
 * Timer Class
 * 
 * This Class keeps track of elapsed time and time differentials
 * 
 * Note: Will not work if arduino runs for >50 days without restart
 */

// Include required to use millis() function
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

/**
 * Class Constructor
 * 
 * Starts timer upon call
 */
Timer::Timer(){
  this->beginTime = millis();
  this->prevRead = 0;
  this->allowGetTime = 0;
}

/**
 * Returns difference between current time and
 * start time of the timer
 */
unsigned long Timer::getTotalTime(){
  return millis() - beginTime;
}

/**
 * Returns difference between current time and
 * most recent timer reset
 * 
 * Returns 0 if there is no previous timer reset
 */
unsigned long Timer::getTime(){
  if(allowGetTime){
    return millis() - prevRead; 
  } else {
    return 0;
  }
}

/**
 * This creates a "checkpoint" time. Subsequent calls
 * to getTime() will return the difference between the
 * time of the getTime() call and the "checkpoint" time
 */
void Timer::resetTimer(){
  prevRead = millis();
  allowGetTime = 1;
}
