#include "Axes.h"
         
volatile Axis Axes[4];
volatile SLONG axisPosition[4];
SBYTE axisDirection[4]; //-1 for reverse, 1 for fwd

void initAxes() {
  //Setup all axes
  for(int i = 0; i < 4; i++){
    pinMode(MOVPORT[i], OUTPUT);
    pinMode(DIRPORT[i], OUTPUT);
    digitalWrite(MOVPORT[i], HIGH);
    digitalWrite(DIRPORT[i], INVERT[i]);
    axisPosition[i] = 0;
  }
  resetAxes();
}

void resetAxes() {
  //What it says on the tin..
  Axes[X].steps = 0;
  Axes[Y].steps = 0;
  Axes[Z].steps = 0;
  Axes[E].steps = 0;
  unsetFlag(FLAGS_AXES);
}

bool moveAxis(ParamIndex axis, float distance, float rate) {
  
  if(isFlagSet(MOVFLAG[axis]) || (rate > MAXSPEED[axis])) return false; //if you're already moving, or you're asking sth outside the limits, fail.
  if(distance == 0) return true;

  //Set the correct direction
  digitalWrite(DIRPORT[axis], ((distance < 0) xor (INVERT[axis])));
  axisDirection[axis] = ((distance < 0)?1:-1);
  
  if(distance < 0) distance = -distance; //Absolute value
  
  Axes[axis].steps      = distance * STEPSMILLI[axis];
  Axes[axis].stepTime   = round(1000000UL / (rate * STEPSMILLI[axis]));
  Axes[axis].lastMicros = 0;
  
  setFlag(MOVFLAG[axis]);
  return true;
}

void stepperWorker(const ULONG now) {
  
  if(!(isFlagSet(FLAGS_AXES))) { //If no axis is moving, movement done
    stopStepperControl();
    return;
  }    
 
  if(isFlagSet(MOVFLAG[X])) { //If X moving
    if(Axes[X].steps) { //If there are steps left
      if(now - Axes[X].lastMicros >= Axes[X].stepTime) { //If it's time to move
        digitalWrite(MOVPORT[X], LOW);
        digitalWrite(MOVPORT[X], HIGH); //double-flip it
        Axes[X].steps--; //One less step to go
        Axes[X].lastMicros = now;
        axisPosition[X] += axisDirection[X]; //Either +1 or -1
      }
    } else unsetFlag(MOVFLAG[X]); //Done movement
  }
  
  //Ditto
  if(isFlagSet(MOVFLAG[Y])) {
    if(Axes[Y].steps) {
      if(now - Axes[Y].lastMicros >= Axes[Y].stepTime) {
        digitalWrite(MOVPORT[Y], LOW);
        digitalWrite(MOVPORT[Y], HIGH);
        Axes[Y].steps--;
        Axes[Y].lastMicros = now;
        axisPosition[Y] += axisDirection[Y];
      }
    } else unsetFlag(MOVFLAG[Y]);
  }
  
  if(isFlagSet(MOVFLAG[Z])) {
    if(Axes[Z].steps) {
      if(now - Axes[Z].lastMicros >= Axes[Z].stepTime) {
        digitalWrite(MOVPORT[Z], LOW);
        digitalWrite(MOVPORT[Z], HIGH);
        Axes[Z].steps--;
        Axes[Z].lastMicros = now;
        axisPosition[Z] += axisDirection[Z];
      }
    } else unsetFlag(MOVFLAG[Z]);
  }
  
  if(isFlagSet(MOVFLAG[E])) {
    if(Axes[E].steps) {
      if(now - Axes[E].lastMicros >= Axes[E].stepTime) {
        digitalWrite(MOVPORT[E], LOW);
        digitalWrite(MOVPORT[E], HIGH);
        Axes[E].steps--;
        Axes[E].lastMicros = now;
        axisPosition[E] += axisDirection[E];
      }
    } else unsetFlag(MOVFLAG[E]);
  }
}

void startStepperControl() {
  setFlag(FLAG_ENABLE);
}

void stopStepperControl() {
  unsetFlag(FLAG_ENABLE);
  doneMoving();
}
