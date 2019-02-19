//Print Function

#include "Arduino.h"

static double factor = 4.88759;
static int precision = 0;

void Println(int32_t in){
  Serial.println(in * factor, precision);
}
