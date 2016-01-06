#include "Base.h"

UBYTE stateFlags = 0x00;
char serialBuffer[64];
UBYTE bufferPos = 0;
volatile ULONG timeInterval = 4;

//Needed to make one that returns 0 on all invalid inputs
UINT sParseUINT(const char *str) {
  UINT buffer = 0;
  for (int i = 0; str[i] != '\0'; i++) {
    if(!isdigit(str[i])) return 0;
    buffer = 10*buffer + (str[i] - 48);
  }
  return buffer;
}

float sParseFloat(const char *str) {
  if(str[0] == '-')
    return -sParseFloat(str+1); //Skip first char
  float buffer = 0;
  float m1 = 10, m2 = 1, m2r = 1; //M1 is buffer multiplier, M2 is current digit multiplier multiplier, M2r is the current digit multiplier
                                  //It's a simple idea, but I suck at explaining :P
  
  for (int i = 0; str[i] != '\0'; i++) {
    if(!isdigit(str[i])) {
      if(str[i] == '.' && m1 == 10) { //decimal point
        //Change multipliers
        m1 = 1;
        m2r = m2 = 0.1;
        continue;
      } else return NAN;//Fail
    }
    buffer = m1*buffer + m2r * float(str[i] - 48); //nice way to cast char to int to float
    m2r *= m2;//Go deeper into decimals
  }
  return buffer;
}

//Adds string to ouutput buffer
void addToBufferS(const char *data, UBYTE len) {
  memcpy(serialBuffer+bufferPos,data,len);
  bufferPos += len;
}

//Adds float to output buffer
void addToBufferF(float data) {
  dtostrf(data,1,2,serialBuffer+bufferPos);
  while(serialBuffer[++bufferPos] != '\0') continue;
}

//Adds char to output buffer
void addToBufferC(char data) {
  serialBuffer[bufferPos++] = data;
}

//Adds int/long to output buffer
void addToBufferI(long data) {
  bufferPos += sprintf(serialBuffer+bufferPos, "%d", data);
} 

//Adds UINT/ULONG to output buffer
void addToBufferUI(ULONG data) {
  bufferPos += sprintf(serialBuffer+bufferPos, "%lu", data);
} 

//Output too serial
void flushSerial() {
  if(bufferPos != 0) {
      serialBuffer[bufferPos] = '\0';
      Serial.print(serialBuffer);
      bufferPos = 0;
  }
}
