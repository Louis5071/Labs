#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h> // I2C library
#include <ArduinoJson.h>

const char* ssid = "SampleWIFI"; 
const char* password = "*******";  

const char* serverName = "http://3.134.97.57:5000";

//constants for the pins
const int greenLED = 12;
const int redLED = 13;
const int Button = 15;
const int ReedSwitch = 2;
const int PIR = 17;
const int PIRLED = 27;



// variables
unsigned long startTime;
int buttonState = 0;



void setup() {
  delay(3000);
  startTime = millis();

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(Button, INPUT);
  pinMode(ReedSwitch, INPUT);
  pinMode(PIR, INPUT);
  pinMode(PIRLED, OUTPUT);
  
  // Test Serial Monitor
  Serial.begin(115200);
  Serial.println("Programm Start");
 
  /*// Setup DHT20 Sensor
  Serial.println("DHT20 Sensor test!");
  Wire1.begin(21, 22);  //  select your pin numbers here
  if (!DHT.begin()) {
    Serial.println("Could not find a valid DHT20 sensor, check wiring!");
    while (1);  
  }
  Serial.println("DHT20 sensor initialized successfully!");*/


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

// Send sensor data to AWS Cloud via HTTP POST
void sendSensorData(bool doorState, bool motion, bool buttonRequest) {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(serverName) + "/update";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // Create JSON document with sensor data
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["doorState"] = doorState;
    jsonDoc["motion"] = motion;
    jsonDoc["buttonRequest"] = buttonRequest;
    String requestBody;
    serializeJson(jsonDoc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    if(httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.print("Error in POST, code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

// Get occupancy state from AWS Cloud via HTTP GET and update the LED indicators
void getOccupancyStatus() {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(serverName) + "/status";
    http.begin(url);
    int httpResponseCode = http.GET();
    if(httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Status response: ");
      Serial.println(response);
      
      // Parse JSON response from AWS Cloud
      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, response);
      if(!error) {
        const char* occupancy = jsonDoc["occupancy"];
        // Update LEDs based on occupancy status
        if(strcmp(occupancy, "occupied") == 0) {
          digitalWrite(greenLED, LOW); // Turn off green LED
          digitalWrite(redLED, HIGH);  // Turn on red LED
        } else if(strcmp(occupancy, "vacant") == 0) {
          digitalWrite(redLED, LOW);   // Turn off red LED
          digitalWrite(greenLED, HIGH);  // Turn on green LED
        }
      }
    } else {
      Serial.print("GET request failed, code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}



void loop() {
   // Read the sensor values
   bool doorState = digitalRead(ReedSwitch);   // Read magnetic door sensor
   bool motion = digitalRead(PIR);         // Read PIR motion sensor

   // Read button state (active HIGH)
   buttonState= (digitalRead(Button) == HIGH);
 
    // Debugging output
   if(motion) {
     digitalWrite(PIRLED, HIGH); // Turn on PIR LED
   } else {
     digitalWrite(PIRLED, LOW);  // Turn off PIR LED
   }

   
   // Send the sensor data to the AWS Cloud server
   sendSensorData(doorState, motion, buttonState);
 
   // Retrieve occupancy status from the server and update LEDs accordingly
   getOccupancyStatus();
 
   // Wait before next cycle (for example, 5 seconds)
   delay(1000);
  
}

/*
// read the state of the pushbutton value
  buttonState = digitalRead(PIR);
  Serial.println(buttonState);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH
  if (buttonState == HIGH) {
  // turn LED on
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH); 
  }
  else {
  // turn LED off
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, LOW);
  }


*/