/* Printer Class to make printing easy */

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

Monitor::Monitor(){
  this->factor = 4.88759;
  this->precision = 0;
}

void Monitor::p(int in){
  Serial.print(in * factor, precision);
  Serial.print("\t");
}

void Monitor::pln(int in){
  Serial.println(in * factor, precision);
}
