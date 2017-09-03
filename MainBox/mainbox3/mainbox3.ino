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
const char MqttClientID[]="mainbox";

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

#define RELAY_ON  LOW
#define RELAY_OFF HIGH
#define RELAY_A  D5
#define RELAY_B  D6 
#define BUTTON_A D7
#define BUTTON_B D8

#define NEOPIXDEL_PIN  2  // (GPIO)  D4
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      1

enum b_state {NO_EDGE,RISING_EDGE,FALLING_EDGE};
typedef enum b_state button_state;
button_state  b1;

button_state edge_detect_A(void);
button_state level_detect_A(void);


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




// Declare OLED display
// display(SDA, SCL);
// SDA and SCL are the GPIO pins of ESP8266 that are connected to respective pins of display.
// OLED::OLED(uint8_t sda, uint8_t scl, uint8_t address, uint8_t offset) //ojo usar GPIOS no Dx 

OLED display(4, 5,0x3c,2);


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXDEL_PIN, NEO_GRB + NEO_KHZ800);


void setup_wifi(void);    
void setup_mqtt(void);


// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(9600, SERIAL_8N1);  // Serial Port Parameters
  // initialize digital pin 13 as an output.
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);

  pinMode(BUTTON_A, INPUT);
  pinMode(BUTTON_B, INPUT);
  digitalWrite(RELAY_A, RELAY_OFF);
  digitalWrite(RELAY_B, RELAY_OFF);

  pixels.begin(); // This initializes the NeoPixel library.
 
  
  display.begin();

  display.print("GEDA IOT",4,4);
 // neopix();


  pixels.setPixelColor(0, pixels.Color(40,0,0)); // Moderately bright green color.
  pixels.show(); // This sends the updated pixel color to the hardware.
  
  
  delay(1000);
  setup_wifi();          // initialize WIFI an connect to network
  setup_mqtt();          // initialize mqtt server 

 pixels.setPixelColor(0, pixels.Color(0,40,0)); // Moderately bright green color.
 pixels.show(); // This sends the updated pixel color to the hardware.
  
}

// the loop function runs over and over again forever
void loop() {


  if (!mqtt_client.connected()) {
      reconnect();
      
 }

  mqtt_client.loop();  //This should be called regularly to allow the client to process incoming messages and maintain its connection to the server

   //level_detect_A();
   edge_detect_A();
   edge_detect_B();
  
}



button_state edge_detect_A(void)
{


   static unsigned char previous_value=0;
   unsigned char present_value;

          present_value=digitalRead(BUTTON_A);

          if((present_value^previous_value)&present_value) //rising edge
          {

             // print_display(" 1 ");
             // digitalWrite(RELAY_A, RELAY_ON);
              mqtt_client.publish("box1/switch_A","1",false);
              
            
          }

          if((present_value^previous_value)&!present_value) //falling edge
          {

             // print_display(" 0 ");
             // digitalWrite(RELAY_A, RELAY_OFF);
              mqtt_client.publish("box1/switch_A","0",false);
              
            
          }

          previous_value=present_value;




      
  
}
button_state edge_detect_B(void)
{


   static unsigned char previous_value=0;
   unsigned char present_value;

          present_value=digitalRead(BUTTON_B);

          if((present_value^previous_value)&present_value) //rising edge
          {

             // print_display(" 1 ");
             // digitalWrite(RELAY_B, RELAY_ON);
              mqtt_client.publish("box1/switch_B","1",false);

              
            
          }

          if((present_value^previous_value)&!present_value) //falling edge
          {

             // print_display(" 0 ");
             // digitalWrite(RELAY_B, RELAY_OFF);
              mqtt_client.publish("box1/switch_B","0",false);
              
            
          }

          previous_value=present_value;

     
  
}





void setup_wifi(void) {

// Start connecting to a WiFi network
 
  debug_message("\n\nConnecting to %s\n",ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debug_message(".");
  }

  debug_message("\nWiFi Connected!!  IP Address:%s \n\n",WiFi.localIP().toString().c_str() );

  String myIP("IP:");

  String nuevo=myIP+WiFi.localIP().toString().c_str();

  //display.print((char*)WiFi.localIP().toString().c_str());

  display.print((char*)nuevo.c_str());

}

void neopix(void)
{
for(int i=0;i<NUMPIXELS;i++){

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,40,20 )); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(50); // Delay for a period of time (in milliseconds).

  }
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

// ===============================Led  PARSER ==================================================

  if(!strcmp(topic,"box1/rgbled"))  //Sample Payload: rgb(79, 176, 80)

  {


    int rgb[3];
    int i;
    char *pt;
    
    
        pt = strtok ((char*)payload+4,",");
        while (pt != NULL) {
            rgb[i] = atoi(pt);
            printf("%d\n", rgb[i++]);
            pt = strtok (NULL, ",");
        }
    
        pixels.setPixelColor(0, pixels.Color(rgb[0],rgb[1],rgb[2])); // Moderately bright green color.
        pixels.show(); // This sends the updated pixel color to the hardware.

  }


  if(!strcmp(topic,"box1/display"))  //Sample Payload: rgb(79, 176, 80)
  {

       display.clear();
       display.print((char*)payload,4,4);


  }

  if(!strcmp(topic,"box1/RelayA"))  //Sample Payload: rgb(79, 176, 80)
  {


      printf("RelA:%s\n", payload);

      if(payload[0]=='0')
        digitalWrite(RELAY_A, RELAY_OFF);
      if(payload[0]=='1')
        digitalWrite(RELAY_A, RELAY_ON);
      
  }


  if(!strcmp(topic,"box1/RelayB"))  //Sample Payload: rgb(79, 176, 80)
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
            mqtt_client.subscribe("box1/rgbled");
            mqtt_client.subscribe("box1/display");
            mqtt_client.subscribe("box1/RelayA");
            mqtt_client.subscribe("box1/RelayB");


      }
      else
      {
            debug_message("failed, rc=");       //Verrrrrr
            debug_message("%s",mqtt_client.state());
            debug_message(" try again in 3 seconds \r\n");
  
      
            delay(3000);  //Wait 3 seconds before retrying
      }
      
  } // end of while
 
}


