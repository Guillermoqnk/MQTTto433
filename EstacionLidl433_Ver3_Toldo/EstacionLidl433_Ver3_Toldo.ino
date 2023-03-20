/****************************************************
 * Definir los margenes * *******************************************
 */
//8395857 100000000001110001010001
//8395860 100000000001110001010100
//LOLIN WEMOS D1 R2 y mini
#include <ESP8266WiFi.h>  
#include "DebugMacros.h"
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "MiRed.h"
#include "Somfy.h"


// for stack analytics
extern "C" {
#include <cont.h>
  extern cont_t g_cont;
}
//#include "Sensores433.h"
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

Ticker dispacher;
Ticker apagaMqtt;
Ticker persianaTimer;
int tiempo_persiana=10;//tiempo total igual a 0.6*10=6 Sg
const char* idMqtt="RF433";
WiFiClient espClient;
PubSubClient clientMqtt(espClient);
unsigned long lectura=0;
unsigned long lastCode;
const char* host = "RF433";
 
#define PORTA_RF  D2
#define INT_RF    0
#define DEBUG 1
#define LED_DEBUG D4
#define EN_TX D6

int canalDeca=150;
int canalLidl=11;
//otros canales deca=97

void syncNTPTime(void);
String getTimestampAsString();


void setup()
{
  Serial.begin(115000);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  

//******************************************
ArduinoOTA.setHostname(host);
 ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();


//*************************************
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.getMode());
  
  clientMqtt.setServer(mqtt_server, 1883);
  clientMqtt.setCallback(callback);
  syncNTPTime();
  reconnect();
  delay(500);
  Serial.println("INICIADO!");
  pinMode(PORTA_RF, INPUT);
  digitalWrite(PORTA_RF, HIGH);
  pinMode(LED_DEBUG, OUTPUT);
  digitalWrite(LED_DEBUG, HIGH);
  pinMode(EN_TX, OUTPUT);
  digitalWrite(EN_TX, LOW);
  mySwitch.enableReceive(digitalPinToInterrupt(D2));  // Receiver on interrupt 0 => that is pin #2
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  iniSomfy();
  mySwitch.enableTransmit(D5);
}
void cierraPuerta(void)
{
 clientMqtt.publish("entrada/puerta", "OFF"); clientMqtt.publish("cocina/humo", "OFF"); clientMqtt.publish("presencia/pasillo", "OFF");
}

void apagaHumo(void)
{
clientMqtt.publish("entrada/puerta", "OFF"); clientMqtt.publish("cocina/humo", "OFF"); clientMqtt.publish("presencia/pasillo", "OFF"); 
}

void apagapresencia(void)
{
clientMqtt.publish("entrada/puerta", "OFF"); clientMqtt.publish("cocina/humo", "OFF"); clientMqtt.publish("presencia/pasillo", "OFF");

}


void loop() {
  
  static int error_count = 0;
  static int connect_count = 0;
  const unsigned int MAX_CONNECT = 20;
  static bool flag = false;
  static float sensorLidl,sensorDeca;


ArduinoOTA.handle();    
if(mySwitch.available()) 
{
  lectura=mySwitch.getReceivedValue();
  bool hayDatos;
  
  output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["protocolo"] = mySwitch.getReceivedProtocol();
  root["valor"] = mySwitch.getReceivedValue();
  String jsonStr;
  root.printTo(jsonStr);
  clientMqtt.publish("config/rf/ultimo", jsonStr.c_str());
  
  switch( mySwitch.getReceivedProtocol()){
    case 1:
      if(mySwitch.getReceivedValue()==4755112){
        clientMqtt.publish("cocina/humo", "ON");
        apagaMqtt.once(30,apagaHumo);
      }
      else if(mySwitch.getReceivedValue()==14328606){
        clientMqtt.publish("entrada/puerta", "ON");
        apagaMqtt.once(30,cierraPuerta);
      }
      else if(mySwitch.getReceivedValue()==4422145){
        clientMqtt.publish("home/alarm/set", "ARM_AWAY");
        
      }
      else if(mySwitch.getReceivedValue()==4422146){
        clientMqtt.publish("home/alarm/set", "DISARM");
        
      }       
      else if(mySwitch.getReceivedValue()==54314){
        clientMqtt.publish("presencia/pasillo", "ON");
        apagaMqtt.once(5,apagapresencia);
      }
      
      delay(100);
      break;
    case 8 :
       
    case 9 :
     if(mySwitch.getReceivedBitlength()==24)hayDatos=imprimeDeca(sensorDeca); else  hayDatos=imprimeLidl(sensorLidl);
    break;
  }
  
    
    digitalWrite(LED_DEBUG, LOW);
    dispacher.once(0.5,apagaLed);

  

  mySwitch.resetAvailable();
}
if (error_count > 3){}
clientMqtt.loop();
delay(100);
}


void apagaLed(void){
  digitalWrite(LED_DEBUG, HIGH);
}

