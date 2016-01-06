#ifndef Base_h_
#define Base_h_

#include <Arduino.h>
#include <math.h>

#define VERSION 0.1

typedef uint_fast32_t ULONG;
typedef int_fast32_t  SLONG;
typedef uint_fast8_t  UBYTE;
typedef int_fast8_t   SBYTE;
typedef uint_fast16_t UINT;

#define strEqual(m,n) (strcmp(m,n) == 0)
#define pow2(n) (n*n)
#define isFlagSet(n) (stateFlags & (n))
#define setFlag(n) (stateFlags |= (n))
#define unsetFlag(n) (stateFlags &= ~(n))

static const UBYTE FLAG_ENABLE        = 0x01,
                   FLAG_X             = 0x02,
                   FLAG_Y             = 0x04,
                   FLAG_Z             = 0x08,
                   FLAG_E             = 0x10,
                   FLAGS_AXES         = FLAG_E | FLAG_Z | FLAG_Y | FLAG_X,
                   FLAG_HOTEND_ON     = 0x20,
                   FLAG_ABSOLUTE_MODE = 0x40,
                   FLAG_ACTIVE_TEMP   = 0x80;
                   
const ULONG PID_PERIOD      = 200000,
            WORKER_PERIOD   = 15;

enum ParamIndex : UBYTE {
  X = 0,
  Y,
  Z,
  E,
  F,//Feed rate
  S,
  R
};

extern UBYTE stateFlags;
extern char serialBuffer[];
extern UBYTE bufferPos;
extern volatile ULONG timeInterval;

UINT  sParseUINT(const char*);
float sParseFloat(const char*);
void  addToBufferS(const char*,UBYTE);
void  addToBufferF(float);
void  addToBufferC(char);
void  addToBufferI(long);
void  addToBufferUI(ULONG);
void  flushSerial(void);

#endif
