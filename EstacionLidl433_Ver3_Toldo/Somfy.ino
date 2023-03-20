/*   This sketch allows you to emulate a Somfy RTS or Simu HZ remote.
   If you want to learn more about the Somfy RTS protocol, check out https://pushstack.wordpress.com/somfy-rts-protocol/
   
   The rolling code will be stored in EEPROM, so that you can power the Arduino off.
   
   Easiest way to make it work for you:
    - Choose a remote number
    - Choose a starting point for the rolling code. Any unsigned int works, 1 is a good start
    - Upload the sketch
    - Long-press the program button of YOUR ACTUAL REMOTE until your blind goes up and down slightly
    - send 'p' to the serial terminal
  To make a group command, just repeat the last two steps with another blind (one by one)
  
  Then:
    - m, u or h will make it to go up
    - s make it stop
    - b, or d will make it to go down
    - you can also send a HEX number directly for any weird command you (0x9 for the sun and wind detector for instance)
*/
#include "Somfy.h"
unsigned int newRollingCode = 1580;       //<-- Change it!
unsigned int rollingCode = 0;
byte checksum;




void iniSomfy() {
  
  //DDRD |= 1<<PORT_TX; // Pin 5 an output
  pinMode(PORT_TX,OUTPUT);
  pinMode(PORT_ENABLE,OUTPUT);
  //PORTD &= !(1<<PORT_TX); // Pin 5 LOW
  digitalWrite(PORT_TX,LOW);
   digitalWrite(PORT_ENABLE,LOW);
  EEPROM.begin(512);

  //ojo cambiar
 //EEPROM.put(EEPROM_ADDRESS, newRollingCode);
   // EEPROM.commit();
  
  if (EEPROM.get(EEPROM_ADDRESS, rollingCode) < newRollingCode) {
    EEPROM.put(EEPROM_ADDRESS, newRollingCode);
    EEPROM.commit();
  }
  Serial.print("Simulated remote number : "); Serial.println(REMOTE, HEX);
  Serial.print("Current rolling code    : "); Serial.println(rollingCode);
}

/*void loop() {
  if (Serial.available() > 0) {
    char serie = (char)Serial.read();
    Serial.println("");
//    Serial.print("Remote : "); Serial.println(REMOTE, HEX);
    if(serie == 'm'||serie == 'u'||serie == 'h') {
      Serial.println("Monte"); // Somfy is a French company, after all.
      BuildFrame(frame, HAUT);
    }
    else if(serie == 's') {
      Serial.println("Stop");
      BuildFrame(frame, STOP);
    }
    else if(serie == 'b'||serie == 'd') {
      Serial.println("Descend");
      BuildFrame(frame, BAS);
    }
    else if(serie == 'p') {
      Serial.println("Prog");
      BuildFrame(frame, PROG);
    }
    else {
      Serial.println("Custom code");
      BuildFrame(frame, serie);
    }

    Serial.println("");
    SendCommand(frame, 2);
    for(int i = 0; i<2; i++) {
      SendCommand(frame, 7);
    }
  }
}

*/
void BuildFrame(byte *frame, byte button) {
  unsigned int code;
  EEPROM.get(EEPROM_ADDRESS, code);
  frame[0] = 0xA7; // Encryption key. Doesn't matter much
  frame[1] = button << 4;  // Which button did  you press? The 4 LSB will be the checksum
  frame[2] = code >> 8;    // Rolling code (big endian)
  frame[3] = code;         // Rolling code
  frame[4] = REMOTE >> 16; // Remote address
  frame[5] = REMOTE >>  8; // Remote address
  frame[6] = REMOTE;       // Remote address

  Serial.print("Frame         : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) { //  Displays leading zero in case the most significant
      Serial.print("0");     // nibble is a 0.
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }
  
// Checksum calculation: a XOR of all the nibbles
  checksum = 0;
  for(byte i = 0; i < 7; i++) {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum &= 0b1111; // We keep the last 4 bits only


//Checksum integration
  frame[1] |= checksum; //  If a XOR of all the nibbles is equal to 0, the blinds will
                        // consider the checksum ok.

  Serial.println(""); Serial.print("With checksum : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }

  
// Obfuscation: a XOR of all the bytes
  for(byte i = 1; i < 7; i++) {
    frame[i] ^= frame[i-1];
  }

  Serial.println(""); Serial.print("Obfuscated    : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }
  Serial.println("");
  Serial.print("Rolling Code  : "); Serial.println(code);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["remote"] = REMOTE;
  root["rolling"] = code;
  String jsonStr;
  root.printTo(jsonStr);
  clientMqtt.publish("config/toldo", jsonStr.c_str());
  //ojo cambiar a 1
  EEPROM.put(EEPROM_ADDRESS, code + 1); //  We store the value of the rolling code in the
                                        // EEPROM. It should take up to 2 adresses but the
                                        // Arduino function takes care of it.
  EEPROM.commit();
}

void SendCommand(byte *frame, byte sync) {
  digitalWrite(PORT_ENABLE,1);
  if(sync == 2) { // Only with the first frame.
  //Wake-up pulse & Silence
    //PORTD |= 1<<PORT_TX; //1
    digitalWrite(PORT_TX, 1);
    delayMicroseconds(9415);
    //PORTD &= !(1<<PORT_TX);//0
    digitalWrite(PORT_TX, 0);
    delayMicroseconds(89565);
  }

// Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < sync; i++) {
    //PORTD |= 1<<PORT_TX;//1
    digitalWrite(PORT_TX, 1);
    delayMicroseconds(4*SYMBOL);
    //PORTD &= !(1<<PORT_TX);//0
    digitalWrite(PORT_TX, 0);
    delayMicroseconds(4*SYMBOL);
  }

// Software sync
  //PORTD |= 1<<PORT_TX;
  digitalWrite(PORT_TX, 1);
  delayMicroseconds(4550);
  //PORTD &= !(1<<PORT_TX);
  digitalWrite(PORT_TX, 0);
  delayMicroseconds(SYMBOL);
  
  
//Data: bits are sent one by one, starting with the MSB.
  for(byte i = 0; i < 56; i++) {
    if(((frame[i/8] >> (7 - (i%8))) & 1) == 1) {
      //PORTD &= !(1<<PORT_TX);
      digitalWrite(PORT_TX, 1);
      delayMicroseconds(SYMBOL);
      //PORTD ^= 1<<5;
      digitalWrite(PORT_TX, 0);
      delayMicroseconds(SYMBOL);
    }
    else {
      //PORTD |= (1<<PORT_TX);
      digitalWrite(PORT_TX, 0);
      delayMicroseconds(SYMBOL);
      //PORTD ^= 1<<5;
      digitalWrite(PORT_TX, 1);
      delayMicroseconds(SYMBOL);
    }
  }
  
  //PORTD &= !(1<<PORT_TX);
  digitalWrite(PORT_TX, 0);
  delayMicroseconds(30415); // Inter-frame silence
  digitalWrite(PORT_ENABLE,0);
}
