#include "AM230.h"



int AM2320::_readReg(uint8_t reg, uint8_t count) {
    // HANDLE PENDING IRQ
    yield();

    // Wake up the sensor by doing an empty i2c transmission
    if (!isConnected()) return AM2320_ERROR_CONNECT;

    // Requesting the data
    _wire->beginTransmission(AM2320_ADDRESS);  // Initiates i2c transmission, handles sending slave adress header
    _wire->write(READ_REG_COMMAND);  // Command code (reading regs)
    _wire->write(reg);  // Start register
    _wire->write(count);  // Number of registers to read from
    int rv = _wire->endTransmission(); // Sends queued data and STOP, also stores error code (0 is good)
    if (rv < 0) return rv;  // Negative values are bad error codes

    // command + datalen + data + CRC
    //   count +    1    +  1   +  2 
    rv = _getData(count + 4);
    return rv;  // Returns the number of bytes stored into the _bits array

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

