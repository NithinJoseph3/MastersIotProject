#include <SPI.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <MPU6050.h>
#include <I2Cdev.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <ThingSpeak.h>


byte mac[]     = { 0xD4, 0xA8, 0xE2, 0xFE, 0xA0, 0xA1 }; // Must be unique on local network
String writeAPIKey = "G420BHANV9PJYGHD";    // Write API Key for a ThingSpeak Channel
const int updateInterval = (16 * 1000);        // Time interval in milliseconds to update ThingSpeak   

  // Variable Setup
long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;

// Initialize Arduino Ethernet Client
EthernetClient client;

MPU6050 accelgyro;


bool blinkState = false;
int accelX = 0;
int accelY = 0;
int accelZ = 0;

void setup(void){
// Local Network Settings

byte ip[]      = { 192,168,0,6 };                // Must be unique on local network
byte gateway[] = { 192,168,0,1};
byte subnet[]  = { 255, 255, 255, 0 };
  Serial.begin(9600);
  Ethernet.begin(mac, ip, gateway, subnet);
  delay(1000);
  Serial.print("ETHERNET SHIELD ip  is     : ");
  Serial.println(Ethernet.localIP()); 
// Start Ethernet on Arduino
  startEthernet();
  //Setup the LEDS to act as outputs
  #define LED_PIN 13

   //ACCEL

//  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
//    #include "Wire.h"
//  #endif
// // join I2C bus (I2Cdev library doesn't do this automatically)
//  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
//        Wire.begin();
//  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
//        Fastwire::setup(400, true);
//  #endif
  // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
     pinMode(LED_PIN, OUTPUT);
}



void loop(){

// Print Update Response to Serial Monitor
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }
  
  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected)
  {
    Serial.println();
    Serial.println("...disconnected.");
    Serial.println();
    
    client.stop();
  }
  
  // Update ThingSpeak
  if(!client.connected() && (millis() - lastConnectionTime > updateInterval))
  {
      blinkState = !blinkState;
      digitalWrite(LED_PIN, blinkState);
      accelX = accelgyro.getXAccelOffset();
      updateThingSpeak("api_key="+String(writeAPIKey)+"&field3="+String(accelX));
       accelY = accelgyro.getYAccelOffset();
      updateThingSpeak("api_key="+String(writeAPIKey)+"&field4="+String(accelY));
      accelZ = accelgyro.getZAccelOffset();
      updateThingSpeak("api_key="+String(writeAPIKey)+"&field5="+String(accelZ));
  }
  
  lastConnected = client.connected();
}

void updateThingSpeak(String tsData)
{
      // ThingSpeak Settings
      char thingSpeakAddress[] = "api.thingspeak.com";

      if (client.connect(thingSpeakAddress, 80)){ 
          client.print("POST /update HTTP/1.1\n");
          client.print("Host: api.thingspeak.com\n");
          client.print("Connection: close\n");
          client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
          client.print("Content-Type: application/x-www-form-urlencoded\n");
          client.print("Content-Length: ");
          client.print(tsData.length());
          client.print("\n\n");
          client.print(tsData);
          lastConnectionTime = millis();
    
          if (client.connected()){
              Serial.println("Connecting to ThingSpeak...");
              Serial.println();
              failedCounter = 0;
          }else {
              failedCounter++;
              Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
              Serial.println();
          }
      }else {
            failedCounter++;
            Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
            Serial.println();
            lastConnectionTime = millis(); 
      }
}

void startEthernet()
{
  
  client.stop();
  Serial.println("Connecting Arduino to network...");
  Serial.println();  
  
  delay(1000);
  
  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0){
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  }else {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
    Serial.println("Data being uploaded to THINGSPEAK Server.......");
    Serial.println();
  }
  delay(1000);
}

