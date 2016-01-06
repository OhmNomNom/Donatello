#include "Axes.h"
         
volatile Axis Axes[4];

void initAxes() {
  //Setup all axes
  for(int i = 0; i < 4; i++){
    pinMode(MOVPORT[i], OUTPUT);
    pinMode(DIRPORT[i], OUTPUT);
    digitalWrite(MOVPORT[i], HIGH);
    digitalWrite(DIRPORT[i], INVERT[i]);
    Axes[i].position = 0;
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
  
  if(distance < 0) distance = -distance; //Absolute value
  
  Axes[axis].steps      = distance * STEPSMILLI[axis];
  Axes[axis].stepTime   = round(1000000UL / (rate * STEPSMILLI[axis]));
  Axes[axis].lastMicros = 0;
  Axes[axis].direction  = ((distance < 0)?1:-1);
  
  setFlag(MOVFLAG[axis]);
  return true;
}

void stepperWorker(const ULONG now) {
  static bool XMov, YMov, ZMov, EMov;
    
  if(!(isFlagSet(FLAGS_AXES))) { //If no axis is moving, movement done
    stopStepperControl();
    return;
  }    
 
  XMov = (now - Axes[X].lastMicros) >= Axes[X].stepTime;
  YMov = (now - Axes[Y].lastMicros) >= Axes[Y].stepTime;
  ZMov = (now - Axes[Z].lastMicros) >= Axes[Z].stepTime;
  EMov = (now - Axes[E].lastMicros) >= Axes[E].stepTime;
  
  if(isFlagSet(MOVFLAG[X]) && Axes[X].steps && XMov) //If X moving & If there are steps left & it's time to move
    digitalWrite(MOVPORT[X], LOW); //Flip to move, we'll reset it to HIGH later (for synchronity)
  
  //Ditto
  if(isFlagSet(MOVFLAG[Y]) && Axes[Y].steps && YMov)
    digitalWrite(MOVPORT[Y], LOW);
  
  if(isFlagSet(MOVFLAG[Z]) && Axes[Z].steps && ZMov)
    digitalWrite(MOVPORT[Z], LOW);
  
  if(isFlagSet(MOVFLAG[E]) && Axes[E].steps && EMov)
    digitalWrite(MOVPORT[E], LOW);
  
  
  //Do the processing that we said we'll do later
  if(isFlagSet(MOVFLAG[X])) {
    if(Axes[X].steps) {
      if(XMov) {
        digitalWrite(MOVPORT[X], HIGH); //Prepare for next movement
        Axes[X].steps--; //We moved 1 step!!
        Axes[X].lastMicros += Axes[X].stepTime; //Last time we moved
        Axes[X].position += Axes[X].direction; //Either +1 or -1, depending on how we're moving
      }
    } else unsetFlag(MOVFLAG[X]); //We finished our movement
  }
  
  //Ditto
  if(isFlagSet(MOVFLAG[Y])) {
    if(Axes[Y].steps) {
      if(YMov) {
        digitalWrite(MOVPORT[Y], HIGH);
        Axes[Y].steps--;
        Axes[Y].lastMicros += Axes[Y].stepTime;
        Axes[Y].position += Axes[Y].direction;
      }
    } else unsetFlag(MOVFLAG[Y]);
  }
  if(isFlagSet(MOVFLAG[Z])) {
    if(Axes[Z].steps) {
      if(ZMov) {
        digitalWrite(MOVPORT[Z], HIGH);
        Axes[Z].steps--;
        Axes[Z].lastMicros += Axes[Z].stepTime;
        Axes[Z].position += Axes[Z].direction;
      }
    } else unsetFlag(MOVFLAG[Z]);
  }
  if(isFlagSet(MOVFLAG[E])) {
    if(Axes[E].steps) {
      if(EMov) {
        digitalWrite(MOVPORT[E], HIGH);
        Axes[E].steps--;
        Axes[E].lastMicros += Axes[E].stepTime;
        Axes[E].position += Axes[E].direction;
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
