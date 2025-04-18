// Link to the Google Drive:
// 

#include <Wire.h> // I2C library
#include <Arduino.h>
#include <BLEDevice.h>

//DHT20 DHT(&Wire1);  //  2nd I2C interface

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
 
void setup() {
 Serial.begin(115200);
 Serial.println("Starting BLE work!");
 
 BLEDevice::init("SDSUCS_Louis");
 BLEServer *pServer = BLEDevice::createServer();
 BLEService *pService = pServer->createService(SERVICE_UUID);
 BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID,
                                        BLECharacteristic::PROPERTY_READ |
                                        BLECharacteristic::PROPERTY_WRITE
                                      );
 
 pCharacteristic->setValue("Server Example -- SDSU IOT Louis");
 pService->start();
 BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
 pAdvertising->addServiceUUID(SERVICE_UUID);
 pAdvertising->setScanResponse(true);
 pAdvertising->setMinPreferred(0x0); 
 pAdvertising->setMinPreferred(0x12);
 BLEDevice::startAdvertising();
 Serial.println("Characteristic defined! Now you can read it in your phone!");
}
void loop() {
 delay(2000);
}