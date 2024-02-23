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

//  allows to overrule AM232X_INVALID_VALUE e.g. to prevent spike in graphs.
#ifndef AM2320_INVALID_VALUE
#define AM2320_INVALID_VALUE                  -999
#endif

const uint8_t AM2320_ADDRESS = 0x5c;


class ty_AM2320 {

    public:


        TwoWire *_wire = &Wire;  // For some reason needs to be a pointer
        
        bool isConnected();

        bool begin();

        int read();

        // address: 0x5c
        // fuction code 0x03
        int _readReg(uint8_t reg, uint8_t count);

        int _getData(uint8_t length);

        uint16_t _crc16(uint8_t *ptr, uint8_t len);

        float getHumidity();


        float _humidity = 0.0;
        uint32_t _lastRead      = 0;
        uint16_t _readDelay     = 2000;
        float    _temperature   = 0.0;
        uint8_t _bits[8]; // Buffer to hold raw data
};



int ty_AM2320::_readReg(uint8_t reg, uint8_t count) {
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


int ty_AM2320::_getData(uint8_t length) {

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
  // CRC is LOW Byte first
  uint16_t crc = _bits[bytes - 1] * 256 + _bits[bytes - 2];
  if (_crc16(&_bits[0], bytes - 2) != crc)
  {
    return AM2320_ERROR_CRC_2;  // read itself has wrong CRC
  }

  return AM2320_OK;

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
        _wire->beginTransmission(AM2320_ADDRESS);
        if ( _wire->endTransmission() == 0) return true;
        yield();  // So that freeRTOS can do otherstuff
        delayMicroseconds(100);  // Delays in micros and not millis
    }
    return false;
}

bool ty_AM2320::begin() {
        if (!isConnected()) return false;
        return true;
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

int ty_AM2320::read() {

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