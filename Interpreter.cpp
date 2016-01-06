#include "Interpreter.h"

UINT cmdLine,
     lineCounter,
     movementLine;
     
Command command;
UBYTE bufferPosition;
CommandStates cmdState;

char cmdBuffer[CMDBUFFER_SIZE+1],
     checksum;
     
float cmdParams[7],
      feedRate = MAXSPEED_LINEAR / 2,
      extrudeRate;

void initInterpreter() {
  clearPrevCommand();
  lineCounter = 1;
  feedRate = MAXSPEED_LINEAR / 2;
  extrudeRate = MAXSPEED[E] / 4;
}

void interpret(char c) {
  if(c == '\n') {
    execCommand();
    clearPrevCommand();
    return;
  }
  
  if(cmdState == STATE_INVALID) return;
  if(cmdState != STATE_CHECKSUM) checksum ^= c;
  
  switch(c) {
  case '*':
    checksum ^= '*'; //undo the xor done above
    cmdState = STATE_CHECKSUM;
    bufferPosition = 0;
    break;
  case ' ': 
    process();
    break;
  default:
    cmdBuffer[bufferPosition++] = c;
    if(bufferPosition >= CMDBUFFER_SIZE)
      cmdState = STATE_INVALID;
  }
}

void process() {
  cmdBuffer[bufferPosition] = '\0'; //Terminate the cstring
reparse:
  switch(cmdState) {
  case STATE_LINENUM:
    if(cmdBuffer[0] != 'N') { //We don't have a line number
      cmdLine = lineCounter;
      cmdState = STATE_COMMAND;
      goto reparse;
    }
    if(bufferPosition < 2) { //1 or no characters, invalid line #
      cmdState = STATE_INVALID;
      break;
    }
    cmdLine = sParseUINT(cmdBuffer+1); //Get the line number
    if(cmdLine < lineCounter) { //Can't go back
      cmdLine = lineCounter;
      cmdState = STATE_INVALID;
      break;
    }
    cmdState = STATE_COMMAND;
    break;
  case STATE_COMMAND:
    if(isFlagSet(FLAG_ENABLE | FLAGS_AXES) && cmdBuffer[0] == 'G') { //Can't execute a G code while a movement is in progress
      cmdState = STATE_INVALID;
      break;
    } 
    //Start checking every G code, probably there's a better way to do this but this isn't really performance critical..
    else if (strEqual(cmdBuffer,"M00"))
      command = CMD_HALT;
    else if (strEqual(cmdBuffer,"M70"))
      command = CMD_FLAGS;
    else if (strEqual(cmdBuffer,"G00"))
      command = CMD_RPOS;
    else if (strEqual(cmdBuffer,"G01"))
      command = CMD_LINEAR;
    else if (strEqual(cmdBuffer,"G90"))
      command = CMD_MODEABS;
    else if (strEqual(cmdBuffer,"G91"))
      command = CMD_MODEINC;
    else if (strEqual(cmdBuffer,"M77"))
      command = CMD_ECHO;
    else if (strEqual(cmdBuffer,"M97"))
      command = CMD_RESUME;
    else if (strEqual(cmdBuffer,"M96"))
      command = CMD_RECOVER;
    else if (strEqual(cmdBuffer,"M114"))
      command = CMD_GETPOS;
    else if (strEqual(cmdBuffer,"M105"))
      command = CMD_GETTEMP;
    else if (strEqual(cmdBuffer,"M03"))
      command = CMD_HOTEND_PASSIVE;
    else if (strEqual(cmdBuffer,"M04"))
      command = CMD_HOTEND_ACTIVE;
    else if (strEqual(cmdBuffer,"M05"))
      command = CMD_HOTEND_OFF;
    else if (strEqual(cmdBuffer,"M104"))
      command = CMD_SETTEMP;
    else if (strEqual(cmdBuffer,"G92"))
      command = CMD_SETPOS;
    else if (strEqual(cmdBuffer,"M71"))
      command = CMD_GETTIME;
    else if (strEqual(cmdBuffer,"M74"))
      command = CMD_GETSTEPS;
    else if (strEqual(cmdBuffer,"M301"))
      command = CMD_SETPID;
    else if (strEqual(cmdBuffer,"M136"))
      command = CMD_GETPID;
    else {
      cmdState = STATE_INVALID;
      break;
    }
    
    cmdState = STATE_PARAMS; 
    cmdParams[X] = cmdParams[Y] = cmdParams[Z] = cmdParams[E] = cmdParams[F] = cmdParams[S] = cmdParams[R] = NAN; //Reset all parameters
    
    break;
  case STATE_PARAMS:
    if(bufferPosition < 2) { //Should have 2 characters or more.
      cmdState = STATE_INVALID;
      break;
    }
    float input;
    input = sParseFloat(cmdBuffer+1);
    if (isnan(input)) {
      cmdState = STATE_INVALID;
      break;
    }
    switch(cmdBuffer[0]) {
      case 'X':
      case 'P':
        cmdParams[X] = input;
        break;
      case 'Y':
      case 'I':
        cmdParams[Y] = input;
        break;
      case 'Z':
      case 'D':
        cmdParams[Z] = input;
        break;
      case 'E':
        cmdParams[E] = input;
        break;
      case 'F':
        cmdParams[F] = input;
        break;
      case 'S':
        cmdParams[S] = input;
        break;
      case 'R':
        cmdParams[R] = input;
        break;
      default:
        cmdState = STATE_INVALID;
        break;
    }
    break;
  case STATE_CHECKSUM:
    UINT check = sParseUINT(cmdBuffer);
    if(checksum != check) cmdState = STATE_INVALID;
    break;
  }
  bufferPosition = 0;
}

