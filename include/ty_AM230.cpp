#include "ty_AM230.h"



bool ty_AM2320::begin() {
  if (!isConnected()) return false;
  return true;
}


bool ty_AM2320::isConnected()
{
    const uint16_t timeout = 3000;  //  datasheet 8.2 - wake up is min 800 us max 3000 us
    uint32_t start = micros();
    while (micros() - start < timeout)  // While within timeout
    {

      // These next two lines start and stop an empty transmission
      // If the transmission was successful it is assumed the i2c device
      // Is currently connected
      _wire->beginTransmission(0x5c);
      if ( _wire->endTransmission() == 0) return true;
      yield();  // So that freeRTOS can do otherstuff
      delayMicroseconds(100);  // Delays in micros and not millis
    }
    return false;
}


int ty_AM2320::read() {  



  /*
    1. Handle too many reads
    2. Read 4 regs starting from 0x00
      remember, after operations like reading, you need to error check and propagate
    3. Decode data
      Humidity is just hex --> dec
      Temperatures above a certain threshold are negative
    
    
    
  */  
  
  // Stoppping too many consecutive reads
  if (millis() - _lastRead < _readDelay) {
    return AM2320_READ_TOO_FAST;
  }
  _lastRead = millis();

  // READ HUMIDITY AND TEMPERATURE REGS
  int rv = _readReg(0x00, 0x04);  // Remember this returns error codes, not data
  if (rv < 0) return rv;  // propagating UNKNOWN error

  if (rv != AM2320_OK) {
    {
      _humidity = AM2320_INVALID_VALUE;
      _temperature = AM2320_INVALID_VALUE;
    }
    return rv;  // propagate a known error code (probably)
  }


  /////////////////////////////////////////////////////
  // IF NO ERRORS:

  // EXTRACT HUMIDITY AND TEMPERATURE
 //  EXTRACT HUMIDITY AND TEMPERATURE
 
 // converting byte array to hex by *256, and that number reps 10x real humidity
 // Also note that the _bits array is offest, since first 2 bytes are the crc
  _humidity = (_bits[2] * 256 + _bits[3]) * 0.1;  // hum is in regs 0x00 and 0x02
  int16_t t = ((_bits[4] & 0x7F) * 256 + _bits[5]);  // Temp is in 0x02 and 0x03
  if (t == 0)
  {
    _temperature = 0.0;     // prevent -0.0;
  }
  else
  {
    _temperature = t * 0.1;
    // If the high bits are 0000 1000, assume its negative
    // This works b/c 0x80 * 256 = 0x8000 over 409 degrees celcius. Not exactly possible.
    if ((_bits[4] & 0x80) == 0x80 )
    {
      _temperature = -_temperature;
    }
  }
  return AM2320_OK;
}


int ty_AM2320::_readReg(uint8_t regToRead, uint8_t count) {

  /*
    1. Wake up the sensor
    2. Send the I2C transmission
     remeber to check for errors after transmission
  */
    // HANDLE PENDING IRQ
    yield();

    // Wake up the sensor by doing an empty i2c transmission
    if (!isConnected()) return AM2320_ERROR_CONNECT;
    int rv = -1;

    // Requesting the data
    // _wire->();                      // TODO The function that starts a M->S transaction
    // _wire->write();                 // TODO Command code that tells AM2320 to read registers
    // _wire->write();                 // TODO fill in with the start reg that needs to be read
    // _wire->write();                 // TODO Number of regs that store the humidity + temp data
    //                                 // TODO The function that ends the transmssion (look up Wire documentation)
    // int rv = _wire->;               
    if (rv < 0) return rv;  // Negative values are bad error codes

    // command + datalen + data + CRC
    //   count +    1    +  1   +  2 
    rv = _getData(count + 4);
    return rv;  // Returns the number of bytes stored into the _bits array

}


int ty_AM2320::_getData(uint8_t length) {
  int bytes = 0;

  /*
    This function assumes that a request has already been sent, the the 
    device is pending to send data back.

    1. Requestion bytes
      Remeber to error check
    2. BUffer data, and check for error codes in recieved data
    3. Check the crc
  
  */

    // Although we send a readreg command code, this line actually follows the
    // i2c protocl to send ACK and read bytes when neccessary.
    // the read bytes are stored in a buffer, that is accessed with 
    // wire->read and wire->available methods

    // bytes = _wire->requestFrom();  // TODO: Use the following function, as well as the length and address to request the 4 temp and humidty registers
    // Function: https://www.arduino.cc/reference/en/language/functions/communication/wire/requestfrom/
    

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

  // CRC is LOW Byte first
  uint16_t crc = _bits[bytes - 1] * 256 + _bits[bytes - 2];
  if (_crc16(&_bits[0], bytes - 2) != crc)
  {
    return AM2320_ERROR_CRC_2;  // read itself has wrong CRC
  }

  return AM2320_OK;

}


uint16_t ty_AM2320::_crc16(uint8_t *ptr, uint8_t len)
{
  uint16_t crc = 0xFFFF;
  while(len--)
  {
    crc ^= *ptr++;
    for (uint8_t i = 0; i < 8; i++)
    {
      if (crc & 0x01)
      {
        crc >>= 1;
        crc ^= 0xA001;
      }
      else
      {
        crc >>= 1;
      }
    }
  }
  return crc;
}

float ty_AM2320::getHumidity() {
  return _humidity;
}

float ty_AM2320::getTemperature() {
  return _temperature;
}


