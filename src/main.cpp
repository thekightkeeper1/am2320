#include <Arduino.h>
#include <Wire.h>

#define AM2320_OK                        0
#define AM2320_ERROR_UNKNOWN            -10
#define AM2320_ERROR_CONNECT            -11
#define AM2320_ERROR_FUNCTION           -12
#define AM2320_ERROR_ADDRESS            -13
#define AM2320_ERROR_REGISTER           -14
#define AM2320_ERROR_CRC_1              -15
#define AM2320_ERROR_CRC_2              -16
#define AM2320_ERROR_WRITE_DISABLED     -17
#define AM2320_ERROR_WRITE_COUNT        -18
#define AM2320_MISSING_BYTES            -19
#define AM2320_READ_TOO_FAST            -20

#define READ_REG_COMMAND 0x03

const uint8_t AM2320_ADDRESS = 0x5c;


class AM2320 {

    public:


        TwoWire *_wire = &Wire;  // For some reason needs to be a pointer
        uint8_t _bits[8]; // Buffer to hold raw data
        
        bool isConnected();

        bool begin();

        // address: 0x5c
        // fuction code 0x03
        //starting address

        int _readReg(uint8_t reg, uint8_t count);

        int _getData(uint8_t length);
};


int AM2320::_readReg(uint8_t reg, uint8_t count) {
    // HANDLE PENDING IRQ
    yield();

    // Wake up the sensor
    if (!isConnected()) return AM2320_ERROR_CONNECT;

    // Requesting the data
    _wire->beginTransmission(AM2320_ADDRESS);  // Will send slave address
    _wire->write(READ_REG_COMMAND);  // Command code (reading regs)
    _wire->write(reg);  // Start register
    _wire->write(count);  // Number of registers to read from
    int rv = _wire->endTransmission(); // Sends queued data and STOP, also stores error code (0 is good)
    if (rv < 0) return rv;  // Negative values are bad error codes

    // command + data len + data + CRC + CRC
    // So 1 extra for command, 1 extra for datalen, and 2 extra for 2 bytes of crc
    rv = _getData(count + 4);
    return rv;

}


int AM2320::_getData(uint8_t length) {

    // Although we send a readreg command code, this line actually follows the
    // i2c protocl to send ACK and read bytes when neccessary.
    // the read bytes are stored in a buffer, that is accessed with 
    // wire->read and wire->available methods
    int bytes = _wire->requestFrom(AM2320_ADDRESS, length);

    if (bytes == 0) return AM2320_ERROR_CONNECT;
    Serial.printf("successfully read %d bytes\n", bytes);
    
    // storing data to easier to read buffer
    // Also now, we can access it from other functions independt Wire lib
    for (int i = 0; i < bytes; i++)
    {
        _bits[i] = _wire->read();
    }

    if (bytes != length)
  {
    switch (_bits[3])
    {
      case 0x80: return AM2320_ERROR_FUNCTION;
      case 0x81: return AM2320_ERROR_ADDRESS;
      case 0x82: return AM2320_ERROR_REGISTER;
      case 0x83: return AM2320_ERROR_CRC_1;  // previous write had a wrong CRC
      case 0x84: return AM2320_ERROR_WRITE_DISABLED;
      default:   return AM2320_ERROR_UNKNOWN;
    }
  }

  // TODO check the crc

}

bool AM2320::isConnected()
{
    const uint16_t timeout = 3000;  //  datasheet 8.2 - wake up is min 800 us max 3000 us
    uint32_t start = micros();
    while (micros() - start < timeout)  // While within timeout
    {

        // These next two lines start and stop an empty transmission
        // If the transmission was successful it is assumed the i2c device
        // Is currently connected
        _wire->beginTransmission(AM2320_ADDRESS);
        if ( _wire->endTransmission() == 0) return true;
        yield();  // So that freeRTOS can do otherstuff
        delayMicroseconds(100);  // Delays in micros and not millis
    }
    return false;
}

bool AM2320::begin() {
        if (!isConnected()) return false;
        return true;
    }

AM2320 mySensor;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    delay(100);
    if (!mySensor.begin())
    {
        Serial.printf("Could not find sensor");
        while (true) {}
    }
    
}


void loop() {
    int start = millis();
    mySensor._readReg(0x00, 0x02);
    
    printf("in %dms \n", millis() - start);
    delay(1000);
}