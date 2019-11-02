/*
  WriteMultipleFields
  
  Description: Writes values to fields 1,2,3 in a single ThingSpeak update every minute.
  
  Hardware: ESP8266 based boards
  
  !!! IMPORTANT - Modify the secrets.h file for this project with your network connection and ThingSpeak channel details. !!!
  
  Note:
  - Requires ESP8266WiFi library and ESP8622 board add-on. See https://github.com/esp8266/Arduino for details.
  - Select the target hardware from the Tools->Board menu
  - This example is written for a network using WPA encryption. For WEP or WPA, change the WiFi.begin() call accordingly.
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize, and 
  analyze live data streams in the cloud. Visit https://www.thingspeak.com to sign up for a free account and create a channel.  
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the README.md folder where the library was installed.
  See https://www.mathworks.com/help/thingspeak/index.html for the full ThingSpeak documentation.
  
  For licensing information, see the accompanying license file.
  
  Copyright 2018, The MathWorks, Inc.
*/

#include "ThingSpeak.h"
#include "secrets.h"
#include <ESP8266WiFi.h>

#define tempPin 0           // This is D3
#define selectLSBPin 2      // This is D4
#define humidPin 4          // This is D2
#define LDRPin 5            // This is D1
#define keyOpenPin 12       // This is D6
#define keyClosePin 13      // This is D7
#define selectMSBPin 14     // This is D5
#define ADCPin A0

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
int sensorValue;
float temp;
float humid;
float bright;

void setup() {
  Serial.begin(115200);  // Initialize serial
  pinMode(tempPin, OUTPUT);
  pinMode(humidPin, OUTPUT);
  pinMode(LDRPin, OUTPUT);
  pinMode(selectLSBPin, OUTPUT);
  pinMode(selectMSBPin, OUTPUT);
  pinMode(keyOpenPin, OUTPUT);
  pinMode(keyClosePin, OUTPUT);
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  int  flagToBreakConnect = 0; // Re-init flag
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED && flagToBreakConnect <= 60){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);            // Wait 5 seconds
      flagToBreakConnect++;   // This is necesary in case of  blackout
    } 
    if(flagToBreakConnect == 60){ // If pass 300 seconds don´t connect
      Serial.println("\nThe system can't connect please check your router configurations"); 
    }
    else{
      Serial.println("\nConnected."); 
    }
  }

  //Turn on the brightness sensor
  digitalWrite(LDRPin, HIGH);
  //Select brightness sensor on multiplexer
  digitalWrite(selectMSBPin, LOW);
  digitalWrite(selectLSBPin, LOW);
  //This is to ensure, the propagation times, don't matter lose three second
  delay(3000);
  // I got the brightness and save 
  sensorValue = analogRead(ADCPin);
  bright = map(sensorValue,0,1023,0,100);
  // Turn off the brightness sensor feed
  digitalWrite(LDRPin, LOW);
  // Debug by serial
  Serial.print("Brightness = ");
  Serial.println(bright);

  //Turn on the humidity sensor
  digitalWrite(humidPin, HIGH);
  //Select humidity sensor on multiplexer
  digitalWrite(selectMSBPin, HIGH);
  //This is to ensure, the propagation times, don't matter lose three seconds
  delay(3000);
  // I got the humidity and save 
  sensorValue = analogRead(ADCPin);
  humid = map(sensorValue,937,430,0,100);
  // Turn off the humidity sensor feed
  digitalWrite(humidPin, LOW);
  // Debug by serial
  Serial.print("Humidity = ");
  Serial.println(humid);

  //Turn on the temperature sensor
  digitalWrite(tempPin, HIGH);
  //Select temperature sensor on multiplexer
  digitalWrite(selectMSBPin, HIGH);
  digitalWrite(selectLSBPin, HIGH);
  //This is to ensure, the propagation times, don't matter lose three seconds
  delay(3000);
  // I got the temperature and save
  sensorValue = analogRead(ADCPin); 
  temp = map(sensorValue,0,1024,0,40);// 40 is The full scale value of temperature that can register
  // Turn off the temperature sensor feed and multiplexer 
  digitalWrite(tempPin, LOW);
  digitalWrite(selectMSBPin, LOW);
  digitalWrite(selectLSBPin, LOW);
  // Debug by serial
  Serial.print("Temperature = ");
  Serial.println(temp);

  controlKey(humid);
  
  // set the fields with the values
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, humid);
  ThingSpeak.setField(3, bright);

  // write to the ThingSpeak channel
  if(humid <= 100){   // This is necesary because sometimes the humid sensor reads a bad value 
      int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
     
      if(x == 200){ 
        Serial.println("Channel update successful.");
      }
      else{
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
    }
    delay(10000); // Wait ten seconds to update the channel again
  }
  

void controlKey(float hum){
  if(hum < 50){ // This is a arbitrary value because the humid parameter depends on the type of crop
      //Turn on the humidity sensor
      digitalWrite(humidPin, HIGH);   
      //Select humidity sensor on multiplexer
      digitalWrite(selectMSBPin, HIGH);
      while(hum<80){
      //This is to ensure, the propagation times, don't matter lose five seconds
      delay(5000);                  
      // I got the humidity and save 
      sensorValue = analogRead(ADCPin);
      hum = map(sensorValue,937,430,0,100);
      // Turn off the humidity sensor feed
      // Debug by serial
      Serial.print("Humidity = ");
      Serial.println(hum);
      //Open key
      digitalWrite(keyOpenPin, HIGH);
    }
    digitalWrite(humidPin, LOW);
    digitalWrite(keyOpenPin, LOW);
    digitalWrite(keyClosePin, HIGH);
    delay(10000);                   // This is the the necesary time to close the valve (ten seconds)
    digitalWrite(keyClosePin, LOW);
  }
}
