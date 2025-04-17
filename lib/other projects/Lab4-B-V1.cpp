#include <Arduino.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <SparkFunLSM6DSO.h>  // Library from SparkFun for the LSM6DSO sensor
#include <math.h>

// BLE UUIDs (you can change these if needed)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Global BLE characteristic pointer for updating step count
BLECharacteristic *pCharacteristic;

// Create an instance of the sensor (compatible with the LSM6DS3/LSM6DSO breakout)
LSM6DSO imu;

int stepCount = 0;          // Total number of steps counted
bool stepDetected = false;  // Flag to avoid multiple counts per step

// Threshold for detecting a step. Adjust this value based on calibration and mounting.
const float threshold = 0.1;  // For example, 1.2 g (change as needed)

// Setup BLE and the sensor in the setup() function
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Bluetooth Step Counter Starting...");

  // Initialize I²C and the LSM6DSO sensor
  Wire.begin();
  if (!imu.begin()) {
    Serial.println("LSM6DSO initialization unsuccessful. Please check wiring.");
    while (1); // Halt if sensor not found
  }
  Serial.println("LSM6DSO successfully initialized");

  // (Optional) Perform sensor calibration here if required.
  // Use your preferred calibration procedure to map the sensor readings
  // and adjust the threshold value accordingly.

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

void loop() {
  // Sample every 20 milliseconds
  delay(20);

  float ax, ay, az;
  // Read acceleration data from the sensor.

  ax = imu.readFloatAccelX();
  ay = imu.readFloatAccelY();
  az = imu.readFloatAccelZ();


  // The readAccel() function returns true if the data was read successfully.
  //if (imu.readAccel(&ax, &ay, &az)) {
    // For step counting, you might choose one axis or combine two axes.
    // Here, we calculate the root mean square (RMS) from the x and y axes.
    float magnitude = sqrt(ax * ax + ay * ay);
    
    // Debug: print the calculated magnitude
    Serial.print("Acceleration Magnitude (x,y RMS): ");
    Serial.println(magnitude);
    Serial.println(ax);
    Serial.println(ay);
    Serial.println(az);


    // Simple step detection:
    // - When the magnitude rises above the threshold and a step has not already been detected,
    //   count a step and set the flag.
    // - Once the magnitude falls below the threshold, reset the flag.
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
  //} 
  //else {
  //  Serial.println("Failed to read from LSM6DSO sensor.");
  //}
}