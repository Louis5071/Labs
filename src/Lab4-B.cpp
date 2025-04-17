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
const float threshold = 1;


void setup() {
  Serial.begin(115200);
  delay(3000); 
  Serial.println("Bluetooth Step Counter Starting...");

  // Initialize I²C and the LSM6DSO sensor
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
  Serial.print("Acceleration Magnitude (x,y RMS): ");
  Serial.println(magnitude);

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
  

/*
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
  */

  delay(20);
}