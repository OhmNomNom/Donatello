#include "Interpreter.h"
#include "Extruder.h"
#include "Base.h"

//Choose correct timer based on board
#if defined(__AVR_ATmega32U4__)
  #include "TimerThree.h"
  #define TIMER Timer3
#elif defined(__AVR_ATmega328P__)
  #include "TimerOne.h"
  #define TIMER Timer1
#endif
  
void setup() {
  Serial.begin(9600);
  
  initAxes();
  initInterpreter();
  initExtruder();
  
  TIMER.attachInterrupt(timerInterrupt, WORKER_PERIOD); //Run the interrupt every WORKER_PERIOD
}

void loop() {
  static ULONG lastPIDMicros = 0; //Last time we ran the heater PID thing
  const ULONG now = micros();
  
  while (Serial.available()) interpret(Serial.read()); //If we're getting commands, interpret them
  flushSerial(); //Flush any output put in the buffer
    
  if(isFlagSet(FLAG_HOTEND_ON) && (now - lastPIDMicros) >= PID_PERIOD) {
    lastPIDMicros = now;
    temperatureWorker(now); //Do the heater PID thing
  }
}

void timerInterrupt() {
  static ULONG prevNow = 0; //Last time we ran this function
  
  const ULONG now = micros();
  timeInterval = now - prevNow; //Store it for debugging
  prevNow = now;

  if(isFlagSet(FLAG_ENABLE)) stepperWorker(now); //Move the motors
    
}
