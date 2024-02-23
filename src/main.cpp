#include <Arduino.h>
#include <Wire.h>
#include "ty_AM230.cpp"



ty_AM2320 mySensor;  // Sensor object

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
    mySensor.read(); 
    
    printf("%f%% in %dus \n", mySensor.getHumidity(), micros() - start);  // Prints the time it took to read
    delay(2000);
}