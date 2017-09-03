/*
  Box1

  modified  August 2017
  by Jacoby Daniel
  
*/

// Board NodeMCU 0.9 (ESP-12 Module) 

#include <Wire.h>             // We need to include Wire.h for I2C communication
#include "OLED.h"
#include <ESP8266WiFi.h> 
#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>     // http://knolleary.net/arduino-client-for-mqtt/ 
                              // https://pubsubclient.knolleary.net/api.html
// Mqtt Stuff

// ****************************** Cloud *********************************************
#define  CLOUD_MQTT  0         //https://www.cloudmqtt.com/ (Amazon Web Services)
#define  CLOUD_DIOTY 1        // http://www.dioty.co/    (colombian guy)
#define  CLOUD_DANY  2        // At home Local network    (At home)
//==========Select cloud======================

#define CLOUD CLOUD_DANY

//============================================
#if CLOUD==CLOUD_DANY

//const char MqttServer[]="mqtt.dioty.co";
IPAddress MqttServer(192,168,0,102);
const unsigned int MqttPort=1883; 
const char MqttUser[]="itba.jacoby@gmail.com";
const char MqttPassword[]="ce8acbf5";
const char MqttClientID[]="bigbox";

#elif CLOUD==CLOUD_DIOTY

const char MqttServer[]="mqtt.dioty.co";
const unsigned int MqttPort=1883; 
const char MqttUser[]="itba.jacoby@gmail.com";
const char MqttPassword[]="ce8acbf5";
const char MqttClientID[]="danyka";

#elif CLOUD==CLOUD_MQTT

const char MqttServer[]="m12.cloudmqtt.com";
const unsigned int MqttPort=16111; 
const char MqttUser[]="eplkycmz";
const char MqttPassword[]="KGMA6Z63gkn8";
const char MqttClientID[]="danyka";

#endif

// Create Cloud sockets
WiFiClient wclient;
PubSubClient mqtt_client(wclient);
///PubSubClient (server, port , [callback], client, [stream])

//=====================================================================

//Board D1 mini Wemos.cc

#define RELAY_OFF  LOW
#define RELAY_ON HIGH
#define OFF  LOW
#define ON HIGH


#define RELAY_A  D5
#define RELAY_B  D6 
#define STATUS_LED  D7 

// Status Led (blue)
// If it blinks fast: Unable to connect to Wifi network
// If it blinks every 2 seconds: Unable to find Broker 
// If it Stays on device ready to operate  


// WiFi credentials
char ssid[] = "IOT_WIFI";
char password[] = "GEDA2016";   // Set password to "" for open networks.


#define DEBUG_OFF  0
#define DEBUG_ON   1

#define debug DEBUG_ON

#define debug_message(fmt,...)          \
  do {              \
    if (debug)          \
       Serial.printf (fmt, ##__VA_ARGS__);     \
  } while(0)



void setup_wifi(void);    
void setup_mqtt(void);


// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(9600, SERIAL_8N1);  // Serial Port Parameters
  // initialize digital pin 13 as an output.
  pinMode(RELAY_A , OUTPUT);
  pinMode(RELAY_B , OUTPUT);
   pinMode(STATUS_LED , OUTPUT);

  
  digitalWrite(RELAY_A, RELAY_OFF);
  digitalWrite(RELAY_B, RELAY_OFF);
  digitalWrite(STATUS_LED,OFF);
  
  
  
  delay(1000);
  setup_wifi();          // initialize WIFI an connect to network
  setup_mqtt();          // initialize mqtt server 

 
  
}

// the loop function runs over and over again forever
void loop() {


  if (!mqtt_client.connected()) {
      reconnect();
      
 }

  mqtt_client.loop();  //This should be called regularly to allow the client to process incoming messages and maintain its connection to the server

 
}


void setup_wifi(void) {

// Start connecting to a WiFi network
 
  debug_message("\n\nConnecting to %s\n",ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    
    digitalWrite(STATUS_LED,OFF);
    delay(250);
    debug_message(".");
    digitalWrite(STATUS_LED,ON);
    delay(250);
  }

  digitalWrite(STATUS_LED,OFF);

  debug_message("\nWiFi Connected!!  IP Address:%s \n\n",WiFi.localIP().toString().c_str() );

  
}


void setup_mqtt(void) {
  
//http://pubsubclient.knolleary.net/api.html   (Arduino mqtt API)

 mqtt_client.setServer(MqttServer, MqttPort);
 mqtt_client.setCallback(callback);
}


void callback(char* topic, byte* payload, unsigned int length) {

 payload[length]=0; // terminator
  
 debug_message("Message arrived [ Topic:%s Length:%d Payload: %s ] \n",topic,length,payload);

 ParseTopic(topic,payload,length);

   

}

void ParseTopic(char* topic, byte* payload, unsigned int length)

{

// ============================== PARSER ==================================================

  if(!strcmp(topic,"bigbox/RelayA"))  
  {

      printf("RelA:%s\n", payload);

      if(payload[0]=='0')
        digitalWrite(RELAY_A, RELAY_OFF);
      if(payload[0]=='1')
        digitalWrite(RELAY_A, RELAY_ON);

  }

  if(!strcmp(topic,"bigbox/RelayB"))  
  {
       printf("RelB:%s\n", payload);

      if(payload[0]=='0')
        digitalWrite(RELAY_B, RELAY_OFF);
      if(payload[0]=='1')
        digitalWrite(RELAY_B, RELAY_ON);


  }


        
}






void reconnect() {

  while (!mqtt_client.connected())      // Loop until we're reconnected
  {
      debug_message("Attempting MQTT connection...");
      // Attempt to connect
 
 
      if (mqtt_client.connect(MqttClientID,MqttUser,MqttPassword))
      {
            debug_message("connected \r\n");
  
  
            // ... and subscribe to topic
           
            mqtt_client.subscribe("bigbox/RelayA");
            mqtt_client.subscribe("bigbox/RelayB");
            digitalWrite(STATUS_LED,ON);

      }
      else
      {
            debug_message("failed, rc=");       //Verrrrrr
            debug_message("%s",mqtt_client.state());
            debug_message(" try again in 3 seconds \r\n");
  
      
            delay(2000);  //Wait 2 seconds before retrying
            digitalWrite(STATUS_LED,ON);
            delay(200);  //Short 200 ms blink
            digitalWrite(STATUS_LED,OFF);

            
      }
      
  } // end of while
 
}


