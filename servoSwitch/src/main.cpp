#include <Arduino.h>
#include <PWMServo.h>
#include <RCSwitch.h>

#define pinServo 10        // orange = signal, red = vcc, brown = gnd
int pos = 0;

PWMServo myServo;
// RCSwitch mySwitch = RCSwitch();

void setup(){
  // OCR0A = 0xAF;            // any number is OK
  // TIMSK |= _BV(OCIE0A);    // Turn on the compare interrupt (below!)

  Serial.begin(9600);
  myServo.attach(pinServo);
  // mySwitch.enableReceive(0);
}

void loop(){

  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}
