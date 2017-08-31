#include <PinChangeInt.h>
#include <PinChangeIntConfig.h>

#include <eHealth.h>
#include <eHealthDisplay.h>


#include <SPI.h>

#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>


#include <PinChangeInt.h>
#include <PinChangeIntConfig.h>

#include <eHealth.h>

#include <eHealthDisplay.h>



byte mac[]     = { 0xD4, 0xA8, 0xE2, 0xFE, 0xA0, 0xA1 }; // Must be unique on local network
String writeAPIKey = "G420BHANV9PJYGHD";    // Write API Key for a ThingSpeak Channel
const int updateInterval = (16 * 1000);        // Time interval in milliseconds to update ThingSpeak   

// Variable Setup
long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;

EthernetClient client;
int cont = 0;

void setup() {
  
  byte ip[]      = { 192,168,0,6 };                // Must be unique on local network
byte gateway[] = { 192,168,0,1};
byte subnet[]  = { 255, 255, 255, 0 };
  Serial.begin(115200);
  Ethernet.begin(mac, ip, gateway, subnet);
  delay(1000);
  Serial.print("ETHERNET SHIELD ip  is     : ");
  Serial.println(Ethernet.localIP()); 
// Start Ethernet on Arduino
  startEthernet();
  
  
  eHealth.initPulsioximeter();

  //Attach the inttruptions for using the pulsioximeter.
  PCintPort::attachInterrupt(6, readPulsioximeter, RISING);
}

void loop() {
  
  // Print Update Response to Serial Monitor
      if (client.available()){
            char c = client.read();
            Serial.print(c);
      }
  
      // Disconnect from ThingSpeak
      if (!client.connected() && lastConnected){
            Serial.println();
            Serial.println("...disconnected.");
            Serial.println();
            client.stop();
      }
  
      // Update ThingSpeak
      if(!client.connected() && (millis() - lastConnectionTime > updateInterval)){
            int ecg = eHealth.getECG();
            Serial.println("ECG "+ecg);
            updateThingSpeak("api_key="+String(writeAPIKey)+"&field6="+String(ecg));
//            int spo2 = eHealth.getOxygenSaturation();
//            Serial.println("SPO2 "+spo2);
//            updateThingSpeak("api_key="+String(writeAPIKey)+"&field7="+String(spo2));
            int bpm = eHealth.getBPM();
            Serial.println("BPM "+bpm);
            updateThingSpeak("api_key="+String(writeAPIKey)+"&field8="+String(bpm));
      }
      lastConnected = client.connected(); 
      delay(2000);
}


//Include always this code when using the pulsioximeter sensor
//=========================================================================
void readPulsioximeter(){

  cont ++;

  if (cont == 50) { //Get only of one 50 measures to reduce the latency
    eHealth.readPulsioximeter();
    cont = 0;
  }
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
      }else{
            failedCounter++;
            Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
            Serial.println();
            lastConnectionTime = millis(); 
      }
}

void startEthernet(){
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



