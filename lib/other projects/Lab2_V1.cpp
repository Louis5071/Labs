#include <Arduino.h>
#include <Adafruit_CAP1188.h> // Touch Sensor library
#include <Wire.h> // I2C library
//#include <pwmWrite.h> // PWM library

// Touch Sensor - Defalut I2C Address: 0x29
Adafruit_CAP1188 cap = Adafruit_CAP1188(); // Create an instance of CAP1188 using I2C
//#define sensitivity 0x3F; // 7 Sensitivity steps: 0x0F (128x) ... 0x7F (1x)


//constants for the pins where sensors are plugged into.
const int greenLED = 12;
const int yellowLED = 13;
const int redLED = 15;
const int Buzzer = 2; // PWM Channel

// Define Buzzer Frequencies
const int BUZZER_FREQ = 200;  // 2 kHz tone for sound

// Timing Variables
unsigned long lastChangeTime = 0;
unsigned long buzzerTimer = 0;
bool buzzerState = false;

// variables
unsigned long startTime;


// Function definitions
void switchtoGreen(){
  digitalWrite(redLED, HIGH);
  delay(3000);
  digitalWrite(yellowLED, HIGH);
  delay(2000);
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, HIGH);

}

void switchcycle(){
  delay(5000);
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, HIGH);
  delay(2000);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, HIGH);
  delay(10000);
  digitalWrite(yellowLED, HIGH);
  delay(2000);
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, HIGH);
}

// Function to control buzzer oscillation
void controlBuzzer(int onTime, int offTime) {
  unsigned long currentMillis = millis();

  if (buzzerState && currentMillis - buzzerTimer >= onTime) {
      ledcWriteTone(0, 0);  // Stop buzzer
      buzzerState = false;
      buzzerTimer = currentMillis;
  } else if (!buzzerState && currentMillis - buzzerTimer >= offTime) {
      ledcWriteTone(0, BUZZER_FREQ);  // Start buzzer
      buzzerState = true;
      buzzerTimer = currentMillis;
  }
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
  
  switchtoGreen();

}

void loop() {
  
  //ledcWrite(0, 100); // 50% duty cycle
  
  // Read CAP1188 Touch Sensor
  uint8_t touched = cap.touched();
  if (touched & 0x01) {  // Check if button 1 is pressed
      switchcycle();
  }

 //touchAttachInterrupt(touchButton, handleTouch, 40);

}





  