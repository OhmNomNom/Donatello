#ifndef Interpreter_h_
#define Interpreter_h_

#include "Base.h"
#include "Axes.h"
#include "Extruder.h"

static const UBYTE CMDBUFFER_SIZE = 16; //Max characters for command buffer (b/w spaces)

enum Command : UBYTE {
  CMD_NONE = 0,              //No command
  
  CMD_RPOS,                  //Rapid positioning            (G00)
  CMD_LINEAR,                //Linear interpolation         (G01)
  CMD_MODEABS,               //Absolute positioning         (G90)
  CMD_MODEINC,               //Incremental positioning      (G91)
  CMD_SETPOS,                //Set current position         (G92)
  
  CMD_HALT,                  //HALT motion                  (M00)
  CMD_HOTEND_PASSIVE,        //Run hotend at passive temp   (M03)
  CMD_HOTEND_ACTIVE,         //Run hotend at Active temp    (M04)
  CMD_HOTEND_OFF,            //Turn off hotend              (M05)
  CMD_FLAGS,                 //Get flags DEBUGGING          (M70)
  CMD_GET_TIME,              //Get time DEBUGGING           (M71)
  CMD_GET_STEPS,             //Get steps DEBUGGING          (M74)
  CMD_ECHO,                  //Echo                         (M77)
  CMD_RECOVER,               //Recover from HALT            (M96)
  CMD_RESUME,                //Resume motion                (M97)
  CMD_SETTEMP,               //Set target temperature       (M104)
  CMD_GETTEMP,               //Get temperature              (M105)
  CMD_POSITION,              //Get position                 (M114)
};

//Our current state in interpreting
enum CommandStates : UBYTE {
  STATE_LINENUM = 0,
  STATE_COMMAND,
  STATE_PARAMS,
  STATE_CHECKSUM,
  
  STATE_INVALID = 255
};

extern UINT cmdLine,
            lineCounter,
            movementLine;
extern Command command;
extern UBYTE bufferPosition;
extern CommandStates cmdState;
extern char cmdBuffer[],
            checksum;
extern float cmdParams[];


void initInterpreter();
void interpret(char);
void process();
void execCommand();
inline void clearPrevCommand();
inline void invalidCommand();
inline void acknowledgeCommand();
void doneMoving();
void cmdRapidPositioning();
void cmdLinearInterpolation();
void cmdEcho();
inline void cmdHalt();
void cmdResume();
void cmdRecover();
void cmdPosition();
void cmdSetTemperature();
void cmdSetPos();
void cmdGetTime();
void cmdGetFlags();
void cmdGetTemp();
void cmdGetSteps();
void cmdHotendPassive();
void cmdHotendActive();
void cmdHotendOff();
void cmdAbsoluteMode();
void cmdIncrementalMode();
void cmdNone();

#endif
