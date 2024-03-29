#include <Arduino.h>
#include <Wire.h>

#include "ty_AM230.cpp"



ty_AM2320 mySensor;  // Sensor object


void setup() {

    // Beginning UART serial and I2C serial
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
    
    printf("Its %.2f%% humidity and %.2f degs Celcius \n", mySensor.getHumidity(), mySensor.getTemperature());  // Prints the time it took to read
    delay(2000);
}