void execCommand() {
  if(bufferPosition != 0) process();
  if(cmdState == STATE_INVALID) {
    invalidCommand();
    return;
  }
  
  noInterrupts(); //Turn off interrupts (movement) while processing the things...
                  //Probably unnecessary, TODO: CHECK!
  switch(command) {
  case CMD_NONE:
    cmdNone();
    break;
  case CMD_FLAGS:
    cmdGetFlags();
    break;
  case CMD_RPOS:
    cmdRapidPositioning();
    break;
  case CMD_LINEAR:
    cmdLinearInterpolation();
    break;
  case CMD_ECHO:
    cmdEcho();
    break;
  case CMD_HALT:
    cmdHalt();
    break;
  case CMD_RESUME:
    cmdResume();
    break;
  case CMD_RECOVER:
    cmdRecover();
    break;
  case CMD_GETPOS:
    cmdGetPosition();
    break;
  case CMD_MODEABS:
    cmdAbsoluteMode();
    break;
  case CMD_MODEINC:
    cmdIncrementalMode();
    break;
  case CMD_SETTEMP:
    cmdSetTemperature();
    break;
  case CMD_SETPOS:
    cmdSetPos();
    break;
  case CMD_GETTIME:
    cmdGetTime();
    break;
  case CMD_GETTEMP:
    cmdGetTemp();
    break;
  case CMD_HOTEND_PASSIVE:
    cmdHotendPassive();
    break;
  case CMD_HOTEND_ACTIVE:
    cmdHotendActive();
    break;
  case CMD_HOTEND_OFF:
    cmdHotendOff();
    break;
  case CMD_GETSTEPS:
    cmdGetSteps();
    break;
  case CMD_SETPID:
    cmdSetPID();
    break;
  case CMD_GETPID:
    cmdGetPID();
    break;
  }
  interrupts();//Reinstate interrupts
  
  lineCounter = cmdLine + 1;
}

inline void clearPrevCommand() { 
    command = CMD_NONE;
    cmdState = STATE_LINENUM;
    bufferPosition = 0;
    checksum = 0;
    cmdParams[X] = 0;
    cmdParams[Y] = 0;
    cmdParams[Z] = 0;
    cmdParams[E] = 0;
    cmdParams[F] = 0;
    cmdParams[S] = 0;
    cmdParams[R] = 0;
}

void doneMoving() {
  addToBufferS("DONE N",6);
  addToBufferI(movementLine);
  addToBufferC('\n');
}

inline void invalidCommand() {
  addToBufferS("INVALID N",9);
  addToBufferI(cmdLine);
  addToBufferC('\n');
  lineCounter = cmdLine;
}

inline void acknowledgeCommand() {
  addToBufferS("ACK N",5);
  addToBufferI(cmdLine);
  addToBufferC('\n');
  flushSerial();
}

