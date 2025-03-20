#include <Arduino.h>
#include <Adafruit_CAP1188.h> // Touch Sensor library
#include <Wire.h> // I2C library

// Touch Sensor - Defalut I2C Address: 0x29
Adafruit_CAP1188 cap = Adafruit_CAP1188(); // Create an instance of CAP1188 using I2C
//#define sensitivity 0x3F; // 7 Sensitivity steps: 0x0F (128x) ... 0x7F (1x)


//constants for the pins where sensors are plugged into.
const int greenLED = 12;
const int yellowLED = 13;
const int redLED = 15;
const int Buzzer = 2; // PWM Channel

// Define Buzzer Frequencies
const int BUZZER_FREQ = 200;

// Timing Variables
unsigned long lastChangeTime = 0;
unsigned long ButtomPressedTime = 0;
unsigned long buzzerTimer = 0;
bool buzzerState = false;
bool buttonPressed = false;

// variables
unsigned long startTime;


// Traffic Light States
enum State { RED, RED_YELLOW, GREEN, YELLOW };
State currentState = RED;

// Function to control buzzer oscillation
void controlBuzzer(int onTime, int offTime) {
    unsigned long currentMillis = millis();

    if (buzzerState && currentMillis - buzzerTimer >= onTime) {
        ledcWriteTone(0, 0);  // Stop buzzer
        buzzerState = false;
        buzzerTimer = currentMillis;
    } else if (!buzzerState && currentMillis - buzzerTimer >= offTime) {
        ledcWriteTone(0, BUZZER_FREQ);  // Start buzzer at 2kHz
        buzzerState = true;
        buzzerTimer = currentMillis;
    }
}

// Function to change traffic light state
void changeState(State newState) {
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, LOW);
    ledcWriteTone(0, 0); // Ensure buzzer is off

    switch (newState) {
        case RED:
            digitalWrite(redLED, HIGH);
            lastChangeTime = millis();
            break;
        case RED_YELLOW:
            digitalWrite(redLED, HIGH);
            digitalWrite(yellowLED, HIGH);
            lastChangeTime = millis();
            break;
        case GREEN:
            digitalWrite(greenLED, HIGH);
            lastChangeTime = millis();
            break;
        case YELLOW:
            digitalWrite(yellowLED, HIGH);
            lastChangeTime = millis();
            break;
    }

    currentState = newState;
}


void setup() {
  delay(3000);
  
  // Test Serial Monitor
  Serial.begin(115200);
  Serial.println("Programm Start");

  // Setup Touchsensor
  Serial.println("CAP1188 test!");

  if (!cap.begin()) {
    Serial.println("CAP1188 not found");
    while (1);
  }
  Serial.println("CAP1188 found!");
  
  startTime = millis();

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(Buzzer, OUTPUT);

  // Configure ESP32 PWM for buzzer
  ledcSetup(0, BUZZER_FREQ, 8);  // Channel 0, 2kHz frequency, 8-bit resolution --> 0...256
  ledcAttachPin(Buzzer, 0); // Setup which PWM Channel is used ; assign PWM-Channel --> GPIO Pin
  
  changeState(RED);  // Start in RED state

}

void loop() {
  unsigned long currentMillis = millis();

  // Read CAP1188 Touch Sensor
  uint8_t touched = cap.touched();
  if (touched & 0x01) {  // Check if button 1 is pressed
      buttonPressed = true;
      ButtomPressedTime = currentMillis;
  }

  // Handle buzzer sound
  if (currentState == RED) {
    controlBuzzer(250, 250);  
  } else if (currentState == GREEN) {
    controlBuzzer(500, 1500);
  } else {
    ledcWriteTone(0, 0);
  }
  

  // Traffic Light Logic
  switch (currentState) {
      case RED:
          if (currentMillis - lastChangeTime >= 10000) {
              changeState(RED_YELLOW);
          }
          break;

      case RED_YELLOW:
          if (currentMillis - lastChangeTime >= 2000) {
              changeState(GREEN);
          }
          break;

      case GREEN:
          if (buttonPressed && (currentMillis - ButtomPressedTime >= 5000)) {
              buttonPressed = false;
              changeState(YELLOW);
          }
          break;

      case YELLOW:
          if (currentMillis - lastChangeTime >= 2000) {
              changeState(RED);
          }
          break;
  }
}





  