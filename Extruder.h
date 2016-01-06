#ifndef Extruder_h_
#define Extruder_h_

#include "Base.h"
#include <math.h>

const float EXT_BETA  = 3950,           //From thermistor datasheet
            EXT_T0    = 298.15,         //Same, 25* C
            EXT_R0    = 100000,         //From thermistor datasheet
            
            ZERO_C    = 273.15,         //0* C in K
            EXTRUDER_MAX_TEMP = 250.0f, //Max temperature allowed, C
            
            //PID Params
            KP        = 8  ,
            KI        = 5  ,
            KI2       = 0.9,
            KD        = 60 ;
            
const UBYTE THERMISTOR_PORT = 10,
            HEATER_PORT     = A1;
            
const int   R_BALANCE       = 10000,        //Ohm, TODO adjust
            BIAS            = 45,           //Bias...
            MAX_OUTPUT      = 255 - BIAS;
            
            
extern float activeTemperature,
             idleTemperature,
             temperatureTolerance;      //Unused now
            
void initExtruder();
float getExtruderTemperature();
void temperatureWorker(const ULONG);
void startTemperatureControl(bool);
void stopTemperatureControl();

#endif
