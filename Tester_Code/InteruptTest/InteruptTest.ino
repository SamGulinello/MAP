    /*
    Copyright 2011 Lex.V.Talionis at gmail
    This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    */
    #include <PinChangeInt.h>
    #include <PinChangeIntConfig.h>
     
    #define PIN 15  // the pin we are interested in
    volatile byte burp=0;    // a counter to see how many times the pin has changed
    byte cmd=0;     // a place to put our serial data
     
    void setup() {
      Serial.begin(9600);
      Serial.print("PinChangeInt test on pin ");
      Serial.print(PIN);
      Serial.println();
      pinMode(PIN, INPUT);     //set the pin to input
      digitalWrite(PIN, HIGH); //use the internal pullup resistor
      PCintPort::attachInterrupt(PIN, burpcount,RISING); // attach a PinChange Interrupt to our pin on the rising edge
    // (RISING, FALLING and CHANGE all work with this library)
    // and execute the function burpcount when that pin changes
      }
     
    void loop() {
      cmd=Serial.read();  
      if (cmd=='p')
      {
        Serial.print("burpcount:\t");
        Serial.println(burp, DEC);
      }
      cmd=0;
    }
     
    void burpcount()
    {
      burp++;
    }
