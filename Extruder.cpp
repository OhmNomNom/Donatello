#include "Extruder.h"

float activeTemperature = 200.0f,
      idleTemperature = 140.0f,
      temperatureTolerance = 5.0f;

void initExtruder() {
  pinMode(HEATER_PORT,OUTPUT);
  pinMode(THERMISTOR_PORT,INPUT);
  analogWrite(HEATER_PORT,0);
}

float getExtruderTemperature() {
  float R,T;

  R = float(analogRead(THERMISTOR_PORT)); // the value
  R = (1024.0f - R) / R ; //=Rtherm/Rbalance
  R = R_BALANCE * R;

  T  = 1.0f / ( (1.0f/EXT_T0) + (1.0f/EXT_BETA)*log(R/EXT_R0) ); //Beta factor method
  T -= ZERO_C; //T in celsius

  return T;
}

void temperatureWorker(const ULONG now) { 
  const int &targetTemperature = (stateFlags & FLAG_ACTIVE_TEMP) ? activeTemperature : idleTemperature; //Which temperature to aim to
  
  if(getExtruderTemperature() >= EXTRUDER_MAX_TEMP) { //FAIL!
    analogWrite(HEATER_PORT,0);
  }
  
  //PID
  static float iTerm = 0,
               prevError = 0;
  
  const float currentTemperature = getExtruderTemperature();
  
  if(!isFlagSet(FLAG_HOTEND_ON)) return;
  

  const float error = targetTemperature - currentTemperature;  

  const float pTerm = KP * error;
  
  iTerm *= KI2;//Decaying integral if KI2 < 1
  iTerm += KI * error;
  
  const float dTerm = KD * (error - prevError);
  
  prevError = error;
  
  //limit it...
  if(iTerm > MAX_OUTPUT) iTerm = MAX_OUTPUT;
  else if(iTerm < 0)     iTerm = 0; //TODO -Bias
  
  int output = pTerm + iTerm + dTerm;
  
  //limit it...
  if(output > MAX_OUTPUT) output = MAX_OUTPUT;
  else if(output + BIAS < 0) output = -BIAS;
  
  analogWrite(HEATER_PORT,BIAS + output);
}

void startTemperatureControl(bool isActive) {
  setFlag(FLAG_HOTEND_ON);
  if(isActive) setFlag(FLAG_ACTIVE_TEMP);
  else unsetFlag(FLAG_ACTIVE_TEMP);
}

void stopTemperatureControl() {
  unsetFlag(FLAG_HOTEND_ON);
  analogWrite(HEATER_PORT,0);
}
