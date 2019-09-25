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
#include <Time.h>
#include <TimeLib.h>

time_t fecha;
#define tempPin 0
#define selectLSBPin 2
#define humidPin 4
#define LDRPin 5
#define keyOpenPin 12
#define keyClosePin 13
#define selectMSBPin 13
#define ADCPin = A0;

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;


unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
double temp;
double humid;
double bright;

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
      delay(5000);  
      flagToBreakConnect++;   // This is necesary in case of  blackout
    } 
    if(flagToBreakConnect == 60){
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
  //This is to ensure, the propagation times, don't matter lose one second
  delay(1000);
  // I got the brightness and save 
  bright = (analogRead(ADCPin)/1024)*100;
  // Turn off the brightness sensor feed
  digitalWrite(LDRPin, LOW);
  // Debug by serial
  Serial.print("Brightness = ");
  Serial.println(bright);

  //Turn on the humidity sensor
  digitalWrite(humidPin, HIGH);
  //Select humidity sensor on multiplexer
  digitalWrite(selectLSBPin, HIGH);
  //This is to ensure, the propagation times, don't matter lose one second
  delay(1000);
  // I got the humidity and save 
  humid = (analogRead(ADCPin)/1024)*100;
  // Turn off the humidity sensor feed
  digitalWrite(humidPin, LOW);
  // Debug by serial
  Serial.print("Humidity = ");
  Serial.println(humid);

  //Turn on the temperature sensor
  digitalWrite(tempPin, HIGH);
  //Select temperature sensor on multiplexer
  digitalWrite(selectMSBPin, HIGH);
  //This is to ensure, the propagation times, don't matter lose one second
  delay(1000);
  // I got the temperature and save 
  temp = (analogRead(ADCPin)/1024)*55;
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
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  delay(1000); // Wait 20 seconds to update the channel again
}

void controlKey(double hum){
  fecha = now();
  Serial.print("Time is : ");
  Serial.println(fecha);
  if(hour(fecha) == 6 || hour(fecha) == 12 ){
      while(hum<90){
      //Turn on the humidity sensor
      digitalWrite(humidPin, HIGH);
      //Select humidity sensor on multiplexer
      digitalWrite(selectLSBPin, HIGH);
      //This is to ensure, the propagation times, don't matter lose one second
      delay(1000);
      // I got the humidity and save 
      hum = (analogRead(ADCPin)/1024)*100;
      // Turn off the humidity sensor feed
      // Debug by serial
      Serial.print("Humidity = ");
      Serial.println(humid);
      //Open key
      digitalWrite(keyOpenPin, HIGH);
    }
    digitalWrite(humidPin, LOW);
    digitalWrite(keyOpenPin, LOW);
    digitalWrite(keyClosePin, HIGH);
    delay(10000)
    digitalWrite(keyClosePin, LOW);
  }
}
