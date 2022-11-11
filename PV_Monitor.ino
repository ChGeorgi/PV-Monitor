#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <time.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "usergraphics.h"
String newHostname = "PV-Display";
      String  topic_str="/"+newHostname+"/Display/#";
       const char* subscribe_topic = topic_str.c_str();
WiFiClient espClient;
PubSubClient client_MQTT(espClient);
String SOH="";
String PVData[20];  // Array recepcion valores MQTT // Array reception MQTT values
long myTimeout=10000;
long myTimer=0;
String show="1";
/*______End of Libraries_______*/
const char* mqtt_server = "192.168.178.28";
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];



/*______Define LCD pins for ArduiTouch _______*/
#define TFT_CS   D1
#define TFT_DC   D2
#define TFT_LED  15  


#define HAVE_TOUCHPAD
#define TOUCH_CS 0
#define TOUCH_IRQ 2
// #define touch_yellow_header  // enable this line for TFTs with yellow header
/*_______End of defanitions______*/


/*____Calibrate TFT LCD_____*/
#define TS_MINX 370
#define TS_MINY 470
#define TS_MAXX 3700
#define TS_MAXY 3600
#define MINPRESSURE 10
#define MAXPRESSURE 2000 
/*______End of Calibration______*/



/*____Program specific constants_____*/
enum { PM_MAIN, PM_OPTION, PM_CLEANING};  // Program modes

/*______End of specific constants______*/
int FB_Follower=0;
int IG_Follower=0;
long YT_Subscriber=0;
long YT_Videos=0;
long YT_Views=0;


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);


//WiFiClientSecure secureClient;


// #define _debug
 int X,Y;
 int Timer_Backlight = 255; 
 uint8_t PMode = PM_MAIN;         // program mode
 bool Touch_pressed = false;
 uint8_t Timer_Cleaning=0;
 


void setup() {
  
  Serial.begin(9600); 
    pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);    // HIGH to Turn on;
  // digitalWrite(TFT_LED, LOW);  // LOW to Turn on - with assembled R2 and T1

  tft.begin();
  touch.begin();
  #ifdef _debug
  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
  #endif

  Serial.println("Starting...");
  String newHostname = "PV-Display";
  WiFi.hostname(newHostname.c_str());
  WiFiManager wifiManager;
  //wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();
  
  //Serial.println("Setting Time");
  //setTime();
   Serial.println("Connect to MQTT"); 
   client_MQTT.setServer(mqtt_server, 1883);
  client_MQTT.setCallback(callback);
  if (client_MQTT.connect("ESP8266Client")) {
       Serial.println("MQTT Online");     

      //// Topico para recibir Holding Registers desde MQTT // Topic to receive Holding Registers from MQTT --- return data in callback funtion
      client_MQTT.subscribe(subscribe_topic);
               
    } else {
      Serial.print("failed, rc=");
      Serial.print(client_MQTT.state());
    
    }
Serial.println("-----------"); 

  tft.setRotation(1); 

    
    String  topic_str="/"+newHostname+"/Display/Status";
       const char* topic = topic_str.c_str();
   client_MQTT.subscribe(subscribe_topic);
   delay(100);
   client_MQTT.publish(topic,"Start");
   draw_screen();
}


TS_Point p;

void loop() { 
 



   if (millis()> myTimeout+myTimer)
   {myTimer=millis();
   if (show=="1"){
   draw_screen();
   show="2";
   }
   else if (show=="2"){
   draw_screen2();
   show="1";
   }
   }
      
      
if (!client_MQTT.connected()) {
            
         reconnect();         /// reconection MQTT
        }                        
       client_MQTT.loop();
}




