#include <Arduino.h>
#include <Wire.h>
#include <BLEDevice.h>
#include "SparkFunLSM6DSO.h"

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Step Detection Parameters
const int   calibrationSamples = 50;
float       baselineAccelY     = 0.0;
const float stepThreshold      = 0.2;   // (in g) acceleration delta that triggers a step
const float resetThreshold     = 0.1;   // (in g) reset
int   stepCount    = 0;
bool  stepDetected = false;

LSM6DSO myIMU;  

BLECharacteristic *pCharacteristic;

void setup() {
  delay(3000);
  Serial.begin(115200);
  Serial.println("Starting BLE Step Counter with LSM6DSO!");
  
  // Sensor Setup
  Wire.begin();
  
  delay(100);

  if (myIMU.begin() == false) {
    Serial.println("Could not connect to LSM6DSO. Check wiring.");
    while (1) { delay(10); }
  }
  Serial.println("LSM6DSO is online.");


  // Calibrate the Y-axis
  Serial.println("Calibrating: Keep the board still...");
  delay(500);

  float sum = 0.0;
  for (int i = 0; i < calibrationSamples; i++) {
    // Read the accelerometer's Y-axis (in g)
    float ay = myIMU.readFloatAccelY();
    sum += ay;
    delay(20);
  }
  baselineAccelY = sum / calibrationSamples;
  
  Serial.print("Calibration complete. Baseline Y-axis: ");
  Serial.println(baselineAccelY, 3);

  // BLE Setup
  BLEDevice::init("SDSUCS_Louis");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Add characteristic with READ, WRITE, and NOTIFY properties
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Initialize the characteristic with a starting value
  pCharacteristic->setValue("0");

  // Start the BLE service
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE advertising started! Connect to see step count updates.");
}

void loop()
{
  //Get all parameters
  Serial.print("\nAccelerometer:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.readFloatAccelX(), 3);
  Serial.print(" Y = ");
  Serial.println(myIMU.readFloatAccelY(), 3);
  Serial.print(" Z = ");
  Serial.println(myIMU.readFloatAccelZ(), 3);

  Serial.print("\nGyroscope:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.readFloatGyroX(), 3);
  Serial.print(" Y = ");
  Serial.println(myIMU.readFloatGyroY(), 3);
  Serial.print(" Z = ");
  Serial.println(myIMU.readFloatGyroZ(), 3);

  Serial.print("\nThermometer:\n");
  Serial.print(" Degrees F = ");
  Serial.println(myIMU.readTempF(), 3);

  delay(1000);
}

/* void loop() {
  // Read the Y-axis acceleration
  float currentAY = myIMU.readFloatAccelY();
  float deltaY = currentAY - baselineAccelY;
  Serial.print("Y-axis acceleration: ");
  Serial.println(currentAY, 3);

  // Basic threshold-based step detection
  if (!stepDetected && (deltaY > stepThreshold)) {
    stepCount++;
    stepDetected = true;

    Serial.print("Step detected! Count = ");
    Serial.println(stepCount);

    // Update BLE characteristic and notify connected device
    String stepsStr = String(stepCount);
    pCharacteristic->setValue(stepsStr.c_str());
    pCharacteristic->notify();
  }

  // Reset the detection state once we drop below a lower threshold
  if (stepDetected && (deltaY < resetThreshold)) {
    stepDetected = false;
  }

  // Sample rate 20ms
  delay(200);
}
*/