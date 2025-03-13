#include <Arduino.h>
#include <Servo.h>
Servo myservo; // create servo object to control a servo

//cosntants for the pins where sensors are plugged into.
const int sensorPin = 2;
const int ledPin = 12;

// variables
unsigned long startTime;
int lightmax = 0;
int lightmin = 4095;
int lightVal;   // current light reading


void setup() {
  Serial.begin(115200);
  startTime = millis();

  myservo.attach(27); // attaches the servo on port 27 to the servo object
  myservo.write(0);  // Start servo in zero position

  pinMode(ledPin, OUTPUT);
  pinMode(sensorPin, INPUT);
}

void loop() {
   if (millis() - startTime < 10000) { 
      // Calibration phase
      Serial.println("Calibration is running...");

      //Blinking LED
      if ((millis() - startTime)/1000 % 2 == 0) {
        digitalWrite(ledPin, HIGH);
      } else {
        digitalWrite(ledPin, LOW);
      }
      
      // Measure MIN and MAX
      lightVal = analogRead(sensorPin); // read the current light levels
      if (lightVal > lightmax) {
        lightmax = lightVal;
      }
      if (lightVal < lightmin) {
        lightmin = lightVal;
      }
      delay(500);

  } else {
    // Running phase
      
    lightVal = analogRead(sensorPin); // Read light sensor value
    int servoPos = map(lightVal, lightmin, lightmax, 0, 179); // Map light to servo position
    myservo.write(servoPos);

    servoPos = constrain(servoPos, 0, 179); // Ensure within limits

    Serial.print("Light Value: ");
    Serial.println(lightVal);
    Serial.print("Servo Position: ");
    Serial.println(servoPos);
    Serial.println("");

    delay(500);

  }
}
  