void draw_screen()
{

  tft.fillScreen(ILI9341_BLACK);
  tft.drawRGBBitmap(0,10, PV,88,78);
  tft.drawRGBBitmap(10,90, solar,71,74);
    tft.drawRGBBitmap(5,165, deliver,71,61);
  tft.setTextSize(2);
  int row= 30;
  int rowheight=30;
  int start= 80;
  int startdata =225;
  tft.setCursor(start, row);
  tft.print("SOH:               %"); 
  tft.setCursor(startdata, row);
  //tft.setTextSize(4);
 // tft.print("99");
  tft.print(PVData[0]);
  row=row+rowheight;
  tft.setCursor(start, row); 
  tft.print("SOC:               %"); 
  tft.setCursor(startdata, row);
  tft.print(PVData[1]);
  row=row+rowheight;
  tft.setCursor(start, row);
  tft.print("Produziert       kwh");
  tft.setCursor(startdata, row);
 tft.print(PVData[2]);
  row=row+rowheight;
  tft.setCursor(start, row);
   tft.print("Verbraucht:      kwh");  
  tft.setCursor(startdata, row);
  tft.print(PVData[6]);
  row=row+rowheight;
  tft.setCursor(start, row);
  tft.print("Bezogen:         kwh");  
  tft.setCursor(startdata, row);
  tft.print(PVData[3]);
  row=row+rowheight;
  tft.setCursor(start, row);
   tft.print("Eingespeist:     kwh"); 
  tft.setCursor(startdata, row);
 tft.print(PVData[4]);
}

void draw_screen2()
{
  tft.fillScreen(ILI9341_BLACK);
  tft.drawRGBBitmap(0,10, PV,88,78);
  tft.drawRGBBitmap(10,90, solar,71,74);
    tft.drawRGBBitmap(5,165, deliver,71,61);
  tft.setTextSize(2);
  int row= 30;
  int rowheight=30;
  int start= 80;
  int startdata =225;

  tft.setCursor(start, row);
       tft.print("Heute");
         tft.setCursor(startdata, row);
  tft.print(PVData[11]+"W");
       row=row+rowheight;
  tft.setCursor(start, row); 
  tft.print("SOC:               %"); 
  tft.setCursor(startdata, row);
  tft.print(PVData[1]); 
  row=row+rowheight;
  
  tft.setCursor(start, row);
  tft.print("Produziert:      kwh");
  tft.setCursor(startdata, row);
 tft.print(PVData[5]);
  
  row=row+rowheight;
  tft.setCursor(start, row);
   tft.print("Genutzt:         kwh");  
  tft.setCursor(startdata, row);
  tft.print(PVData[8]);
  row=row+rowheight;
  tft.setCursor(start, row);
   tft.print("Eingespeist:     kwh"); 
  tft.setCursor(startdata, row);
 tft.print(PVData[7]);
 row=row+rowheight;
  tft.setCursor(start, row);
   tft.print("Verbraucht:      kwh");  
  tft.setCursor(startdata, row);
  tft.print(PVData[9]);
  row=row+rowheight;
  
  tft.setCursor(start, row);
   tft.print("Bezogen:         kwh"); 
  tft.setCursor(startdata, row);
 tft.print(PVData[10]);
}



void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String string;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      string+=((char)payload[i]);
  }
  Serial.println();

if (topic = "/PV-Display/Display/PVData"){
   
   int cantidad=0 ; 
   PVData[cantidad]="";
    for (int i = 0; i < length; i++) {
                 if ( payload[i] == 44 )  // ","= 44 ascii // detecta separador "," //  Detect tab ","
                 {                    
                       cantidad++;  
                          PVData[cantidad]="";      
                 }
                 else
                 {
                   //Serial.println(cantidad);
                    PVData[cantidad]+=((char)payload[i]);
                   /// Serial.println((char)payload[i]);
                 }
    }
}   
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    //digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
  //  digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
client_MQTT.subscribe(subscribe_topic);
}
void reconnect() {  
  // Loop until we're reconnected  // Bucle hasta que se vuelva a conectar
  while (!client_MQTT.connected()) {    
  
    if (client_MQTT.connect("ESP8266Client")) {
      // Serial.println("MQTT Online");     
       
      //// Topico para recibir Holding Registers desde MQTT // Topic to receive Holding Registers from MQTT --- return data in callback funtion
      client_MQTT.subscribe(subscribe_topic);         
    } else {
      Serial.print("failed, rc=");
      Serial.print(client_MQTT.state());
    
    }
  }
}