void cmdRapidPositioning() { //G00
  float motion[4];
  
  if(isFlagSet(FLAG_ABSOLUTE_MODE)) {
    motion[X] = isnan(cmdParams[X])?0:(cmdParams[X] - Axes[X].position*STEPLENGTH[X]);
    motion[Y] = isnan(cmdParams[Y])?0:(cmdParams[Y] - Axes[Y].position*STEPLENGTH[Y]);
    motion[Z] = isnan(cmdParams[Z])?0:(cmdParams[Z] - Axes[Z].position*STEPLENGTH[Z]);
  } else {
    motion[X] = isnan(cmdParams[X])?0:cmdParams[X];
    motion[Y] = isnan(cmdParams[Y])?0:cmdParams[Y];
    motion[Z] = isnan(cmdParams[Z])?0:cmdParams[Z];
  }
  motion[E] = cmdParams[E];
  
  float feedRate = NAN;
  if(cmdParams[F] > 0) feedRate = cmdParams[F];
  
  bool fail = false;
  if(!isnan(cmdParams[X])) if(!moveAxis(X,motion[X],isnan(feedRate)?MAXSPEED[X]:feedRate)) fail = true;
  if(!isnan(cmdParams[Y])) if(!moveAxis(Y,motion[Y],isnan(feedRate)?MAXSPEED[Y]:feedRate)) fail = true;
  if(!isnan(cmdParams[Z])) if(!moveAxis(Z,motion[Z],isnan(feedRate)?MAXSPEED[Z]:feedRate)) fail = true;
  if(!isnan(cmdParams[E])) if(!moveAxis(E,motion[E],isnan(feedRate)?MAXSPEED[E]:feedRate)) fail = true;

  if(!fail) {
    acknowledgeCommand();
    movementLine = cmdLine;
    startStepperControl();
  } else {
    invalidCommand();
    resetAxes();
  }
}

void cmdLinearInterpolation() { //G01
  if(cmdParams[F] > MAXSPEED_LINEAR || cmdParams[S] > MAXSPEED[E]) {
    invalidCommand();
    return;
  }
  
  float motion[4];
  if(isFlagSet(FLAG_ABSOLUTE_MODE)) {
    motion[X] = isnan(cmdParams[X])?0:(cmdParams[X] - Axes[X].position*STEPLENGTH[X]);
    motion[Y] = isnan(cmdParams[Y])?0:(cmdParams[Y] - Axes[Y].position*STEPLENGTH[Y]);
    motion[Z] = isnan(cmdParams[Z])?0:(cmdParams[Z] - Axes[Z].position*STEPLENGTH[Z]);
  } else {
    motion[X] = isnan(cmdParams[X])?0:cmdParams[X];
    motion[Y] = isnan(cmdParams[Y])?0:cmdParams[Y];
    motion[Z] = isnan(cmdParams[Z])?0:cmdParams[Z];
  }
  motion[E] = cmdParams[E];
  
  float travel = sqrt(pow2(motion[X]) + pow2(motion[Y]) + pow2(motion[Z]));
  
  if(cmdParams[F] > 0) feedRate = cmdParams[F];
  
  float velocity[3];
  velocity[X] = abs(feedRate * (motion[X] / travel));
  velocity[Y] = abs(feedRate * (motion[Y] / travel));
  velocity[Z] = abs(feedRate * (motion[Z] / travel));
  velocity[E] = abs(feedRate * (motion[E] / travel));
  
  bool fail = false;
  if(!isnan(cmdParams[X])) if(!moveAxis(X,motion[X],velocity[X])) fail = true;
  if(!isnan(cmdParams[Y])) if(!moveAxis(Y,motion[Y],velocity[Y])) fail = true;
  if(!isnan(cmdParams[Z])) if(!moveAxis(Z,motion[Z],velocity[Z])) fail = true;
  if(!isnan(cmdParams[E])) if(!moveAxis(E,motion[E],velocity[E])) fail = true;

  if(!fail) {
    acknowledgeCommand();
    movementLine = cmdLine;
    startStepperControl();
  } else {
    invalidCommand();
    resetAxes();
  }
}

void cmdEcho() { //M77
  addToBufferS("ECHO N",6);
  addToBufferI(cmdLine);
  addToBufferS(" X",2);
  addToBufferF(cmdParams[X]);
  addToBufferS(" Y",2);
  addToBufferF(cmdParams[Y]);
  addToBufferS(" Z",2);
  addToBufferF(cmdParams[Z]);
  addToBufferS(" E",2);
  addToBufferF(cmdParams[F]);
  addToBufferS(" F",2);
  addToBufferF(cmdParams[E]);
  addToBufferS(" S",2);
  addToBufferF(cmdParams[S]);
  addToBufferS(" R",2);
  addToBufferF(cmdParams[R]);
  addToBufferC('\n');
}

inline void cmdHalt() { //M00
  unsetFlag(FLAG_ENABLE);
  addToBufferS("HALT N",6);
  addToBufferI(cmdLine);
  addToBufferC('\n');
}

void cmdResume() { //M97
  if(isFlagSet(FLAG_ENABLE) || !isFlagSet(FLAGS_AXES)) {
    invalidCommand();
    return;
  }
  addToBufferS("RESUME N",8);
  addToBufferI(cmdLine);
  addToBufferC('\n');
  
  startStepperControl();
}

