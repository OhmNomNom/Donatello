#ifndef Axes_h_
#define Axes_h_

#include "Base.h"
#include "Interpreter.h"

//Arduino ports that move/control the direction
static const UBYTE X_MOVPORT   = 9,
                   Y_MOVPORT   = 6,
                   Z_MOVPORT   = 4,
                   E_MOVPORT   = 16,
                  
                   X_DIRPORT   = 8,
                   Y_DIRPORT   = 7,
                   Z_DIRPORT   = 5,
                   E_DIRPORT   = 14;
                  
//Steps per mm for various axes
static const UINT  X_STEPSMILLI = 315,
                   Y_STEPSMILLI = 315,
                   Z_STEPSMILLI = 630,
                   E_STEPSMILLI = 97;
                  
//Invert the +/- direction for a specific axis
static const bool  X_INVERT    = false,
                   Y_INVERT    = false,
                   Z_INVERT    = false,
                   E_INVERT    = false;
                  
//Max speed for axis, in mm/sec
static const float X_MAXSPEED  = 20.0,
                   Y_MAXSPEED  = 20.0,
                   Z_MAXSPEED  = 20.0,
                   E_MAXSPEED  = 2.0,
                   MAXSPEED_LINEAR = 20.0;


//Putting everything neatly in an array
static const UBYTE MOVFLAG  []   = {FLAG_X,       FLAG_Y,       FLAG_Z,       FLAG_E    },
                   MOVPORT  []   = {X_MOVPORT,    Y_MOVPORT,    Z_MOVPORT,    E_MOVPORT },
                   DIRPORT  []   = {X_DIRPORT,    Y_DIRPORT,    Z_DIRPORT,    E_DIRPORT };
static const bool  INVERT   []   = {X_INVERT,     Y_INVERT,     Z_INVERT,     E_INVERT  };

static const UINT  STEPSMILLI [] = {X_STEPSMILLI, Y_STEPSMILLI, Z_STEPSMILLI, E_STEPSMILLI};

static const float MAXSPEED   [] = {X_MAXSPEED,        Y_MAXSPEED,        Z_MAXSPEED,        E_MAXSPEED       },
                   STEPLENGTH [] = {1.0f/X_STEPSMILLI, 1.0f/Y_STEPSMILLI, 1.0f/Z_STEPSMILLI, 1.0f/E_STEPSMILLI}; //Length of step, in mm
                  

struct Axis {
  ULONG stepTime;   //Time for each step
  ULONG steps;      //# of steps remaining in motion
  SLONG lastMicros; //Last time we moved
};

extern volatile Axis Axes[];
extern volatile SLONG axisPosition[];

void initAxes();
void resetAxes();
bool moveAxis(ParamIndex,float,float);
void stepperWorker(const ULONG);
void startStepperControl();
void stopStepperControl();

#endif
