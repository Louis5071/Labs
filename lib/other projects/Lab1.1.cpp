#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h> // SPI library

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
//TFT_eSprite img = TFT_eSprite(&tft);  // Sprite

// Programm to use the OLED Display

// variables
unsigned long startTime;
int currentTime = 0;
int previousTime = 0;



void setup() {
  Serial.begin(115200);
  startTime = millis();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  //tft.loadFont("NotoSansBold36");
  //img.setFreeFont();

}

void loop() {
   
  currentTime = (millis() - startTime)/1000;

  if (currentTime != previousTime) {
    // Convert Int to Str
    //char buffer[10];
    //itoa(currentTime, buffer, 10);
    //tft.drawString(buffer, 120, 68, 10); 
    //tft.drawFloat(currentTime, 120, 68, 100);   
    tft.drawNumber(currentTime, 80, 40, 8);
    Serial.println(currentTime);
    previousTime = currentTime;
    }

}


  