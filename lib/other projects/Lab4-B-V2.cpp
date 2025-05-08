// Link to the Google Drive:
// https://drive.google.com/drive/folders/1UM9p6dqo-4r_iQ5tfjsim7sZyf-zWvU2?usp=sharing

#include <Arduino.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <SparkFunLSM6DSO.h>  // Library from SparkFun for the LSM6DSO sensor
#include <math.h>

LSM6DSO myIMU; //Default constructor is I2C, addr 0x6B

// BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Global BLE characteristic pointer for updating step count
BLECharacteristic *pCharacteristic;

int stepCount = 0;          // Total number of steps counted
bool stepDetected = false;  // Flag to avoid multiple counts per step

// Threshold for detecting a step
float threshold = 1;


// Calibration Function
// Samples the sensor for 5 seconds (250 samples at 20ms interval) while at rest.
// It calculates the average (baseline) magnitude based on the X and Y axes,
// then adds a margin to determine the step detection threshold.

void calibrateSensor() {
  Serial.println("Calibrating sensor. Please keep the sensor still for 5 seconds...");
  const int numSamples = 250;  // 5 seconds at 20 ms per sample
  float sumMagnitude = 0.0;
  
  for (int i = 0; i < numSamples; i++) {
    float ax = myIMU.readFloatAccelX();
    float ay = myIMU.readFloatAccelY();
    // Calculate the magnitude using the x and y axes
    float magnitude = sqrt(ax * ax + ay * ay);
    sumMagnitude += magnitude;
    delay(20);
  }
  
  float baseline = sumMagnitude / numSamples;
  threshold = baseline + threshold;
  
  Serial.print("Calibration complete. Baseline: ");
  Serial.print(baseline, 3);
  Serial.print(" | Calibrated Step Threshold: ");
  Serial.println(threshold, 3);
}



void setup() {
  Serial.begin(115200);
  delay(3000); 
  Serial.println("Bluetooth Step Counter Starting...");

  // Initialize I2C and the LSM6DSO sensor
  Wire.begin();
  delay(10);
  if( myIMU.begin() )
    Serial.println("LSM6DSO successfully initialized");
  else { 
    Serial.println("LSM6DSO initialization unsuccessful. Please check wiring.");
    Serial.println("Freezing");
  }

  if( myIMU.initialize(BASIC_SETTINGS) )
    Serial.println("Loaded Settings.");

  // Calibrate the sensor before starting step detection.
  calibrateSensor();

  // Initialize BLE
  BLEDevice::init("SDSUCS Step Counter");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create characteristic with read, write and notify properties
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  // Add descriptor to allow a client to enable notifications
  pCharacteristic->addDescriptor(new BLE2902());

  // Set initial step count value (as string)
  pCharacteristic->setValue(String(stepCount).c_str());

  // Start the service
  pService->start();

  // Start advertising BLE service
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0);  
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE advertising started; connect using a BLE scanner on your cell phone.");
}


void loop()
{
  // Read sensor data
  float ax, ay, az;
  ax = myIMU.readFloatAccelX();
  ay = myIMU.readFloatAccelY();
  az = myIMU.readFloatAccelZ();

  //Debug:
  /*Serial.println(ax, 3);
  Serial.println(ay, 3); 
  Serial.println(az, 3);
  Serial.println();*/
  
  // calculate the root mean square (RMS) from the x and y axes.
  float magnitude = sqrt(ax * ax + ay * ay);
    
  // Debug: print the calculated magnitude
  //Serial.print("Acceleration Magnitude (x,y RMS): ");
  //Serial.println(magnitude);

  // Step detection:
    if (magnitude > threshold && !stepDetected) {
      stepCount++;
      stepDetected = true;

      // Update the BLE characteristic with the new step count
      String stepsString = String(stepCount);
      pCharacteristic->setValue(stepsString.c_str());
      pCharacteristic->notify();  // Notify subscribed BLE clients

      Serial.print("Step detected! Total steps: ");
      Serial.println(stepCount);
    }
    if (magnitude < threshold && stepDetected) {
      stepDetected = false;
    }

  delay(20);
}