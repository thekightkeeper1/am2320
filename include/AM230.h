#pragma once

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
