#ifndef SOMFY
#define SOMFY
#include <EEPROM.h>
#define PORT_TX D5 //5 of PORTD = DigitalPin 5
#define PORT_ENABLE EN_TX

#define SYMBOL 640
#define HAUT 0x2
#define STOP 0x1
#define BAS 0x4
#define PROG 0x8
#define EEPROM_ADDRESS 0
byte frame[7];
void BuildFrame(byte *frame, byte button);
void SendCommand(byte *frame, byte sync);
#define REMOTE 0x121300    //<-- Change it! Código del mando
#endif
