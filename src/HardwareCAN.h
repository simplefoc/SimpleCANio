#pragma once

#include "Arduino.h"
#include "CanMsg.h"
#include "CanFilter.h"


#ifndef NC
#define NC 0xFF
#endif

#ifndef MAX_FRAME_SIZE
#define MAX_FRAME_SIZE 64
#endif


enum CanMode
{
    CAN_NORMAL = 0x00U,   // messages will go to tranceiver
    CAN_LOOPBACK = 0x01U, // messages will go to internal loopback
    // CAN_LOOPBACK_EXTERNAL = 0x02U,  <-- some boards support this
};

enum CanErrorType
{
    CAN_ERROR_INIT_TIMEOUT = 0x10U,
    CAN_ERROR_CLOCK = 0x03U,
    CAN_ERROR_TIMING = 0x04U,
    CAN_ERROR_BITRATE_TOO_HIGH = 0x05U,
};

enum CanStatus
{
    CAN_OK = 0x00U,
    CAN_ERROR = 0x01U,
    CAN_NO_DATA = 0x02U,
};

struct CanTiming
{
    uint32_t prescaler;
    uint32_t sjw;
    uint32_t tseg1;
    uint32_t tseg2;
};


enum class CanBitRate : int 
{
    BR_125k = 125000,
    BR_250k = 250000,
    BR_500k = 500000,
    BR_1000k = 1000000,
};

class HardwareCAN 
{

public:
    HardwareCAN();
    
    virtual bool init(uint16_t rx_pin, uint16_t tx_pin, uint16_t shdn_pin = NC, uint16_t enable_pin = NC) = 0;
    
    // setup methods
    virtual bool begin(int can_bitrate) = 0;
    bool begin(CanBitRate const can_bitrate);
    virtual void end() = 0;

    virtual void filter(CanFilter filter) = 0;

	virtual CanStatus subscribe(void (*_messageReceiveCallback)() = nullptr);
	virtual CanStatus unsubscribe();

    virtual int write(CanMsg const &msg) = 0;
    virtual size_t available() = 0;
    virtual CanMsg read() = 0;

    int enableInternalLoopback();
    int disableInternalLoopback();

    void logTo(Stream *serial);

	static void (*_callbackFunction)();

protected:
    virtual CanTiming solveCanTiming(uint32_t clockFreq, uint32_t bitrate, uint8_t multiplier = 1);
    void logMessage(CanMsg const *msg);
    void failAndBlink(CanErrorType errorType);
    
    Print *_Serial = &Serial;
    CanMode mode = CAN_NORMAL;

	uint16_t pinRX_;
	uint16_t pinTX_;
	uint16_t pinSHDN_;
	uint16_t pinEnable_;
};