void repitePersiana(void)
{
static byte count=0;

mySwitch.send(lastCode,24); 
if(count++==tiempo_persiana){
  count=0;
  persianaTimer.detach();
  digitalWrite(EN_TX, LOW);
}

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if(!strcmp(topic,"config/sensores/deca/canal"))canalDeca=atoi((char *)payload);
    else if(!strcmp(topic,"config/sensores/lidl/canal"))canalLidl=atoi((char *)payload);
      else if(!strcmp(topic,"toldo/salon/set")){
            Serial.println("En Proceso toldo");
            if(!strncmp((char *)payload,"OPEN",length))BuildFrame(frame, HAUT);
              else if (!strncmp((char *)payload,"CLOSE",length))BuildFrame(frame, BAS);
                else if(!strncmp((char *)payload,"STOP",length))BuildFrame(frame, STOP);
                   //BuildFrame(frame, STOP);
                   SendCommand(frame, 2);
                   for(int i = 0; i<2; i++) { //creo que está repitiendo
                      SendCommand(frame, 7); //ojo aquí el Enable Trasnmit esta incluido en el codigo.
                    }
        
      }
        else if(!strcmp(topic,"persiana/cocina/set")){
          if(!strncmp((char *)payload,"OPEN",length)) lastCode=9848577;
            else if(!strncmp((char *)payload,"CLOSE",length)) lastCode=9848578;
              else if(!strncmp((char *)payload,"STOP",length))lastCode=9848578;
          
          //mySwitch.send(lastCode,24);
          digitalWrite(EN_TX, HIGH);
          tiempo_persiana=10;
          persianaTimer.attach(0.6,repitePersiana);
          //mySwitch.disableTransmit();
        }
         else if(!strcmp(topic,"persiana/habitacion/set")){
          if(!strncmp((char *)payload,"OPEN",length)) lastCode=9854145;
            else if(!strncmp((char *)payload,"CLOSE",length)) lastCode=9854146;
              else if(!strncmp((char *)payload,"STOP",length))lastCode=9854146;
          //mySwitch.send(lastCode,24);
          digitalWrite(EN_TX, HIGH);
          tiempo_persiana=40;
          persianaTimer.attach(0.6,repitePersiana);
          //mySwitch.disableTransmit();
        }
        
        else if (!strcmp(topic, "persiana/wylli/set")){
          if (!strncmp((char *)payload, "OPEN", length)) lastCode = 9865153;
          else if (!strncmp((char *)payload, "CLOSE", length)) lastCode = 9865154;
          else if (!strncmp((char *)payload, "STOP", length)) lastCode = 9865154;
          //mySwitch.send(lastCode,24);
          digitalWrite(EN_TX, HIGH);
          tiempo_persiana = 10;
          persianaTimer.attach(0.6, repitePersiana);
          //mySwitch.disableTransmit();
        }        
        
          else if(!strcmp(topic,"home/alarm")){
            if(!strncmp((char *)payload,"armed_away",length)) lastCode=4422145; //Armado
             else if(!strncmp((char *)payload,"armed_home",length)) lastCode=4422146; //apaga
              else if(!strncmp((char *)payload,"disarmed",length)) lastCode=4422146; //apaga
                else if(!strncmp((char *)payload,"triggered",length)) lastCode=4422148; //Sirena
                  /*else if(!strncmp((char *)payload,"pending",length)) lastCode=4422145;
                    else if(!strncmp((char *)payload,"armed_night",length)) lastCode=4422145;
                      else if(!strncmp((char *)payload,"armed_home",length)) lastCode=4422145;*/
                        else return;
             digitalWrite(EN_TX, HIGH);
             mySwitch.send(lastCode,24);
             digitalWrite(EN_TX, LOW);
          }
           else if(!strcmp(topic,"pasillo/luz/set")){
          if(!strncmp((char *)payload,"ON",length)) lastCode=9854245;
            else if(!strncmp((char *)payload,"OFF",length)) lastCode=9854246;
              else return;
              digitalWrite(EN_TX, HIGH);
             mySwitch.send(lastCode,24);
             digitalWrite(EN_TX, LOW);
        }
        else if(!strcmp(topic,"persiana/salon/set"))
        { if(!strncmp((char *)payload,"OPEN",length)) lastCode=8395860;
            else if(!strncmp((char *)payload,"CLOSE",length)) lastCode=8395857;
              else if(!strncmp((char *)payload,"STOP",length))lastCode=8395857;
          //mySwitch.send(lastCode,24);
          digitalWrite(EN_TX, HIGH);
          tiempo_persiana=10;
          persianaTimer.attach(0.6,repitePersiana);
          //mySwitch.disableTransmit();
          }
}

void reconnect() {
  // Loop until we're reconnected
  while (!clientMqtt.connected()) { //aunque haya pasado el tiempo el si pregunta le dicen que conectado y no vuelve a conectar
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    char topic[40];
    sprintf(topic,"Dispositivos/%s",idMqtt);
    Serial.println(topic);
    Serial.println(getTimestampAsString().c_str());
    if (clientMqtt.connect(idMqtt,mqtt_user,mqtt_pass,topic,0,true,"offline")) { // (clientID, username, password, willTopic, willQoS, willRetain, willMessage)
      Serial.println("connected");
      clientMqtt.publish(topic,"online",true);
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      if(clientMqtt.subscribe("config/sensores/lidl/#"))Serial.println("Suscrito a Lidl");
      clientMqtt.subscribe("config/sensores/deca/#");
      clientMqtt.subscribe("toldo/salon/set/#");
      clientMqtt.subscribe("persiana/cocina/set/#");
      clientMqtt.subscribe("persiana/cocina/pos/#");
      clientMqtt.subscribe("persiana/salon/set/#");
      clientMqtt.subscribe("persiana/salon/pos/#");
      clientMqtt.subscribe("persiana/habitacion/set/#");
      clientMqtt.subscribe("persiana/habitacion/pos/#");
      clientMqtt.subscribe("persiana/wylli/set/#");
      clientMqtt.subscribe("persiana/wylli/pos/#");
      clientMqtt.subscribe("home/alarm/#");
      clientMqtt.subscribe("pasillo/luz/set/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientMqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
