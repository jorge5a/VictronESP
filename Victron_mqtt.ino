/*************************************************************************************
 ReadVeDirectFrameHandler

 Uses VeDirectFrameHandler library

 This example and library tested with NodeMCU 1.0 using Software Serial.
 If using with a platform containing 2 harware UART's, use those, not SoftwareSerial.
 Tested with Victron BMV712.

 VEDirect Device:
   pin 1 - gnd
   pin 2 - RX
   pin 3 - TX
   pin 4 - power

 History:
   2020.05.05 - 0.3 - initial release

**************************************************************************************/

#include "Arduino.h"
#include <SoftwareSerial.h>
#include "VeDirectFrameHandler.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


VeDirectFrameHandler myve;

// SoftwareSerial
#define rxPin D7                            // RX using Software Serial so we can use the hardware UART to check the ouput
#define txPin D8                            // TX Not used
SoftwareSerial veSerial(rxPin, txPin);         
const char* ssid = "ciclope1_rep";
const char* password = "almaciga1275c";
const int FW_VERSION = 1;
const char* topic = "fp/taller/victron";
const char* mqtt_server = "bqtest.dyndns.org";
const char* mqttUser = "bqtest";
const char* mqttPassword = "almaciga";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
	Serial.begin(115200);                   // output serial port
    veSerial.begin(19200);                  // input serial port (VE device)
    veSerial.flush();
     setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {    
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
    ReadVEData();
    EverySecond();
}

void ReadVEData() {
    while ( veSerial.available() ) {
        myve.rxData(veSerial.read());
    }
    yield();
}

void EverySecond() {
    static unsigned long prev_millis;

    if (millis() - prev_millis > 15000) {
        PrintData();
        prev_millis = millis();
        sendValues();
    }
}

void PrintData() {
    for ( int i = 0; i < myve.veEnd; i++ ) {
    Serial.print(myve.veName[i]);
    Serial.print("= ");
    Serial.println(myve.veValue[i]);    
    }
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqttUser, mqttPassword)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void  sendValues() {
  String s;
  
  s="{\"key\":\"version\",\"value\":"+String(FW_VERSION)+"}";
  for ( int i = 0; i < myve.veEnd; i++ ) {
    if (isValidNumber(myve.veValue[i])){  
      
      s += ",{\"key\":\""+String(myve.veName[i])+"\",\"value\":" + String(myve.veValue[i]) + "}";
      }
      else{
        s += ",{\"key\":\""+String(myve.veName[i])+"\",\"value\": \"" + String(myve.veValue[i]) + "\"}";
      }
    
    Serial.print(myve.veName[i]);
     
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
    
   
   

  Serial.println();
  Serial.println("closing connection");



 
}

boolean isValidNumber(String str){
boolean isNum=false;
for(byte i=0;i<str.length();i++)
{
isNum = isDigit(str.charAt(i))  || str.charAt(i) == '.' || str.charAt(i) == '-';
if(!isNum) return false;
}
return isNum;
}
