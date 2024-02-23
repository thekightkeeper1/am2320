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
