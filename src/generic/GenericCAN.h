#include <Arduino.h>
#include "HardwareCAN.h"


class GenericCANio : public HardwareCAN
{
public:
    GenericCANio(
        bool (*initFunction)() = nullptr,
        CanMsg (*readFunction)() = nullptr,
        int (*writeFunction)(CanMsg const &msg) = nullptr,
        bool (*beginFunction)() = nullptr,
        size_t (*availableFunction)() = nullptr,
        void (*endFunction)() = nullptr
    ) 
    {
        if(initFunction) _initFunction = initFunction;
        if(readFunction) _readFunction = readFunction;
        if(writeFunction) _writeFunction = writeFunction;
        if(beginFunction) _beginFunction = beginFunction;
        if(availableFunction) _availableFunction = availableFunction;
        if(endFunction) _endFunction = endFunction;

    }

    bool (*_initFunction)() = []() { return true; };
    CanMsg (*_readFunction)() = nullptr;
    int (*_writeFunction)(CanMsg const &msg) = nullptr; 
    bool (*_beginFunction)() = []() { return true; };
    size_t (*_availableFunction)() = []() { return (size_t)1; };
    void (*_endFunction)() = []() {};

    bool init(uint16_t pinRX, uint16_t pinTX, uint16_t pinSHDN = NC, uint16_t enable_pin = NC) override
    {
        return _initFunction();
    }

    bool begin(int can_bitrate = 0) override
    {
        return _beginFunction();
    }

    void end() override {
        return _endFunction();
    }

    void filter(CanFilter filter) override {}

    int write(CanMsg const &msg) override
    {
        if (_writeFunction == nullptr) {
          _Serial->println("CAN-ERROR: Write function not defined!");
          return -1;
        }else{
            return _writeFunction(msg);
        }
    }
    CanMsg read() override
    {
        if (_readFunction != nullptr) {
            return _readFunction();
        } else {
            _Serial->println("CAN-ERROR: Read function not defined!");
        }
        return CanMsg();
    }
    size_t available() override
    {
        return _availableFunction();
    }
};