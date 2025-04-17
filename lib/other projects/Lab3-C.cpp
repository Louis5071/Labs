// Link to the Google Drive:
// https://drive.google.com/drive/folders/1QxGruO2aD6xiSUlQN3SufxeVR1aypMW0?usp=sharing

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h> // I2C library
#include "DHT20.h"

const char* ssid = "EuropeanWIFI"; 
const char* password = "Frenchroom6969";  

const char* serverName = "http://3.134.97.57:5000/data";

DHT20 DHT(&Wire1);  //  2nd I2C interface

// variables
unsigned long startTime;

void setup() {
  delay(3000);
  startTime = millis();
  
  // Test Serial Monitor
  Serial.begin(115200);
  Serial.println("Programm Start");
 
  // Setup DHT20 Sensor
  Serial.println("DHT20 Sensor test!");
  Wire1.begin(21, 22);  //  select your pin numbers here
  if (!DHT.begin()) {
    Serial.println("Could not find a valid DHT20 sensor, check wiring!");
    while (1);  
  }
  Serial.println("DHT20 sensor initialized successfully!");


  // WiFi connection
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
  Serial.println();
  Serial.println("Connected to Wi-Fi");
  Serial.println();

  
}



void loop() {
   
   //  READ DATA from DHT20
   int status = DHT.read(); // DHT20_OK should be 0
   float temperature = DHT.getTemperature();
  float humidity = DHT.getHumidity();

   //  DISPLAY DATA, sensor has only one decimal.
   Serial.print(humidity, 1);
   Serial.print("%,\t");
   Serial.print(temperature, 1);
   Serial.println("°C");
  
  if (WiFi.status() == WL_CONNECTED) { // Check if Wi-Fi is connected
        
      HTTPClient http;
      http.begin(serverName); // Connect to Flask server
      http.addHeader("Content-Type", "application/json");

      String jsonData = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";
      int httpResponseCode = http.POST(jsonData);

      Serial.printf("Server Response Code: %d\n", httpResponseCode);
      Serial.println(http.getString());

      http.end();

    } else {
        Serial.println("Wi-Fi Disconnected");
    }

    delay(5000); // Wait 5 seconds before sending next request
}