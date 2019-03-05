/**
 * Print Class
 * 
 * This makes printing to Serial easier for the programmer
 * It also converts the output numbers into units of mV
 */
#include "Arduino.h"

class Monitor {
  private:
    double factor;
    int precision;
    
  public:
    Monitor();
    void p(int);
    void pln(int);
};

/**
 * Monitor class constructor
 */
Monitor::Monitor(){
  this->factor = 4.88759;
  this->precision = 0;
}

/**
 * This function prints numbers in mV without creating
 * a new line
 */
void Monitor::p(int in){
  Serial.print(in * factor, precision);
  Serial.print("\t");
}

/**
 * This function prints numbers in mV and creates a 
 * new line
 */
void Monitor::pln(int in){
  Serial.println(in * factor, precision);
}
