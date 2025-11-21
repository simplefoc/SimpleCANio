#pragma once

#include "Arduino.h"
#if defined(ARDUINO_ARCH_ESP32)

#include "HardwareCAN.h"
#include "driver/twai.h"

class ESP_TWAI_CAN : public HardwareCAN
{

public:
    // match base: provide init(), not constructor-only pin capture
    ESP_TWAI_CAN(uint16_t rx_pin, uint16_t tx_pin, uint16_t shdn_pin = NC, uint16_t enable_pin = NC)
        : HardwareCAN()
    {
        init(rx_pin, tx_pin, shdn_pin, enable_pin);
    }

    bool init(uint16_t pinRX, uint16_t pinTX, uint16_t pinSHDN = NC, uint16_t enable_pin = NC) override;


    bool begin(int can_bitrate) override;
    void end() override;

    void filter(CanFilter filter) override;

    int write(CanMsg const &msg) override;
    CanMsg read() override;
    size_t available() override;

    // Not implemented in ESP-TWAI driver
    // CanStatus subscribe(void (*_messageReceiveCallback)() = nullptr) override;
    // CanStatus unsubscribe() override;

private:
    twai_general_config_t _general_config;
    twai_timing_config_t _timing_config;
    twai_filter_config_t _filter_config;
    twai_message_t _rxEspFrame;
    twai_message_t _txEspFrame;
    twai_status_info_t _statusInfo;
    CanFilter _filter{CanFilter(FilterType::ACCEPT_ALL)};

    CanStatus logStatus(char op, esp_err_t status);
};

#endif
