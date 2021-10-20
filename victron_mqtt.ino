#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#include <SoftwareSerial.h>
#include "VeDirectFrameHandler.h"
#define TRIGGER_PIN 0

bool reconfig=false;
const char* fwUrlBase = "http://test.ga1a.eu/ota/";
const int FW_VERSION = 0;
//modificar el numero de la nave
const char*  topic = "fp/taller";
const char* TYPE = "victron";
const char* mqtt_server = "bqtest.dyndns.org";
const char* mqttUser = "bqtest";
const char* mqttPassword = "almaciga";

unsigned long lastupdate = 0; // check FW update every FRECUENCY*720
unsigned long sendupdate = 0; // minimal sendupadate





String mac;
WiFiClient espClient;
PubSubClient client(espClient);
VeDirectFrameHandler myve;

// SoftwareSerial
#define rxPin D7                            // RX using Software Serial so we can use the hardware UART to check the ouput
#define txPin D8                            // TX Not used
SoftwareSerial veSerial(rxPin, txPin);  
# define FRECUENCY 7000
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
     veSerial.begin(19200);                  // input serial port (VE device)
    veSerial.flush();
  getMac();
  client.setServer(mqtt_server, 1883);

  pinMode(TRIGGER_PIN, INPUT_PULLUP);

attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), reconfigure, FALLING);

}

void loop() {

 

  if (millis() - lastupdate > FRECUENCY * 150) {
    lastupdate = millis();
   // checkForUpdates();
  }





  ReadVEData();
   // if (!client.connected()) {
    //  reconnect();
   // }
  //  client.loop();
    
   // sendValues();
  
PrintData();
  delay(FRECUENCY);



}


//mqtt



void reconnect() {
  // Loop until we're reconnected
  unsigned long now = millis();
  while (!client.connected() && millis() - now < 25000 ) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");



    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay2(5000);
    }
  }
}



bool sendValues() {
  String s;
  s="{\"key\":\"type\",\"value\":\""+String(TYPE)+"\"},{\"key\":\"version\",\"value\":"+String(FW_VERSION)+"}, {\"key\":\"mac\",\"value\":\""+mac+"\"}";
   for ( int i = 0; i < myve.veEnd; i++ ) {
    Serial.print(myve.veName[i]);
     s += ",{\"key\":\""+String(myve.veName[i])+"\",\"value\":" + String(myve.veValue[i]) + "}";
    Serial.print("= ");
    Serial.println(myve.veValue[i]);    
    }
 
   int msgLen = 0;

  
     client.loop();

  msgLen += s.length()+2;
  Serial.print("msgLen=");Serial.println(msgLen);
  Serial.println(s);
 client.beginPublish(topic, msgLen, false);    Serial.print("Publish message: ");
    client.print("[");
    client.print(s);
   
    client.print("]");
     client.endPublish();
    
    delay2(1000);
   

  Serial.println();
  Serial.println("closing connection");



  return true;
}




void delay2(unsigned long espera)
{
  unsigned long tiempo = millis();



  while ( millis() - tiempo < espera) {
    if (tiempo > millis()) tiempo = 0;
if (reconfig){ 
  detachInterrupt(digitalPinToInterrupt(TRIGGER_PIN)) ;
  reconfig=false;
  WiFiManager wifiManager;
    wifiManager.resetSettings();
delay(1000);
    if (!wifiManager.autoConnect("AutoConnectAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.reset();
      delay(5000);
    }
   // attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), reconfigure, FALLING);

}
    delay(10);
  }
}

void getMac() {

  mac = WiFi.macAddress();
  mac.replace(":", "-");


}

 // is configuration portal requested?
void reconfigure() {
   
    reconfig=true;
  }

void  ReadVEData() {
    while ( veSerial.available() ) {
        myve.rxData(veSerial.read());
    }
    yield();
    
}


void PrintData() {
    for ( int i = 0; i < myve.veEnd; i++ ) {
    Serial.print(myve.veName[i]);
    Serial.print("= ");
    Serial.println(myve.veValue[i]);    
    }
}
