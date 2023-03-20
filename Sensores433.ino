//#include "Sensores433.h"
#define comparaFloat(a,b) (fabs(a-b)>0.05)
/*struct bitsMeans{
  byte inicio;
  byte fin;
  char nombre[];
};
enum sensorName {DECAT=0,LIDL};
static const bitsMeans sensorStruct[DECAT][]={{0,3,"decimales"},{4,7,"pad"},{8,14,"entera"},{15,15,"signo"},{16,23,"canal"}}; //sensor de teperatura del Decathlon
static const bitsMeans sensorStruct[LIDL][]={{0,3,"crc"},{4,11,"pad"},{12,23,"grados"},{24,24,"C/F"},{25,26,"nose"},{27,32,"canal"}}; //Sensor de temperatura del Lidl

bool extraeDatosSensor(unsigned long muestra,sensorName sensor)
{
byte tamaño=sizeof(sensorStruct)/sizeof(bitsMeans);
for(k=0;k<
}*/

bool imprimeDeca(float &temperatura)
{
  /*0-3 decimales
   * 4-7 pad
   * 8-14 entera
   * 15 Signo
   * 16-23 canal
   * 
   */
  static byte muestras=0;
  static int anterior;
  static unsigned long tiempo=0;
  unsigned long intervalo=millis()-tiempo;
  if(intervalo<2000)return false; //tiempo solo se actualiza cuando el dato es correcto
  byte decimal=bitsRead(lectura,0,3); // lee la parte decimal
  byte grados=bitsRead(lectura,8,14); //lee siete 
  byte signo=bitsRead(lectura,15,15);
  byte IDCanal=bitsRead(lectura,16,23); //lee el canal 
  if(!muestras){anterior=grados*10+decimal;muestras++;}//si es la primera muestra inicio la variable
    else if (anterior==(grados*10+decimal))muestras++;//si no es cero miro para ver si es igual 
      else muestras=0;
  if(muestras>1)
  {
   muestras=0;
  
  Serial.print(" Time:");
  Serial.print(millis()/100);
  Serial.print(" Canal:");
  Serial.print(IDCanal);
  Serial.print(" Temperatura:");
  Serial.print(grados);
  Serial.print(".");
  Serial.print(decimal);
  Serial.print(" Pad:");
  Serial.println(bitsRead(lectura,4,7));
  tiempo=millis();
  if(IDCanal==canalDeca){
    float tempActual=grados+decimal/10.0;
    enviaTempMqtt("Interior",tempActual);
    if(comparaFloat(temperatura,tempActual)){temperatura=tempActual;return true;}
  }
  }
  return false;
}


bool imprimeLidl(float &temperatura)
{
  /*
   * 0-3 crc
   * 4-11 pad
   * 12-23 grados invertir lsb firt
   * 24 C/F
   * 25-26 nose 
   * 27-31 Id canal 
   */
  // esto no esta bien, trasnmite 36 bit y lo mee en un unsigned long que son 32
  static byte muestras=0;
  static int anterior;
  static unsigned long tiempo=0;
  unsigned long intervalo=millis()-tiempo;
  if(intervalo<2000)return false; //tiempo solo se actualiza cuando el dato es correcto
 
  int grados=bitsRead(lectura,12,23);
  int temp=0;
  for (byte n=0;n<12;n++)temp = ( temp << 1 ) | ( 0x0001 & ( grados >> n ) ); //invierte los bits
  //Serial.print(temp,BIN);
  if(temp & 0x0800)
  {
 /* Serial.print(",");
  Serial.print(temp,BIN);
  temp= temp ^ 0xFFF;
  Serial.print(",");
  Serial.print(temp,BIN);
  temp++;
  Serial.print(temp);
  //grados=(temp)&& 0xFFF; mal solo un &
  Serial.print(",");
  Serial.println(grados);
  grados=-temp ;*/
  //a | ~((1 << 12) - 1)=~(0b0001000000000000-1)=~(4096-1)=~(4095)=~(0b0000 1111 1111 1111)=0b11111 0000 0000 000 esto me hace suponer que los int se guardan en complemento a 2
  grados=temp | ~((1 << 12) - 1);//grados=temp | 0xf000
  }else grados=temp;
  //grados=(temp & 0x0800)? temp | 0xf000:temp;
  byte IDCanal=bitsRead(lectura,27,32);
  
  if(!muestras){anterior=grados;muestras++;}
    else if (anterior==grados)muestras++;
      else muestras=0;
  if(muestras>1){
     muestras=0;
    //if(IDCanal==44){

    Serial.print(" Time:");
    Serial.print(millis()/100);
    Serial.print(" Canal:");
    Serial.print(IDCanal);
    Serial.print(" nose:");
    Serial.print(bitsRead(lectura,25,26));
    Serial.print(" Temperatura:");
    Serial.print(grados/10.0);
    Serial.print(bitsRead(lectura,24,24)?"F ":"C ");
    Serial.print(" Pad:");
    Serial.print(bitsRead(lectura,4,11));
    Serial.print(" crc:");
    Serial.println(bitsRead(lectura,0,3)); 
    Serial.print(" canalLidl:");
    Serial.println(canalLidl); 
    tiempo=millis(); 
    //temperatura=grados/10.0;
    if(IDCanal==canalLidl){
      if(grados > -120 && grados<500)
      enviaTempMqtt("Exterior",grados/10.0);
      if (comparaFloat(temperatura,grados/10.0)){temperatura=grados/10.0;return true;}
    }
  }
  return false;
}

void enviaTempMqtt(char *sensor,float temperatura)
{
  if (!clientMqtt.connected()) {
    reconnect();
  }
  clientMqtt.loop();

    char cadena[6];
    String(temperatura).toCharArray(cadena,5);
    Serial.print("Publish message: ");
    Serial.print(sensor);
    Serial.print(" : ");
    Serial.println(temperatura);
    clientMqtt.publish(sensor, cadena);
    clientMqtt.publish("Hora",getTimestampAsString().c_str());
    
}

unsigned long bitsRead(unsigned long n,byte inicio, byte final) //ambos inclusive
{
 n=n>>inicio;
 byte longitud=final-inicio;
 n=n & bit(longitud+1)-1;
 return n;
}
