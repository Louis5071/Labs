#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h> // SPI library
#include <Adafruit_CAP1188.h> // Touch Sensor library
#include <Wire.h> // I2C library

// Define Software SPI Pins
#define CAP1188_CS   25   // Chip Select (CS)
#define CAP1188_CLK  26  // Clock (SCK)
#define CAP1188_MISO 32  // Master In, Slave Out (MISO)
#define CAP1188_MOSI 33  // Master Out, Slave In (MOSI)
#define CAP1188_RESET 27  // Master Out, Slave In (MOSI)


// Display
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// Touch Sensor
// Create an instance of CAP1188 using Software SPI
Adafruit_CAP1188 cap = Adafruit_CAP1188(CAP1188_CLK, CAP1188_MISO, CAP1188_MOSI, CAP1188_CS, CAP1188_RESET);
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
  
  // Setup Touchsensor
  Serial.println("CAP1188 test!");

  if (!cap.begin()) {
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


  