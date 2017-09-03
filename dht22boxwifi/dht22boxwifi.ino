 /**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 by Daniel Eichhorn
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

//======== Debug Tools =========
 
#define DEBUG_OFF  0
#define DEBUG_ON   1

#define debug DEBUG_ON

#define debug_message(fmt,...)          \
  do {              \
    if (debug)          \
       Serial.printf (fmt, ##__VA_ARGS__);     \
  } while(0)

//========================================

#include <ESP8266WiFi.h> 
#include <PubSubClient.h>     // http://knolleary.net/arduino-client-for-mqtt/ 

// WiFi credentials
char ssid[] = "IOT_WIFI";
char password[] = "GEDA2016";   // Set password to "" for open networks.
                  
                              
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
const char MqttClientID[]="climate";

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


// Create Wifi sockets
WiFiClient wifi_client;

// Create Cloud sockets
PubSubClient mqtt_client(wifi_client);


void callback(char* topic, byte* payload, unsigned int length);
 
//=======================================
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN            2         // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11 
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;             // Minimun delay between sensor readings based on sensor details (This is fixed and cant be changed its about 2 sec)
uint32_t Usr_delayMS=3000;    // Aditional delay may be changed by User (default 3sec). Total Delay between readings is delayMS + Usr_delayMS (default 5 sec)

// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
// or #include "SH1106.h" alis for `#include "SH1106Wire.h"`
// For a connection via I2C using brzo_i2c (must be installed) include
// #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Brzo.h"
// #include "SH1106Brzo.h"
// For a connection via SPI include
// #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Spi.h"
// #include "SH1106SPi.h"

// Include custom images
#include "images.h"

// Initialize the OLED display using SPI
// D5 -> CLK
// D7 -> MOSI (DOUT)
// D0 -> RES
// D2 -> DC
// D8 -> CS
// SSD1306Spi        display(D0, D2, D8);
// or
// SH1106Spi         display(D0, D2);

// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D5 -> SCL
// SSD1306Brzo display(0x3c, D3, D5);
// or
// SH1106Brzo  display(0x3c, D3, D5);

// Initialize the OLED display using Wire library
//SSD1306  display(0x3c, D3, D5);

#define SCL_PIN  D1
#define SDA_PIN  D2



SSD1306  display(0x3c, SDA_PIN, SCL_PIN);



#define DEMO_DURATION 3000
typedef void (*Demo)(void);

int demoMode = 0;
int counter = 1;


void start_dht(void);
void setup_wifi(void);
void setup_mqtt(void);

void medir(void);

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();


  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);


  start_dht();
  display.clear();
  drawImageDemo();
  display.display();

  setup_wifi();          // initialize WIFI an connect to network
  setup_mqtt();          // initialize mqtt server 
 
  delay(1000);

// clear the display
  display.clear();
  // draw the current demo method
  //demos[demoMode]();
  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);


  
}
long TimeSinceLastMeasure = 0;
unsigned char DisplaySensorEnable=1; // Show readings on display     

void loop() {
  
 // display.drawString(0, 0,String(millis()));

 
  if (!mqtt_client.connected()) {
        reconnect();
  }
 
  mqtt_client.loop();  //This should be called regularly to allow the client to process incoming messages and maintain its connection to the server


  //drawFontFaceDemo();
  
  


  if (millis() - TimeSinceLastMeasure > delayMS+Usr_delayMS) {   // Delay between measurements.
         
      medir();
           
      TimeSinceLastMeasure = millis();
  }

}





void start_dht(void)
{

 Serial.begin(9600); 
 // Initialize device.
  dht.begin();
  Serial.println("DHTxx Unified Sensor Example");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  debug_message("\n\ndelay between sensor readings based on sensor details %d ms\n",delayMS);
}

char str[20];

void medir(void) {
  // Delay between measurements.
  /////////////////////////////////////////delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event,event_T,event_H;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
     debug_message("Error reading temperature!\n");
  }
  else {
    debug_message("Temperature:  %s *C\n",String(event.temperature).c_str());
    event_T=event;
  }
  
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
  
    debug_message("Humidity:  %s %\n",String(event.relative_humidity).c_str());
    event_H=event;

    if(DisplaySensorEnable==1)
    {  
      display.clear();
      sprintf(str, "T: %s *C \nH: %s %",String(event_T.temperature).c_str(),String(event_H.relative_humidity).c_str()); 
      display.drawString(0, 0,str);
      display.display();
    }
    
    mqtt_client.publish("climate_box/Temp",String(event_T.temperature).c_str(),false);
    mqtt_client.publish("climate_box/Humedad",String(event_H.relative_humidity).c_str(),false);
    
  }
}




void setup_mqtt(void) {
  
//http://pubsubclient.knolleary.net/api.html   (Arduino mqtt API)

mqtt_client.setServer(MqttServer, MqttPort);
mqtt_client.setCallback(callback);
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

  display.clear();
  display.drawString(0, 0,(char*)nuevo.c_str());
  display.display(); // write buffer to display


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
            mqtt_client.subscribe("climate_box/display");
            mqtt_client.subscribe("climate_box/SetReadDelay");
            mqtt_client.subscribe("climate_box/ReadoutEnable");
   


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


void callback(char* topic, byte* payload, unsigned int length)
{

 payload[length]=0; // terminator
  
 debug_message("Message arrived [ Topic:%s Length:%d Payload: %s ] \n",topic,length,payload);

 ParseTopic(topic,payload,length);

}

void ParseTopic(char* topic, byte* payload, unsigned int length)

{
      if(!strcmp(topic,"climate_box/display"))  
      {

          display.clear();
          display.drawString(0, 0,(char*)payload);
          display.display(); // write buffer to display

      }

      if(!strcmp(topic,"climate_box/SetReadDelay"))  
      {

          Usr_delayMS=atoi((char*)payload);
          debug_message("Reading interval: %d\n",Usr_delayMS+delayMS);

      }

      if(!strcmp(topic,"climate_box/ReadoutEnable"))  
      {

        debug_message("Redaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");

          if(payload[0]=='1')
          {
            DisplaySensorEnable=1;
            display.clear();
            display.drawString(0, 0,"Readout Enabled");
            display.display(); // write buffer to display
            debug_message("Enabled\n");
          }
          if(payload[0]=='0') 
          {
            DisplaySensorEnable=0; 
            display.clear();
            display.drawString(0, 0,"Readout Disabled");
            display.display(); // write buffer to display
            debug_message("Disabled\n");
          }
      }
        



      
}

//////////////////// DISPLAY DEMOS //////////////////////////


void drawFontFaceDemo() {
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Hello world");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, "Hello world");
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 26, "Hello world");
}

void drawTextFlowDemo() {
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawStringMaxWidth(0, 0, 128,
      "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore." );
}

void drawTextAlignmentDemo() {
    // Text alignment demo
  display.setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 22, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 33, "Right aligned (128,33)");
}

void drawRectDemo() {
      // Draw a pixel at given position
    for (int i = 0; i < 10; i++) {
      display.setPixel(i, i);
      display.setPixel(10 - i, i);
    }
    display.drawRect(12, 12, 20, 20);

    // Fill the rectangle
    display.fillRect(14, 14, 17, 17);

    // Draw a line horizontally
    display.drawHorizontalLine(0, 40, 20);

    // Draw a line horizontally
    display.drawVerticalLine(40, 0, 20);
}

void drawCircleDemo() {
  for (int i=1; i < 8; i++) {
    display.setColor(WHITE);
    display.drawCircle(32, 32, i*3);
    if (i % 2 == 0) {
      display.setColor(BLACK);
    }
    display.fillCircle(96, 32, 32 - i* 3);
  }
}

void drawProgressBarDemo() {
  int progress = (counter / 5) % 100;
  // draw the progress bar
  display.drawProgressBar(0, 32, 120, 10, progress);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, String(progress) + "%");
}

void drawImageDemo() {
    // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
    // on how to create xbm files
    display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}




