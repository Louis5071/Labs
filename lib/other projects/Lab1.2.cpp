#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h> // SPI library
#include <Adafruit_CAP1188.h> // Touch Sensor library
#include <Wire.h> // I2C library

// Display
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// Touch Sensor - Defalut I2C Address: 0x29
Adafruit_CAP1188 cap = Adafruit_CAP1188(); // Create an instance of CAP1188 using I2C
//#define sensitivity 0x3F; // 7 Sensitivity steps: 0x0F (128x) ... 0x7F (1x)

// constants


// variables
int touchCount = 0; // Counter for touch events


void setup() {
  delay(3000);
  
  // Test Serial Monitor
  Serial.begin(115200);
  Serial.println("Programm Start");

  
  // Setup and test display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  

  // Setup Touchsensor - 0x29 is the default address
  Serial.println("CAP1188 test!");

  if (!cap.begin(0x29)) {
    Serial.println("CAP1188 not found");
    while (1);
  }
  Serial.println("CAP1188 found!");

}



void loop() {

  // Read the touch sensor status
  uint8_t touched = cap.touched();

  
    for (int i = 0; i < 8; i++) {
        if (touched & (1 << i)) {
            touchCount++; // Increment touch count
        }
    }
    
    Serial.println();
    Serial.print("Touch Count: ");
    Serial.println(touchCount);

    tft.drawNumber(touchCount, 110, 40, 8);
  


touchCount = 0; // Reset touch count
delay(200);
}


  