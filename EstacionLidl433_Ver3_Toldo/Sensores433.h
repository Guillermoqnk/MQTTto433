#ifndef SENSORS433
#define SENSORS433

#include "Arduino.h"
#define comparaFloat(a,b) (fabs(a-b)>0.05)
#define LONGMEN 36
#define TOLERANCIA 0.2
#define TOLERANCIAH (1.0+TOLERANCIA)
#define TOLERANCIAL (1.0-TOLERANCIA)
#define SINC 525
const unsigned int SINCH = (SINC*TOLERANCIAH);
const unsigned int SINCL = (SINC*TOLERANCIAL);
#define CERO 2100
const unsigned int CEROH  =(CERO*TOLERANCIAH);
const unsigned int CEROL  =(CERO*TOLERANCIAL);
#define UNO 4200
const unsigned int UNOH  =(UNO*TOLERANCIAH);
const unsigned int UNOL  =(UNO*TOLERANCIAL);
#define ESPACIO 8400
const unsigned int ESPACIOL =(ESPACIO*TOLERANCIAL);
const unsigned int ESPACIOH =(ESPACIO*TOLERANCIAH);
bool Espacio = 1; //1 espacio 0 sinc
bool Datos = 0; // 1 datos 0 sinc
bool BufferFull=0;
volatile unsigned long lectura=0;
int mensaje=0;
byte sensorTipo;


void intRF();
bool imprimeDeca(float &temperatura);
bool imprimeLidl(float &temperatura);
void enviaTempMqtt(char *sensor,float temperatura);
#endif