void cmdRecover() { //M96
  if(isFlagSet(FLAG_ENABLE) || !isFlagSet(FLAGS_AXES)) {
    invalidCommand();
    return;
  }
  resetAxes(); 
  addToBufferS("RECOVER N",9);
  addToBufferI(cmdLine);
  addToBufferC('\n');
}

void cmdGetPosition() { //M114
  addToBufferS("POS N",5);
  addToBufferI(cmdLine);
  addToBufferS(" X",2);
  addToBufferF(Axes[X].position*STEPLENGTH[X]);
  addToBufferS(" Y",2);
  addToBufferF(Axes[Y].position*STEPLENGTH[Y]);
  addToBufferS(" Z",2);
  addToBufferF(Axes[Z].position*STEPLENGTH[Z]);
  addToBufferS(" E",2);
  addToBufferF(Axes[E].position*STEPLENGTH[E]);
  addToBufferC('\n');
}

void cmdSetTemperature() { //M104
  if(!isnan(cmdParams[S])) activeTemperature = cmdParams[S];
  if(!isnan(cmdParams[R])) idleTemperature = cmdParams[R];
  if(!isnan(cmdParams[F])) temperatureTolerance = cmdParams[F];
  acknowledgeCommand();
}

void cmdSetPos() { //G92
  if(!isnan(cmdParams[X])) Axes[X].position = cmdParams[X] / STEPLENGTH[X];
  if(!isnan(cmdParams[Y])) Axes[Y].position = cmdParams[Y] / STEPLENGTH[Y];
  if(!isnan(cmdParams[Z])) Axes[Z].position = cmdParams[Z] / STEPLENGTH[Z];
  acknowledgeCommand();
}

void cmdGetTime() { //M71
  addToBufferS(" M71 T",5);
  addToBufferUI(micros());
  addToBufferS(" S",2);    
  addToBufferI(timeInterval);
  addToBufferC('\n');
}

void cmdGetFlags() { //M70
  addToBufferS(" M70 ",5);
  addToBufferI(stateFlags);
  addToBufferC('\n');
}

void cmdGetTemp() { //M105
  addToBufferS("M105 T",6);
  addToBufferF(getExtruderTemperature());
  addToBufferS(" S",2);    
  addToBufferF(activeTemperature);
  addToBufferS(" R",2);    
  addToBufferF(idleTemperature);
  addToBufferS(" F",2);    
  addToBufferI(isFlagSet(FLAG_HOTEND_ON)? (isFlagSet(FLAG_ACTIVE_TEMP)? 2 : 1) : 0);
  addToBufferC('\n');
}

void cmdGetSteps() { //M74
  addToBufferS("M74 X",5);
  addToBufferF(Axes[X].steps);
  addToBufferS(" Y",2);
  addToBufferF(Axes[Y].steps);
  addToBufferS(" Z",2);
  addToBufferF(Axes[Z].steps);
  addToBufferS(" E",2);
  addToBufferF(Axes[E].steps);
  addToBufferS(" F",2);
  addToBufferF(Axes[X].lastMicros);
  addToBufferS(" S",2);
  addToBufferF(Axes[X].stepTime);
  addToBufferC('\n');
}

void cmdHotendPassive() { //M03
  startTemperatureControl(false);
  acknowledgeCommand();
}

void cmdHotendActive() { //M04
  startTemperatureControl(true);
  acknowledgeCommand();
}

void cmdHotendOff() { //M05
  stopTemperatureControl();
  acknowledgeCommand();
}

void cmdAbsoluteMode() { //G90
  setFlag(FLAG_ABSOLUTE_MODE);
  acknowledgeCommand();
}

void cmdIncrementalMode() { //G91
  unsetFlag(FLAG_ABSOLUTE_MODE);
  acknowledgeCommand();
}

void cmdSetPID() { //M301
  if(!isnan(cmdParams[P])) KP  = cmdParams[P];
  if(!isnan(cmdParams[I])) KI  = cmdParams[I];
  if(!isnan(cmdParams[D])) KD  = cmdParams[D];
  
  //Secret parameter
  if(!isnan(cmdParams[F])) KI2 = cmdParams[F];
  
}

void cmdGetPID() { //M136
  addToBufferS("M136 KP",7);
  addToBufferF(KP);
  addToBufferS(" KI",3);
  addToBufferF(KI);
  addToBufferS(" KD",3);
  addToBufferF(KD);
  addToBufferS(" KI2",4);
  addToBufferF(KI2);
  addToBufferC('\n');
}

void cmdNone() { //No command
  addToBufferS("NONE N",6);
  addToBufferI(cmdLine);
  addToBufferC('\n');
}
