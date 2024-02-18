#include <Arduino.h>
#include <Wire.h>
#include "AM230.h"



AM2320 mySensor;  // Sensor object

void setup() {

    // Beging UART serial and I2C serial
    Serial.begin(115200);
    Wire.begin();
    delay(100);

    // Stopping if the sensor could not connect
    if (!mySensor.begin())
    {
        Serial.printf("Could not find sensor");
        while (true) {}
    }
    
}


void loop() {
    int start = micros();
    mySensor._readReg(0x00, 0x02); // Reading the humidity(?) register 
    
    printf("in %dus \n", micros() - start);  // Prints the time it took to read
    delay(1000);
}