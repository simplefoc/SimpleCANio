#pragma once

#if ARDUINO_ARCH_GD32 && 0

#include "Arduino.h"
#include "HardwareCAN.h"
#include "gd32f30x.h"
#include "gd32f30x_can.h"
#include "gd32f30x_gpio.h"

class GD_CAN : public HardwareCAN
{

public:
    GD_CAN() = default;
    bool init(uint16_t pinRX, uint16_t pinTX, uint16_t pinSHDN = NC, uint16_t enable_pin = NC) override;

    bool begin(int can_bitrate) override;
    void end() override;

    void filter(CanFilter filter) override;

    int write(CanMsg const &msg) override;
    CanMsg read() override;
    size_t available() override;

    static uint32_t hcan_;


    // Not implemented in GD32 CAN driver
    // CanStatus subscribe(void (*_messageReceiveCallback)() = nullptr) override;
    // CanStatus unsubscribe() override;

private:
    bool started_ = false;
    void applyFilter(); // filter is applied after begin() is called
    CanFilter filter_;
    can_receive_message_struct rxHeader_;
    can_trasnmit_message_struct txHeader_;
    CanStatus logStatus(char op, uint32_t status);
};

#endif