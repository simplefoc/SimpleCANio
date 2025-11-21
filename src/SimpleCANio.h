#ifndef SIMPLECANIO_H
#define SIMPLECANIO_H

#include "Arduino.h"
#include "HardwareCAN.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "esp/twai/CAN.h"
using CANio = ESP_TWAI_CAN;           // Use CANio as an alias for TWAI_CAN
#elif defined(ARDUINO_ARCH_STM32)
    #if defined(FDCAN1_BASE) 
        #if !defined(HAL_FDCAN_MODULE_ENABLED)
            #define HAL_FDCAN_MODULE_ENABLED 
        #endif
        #include "stm/fdcan/CAN.h"
        using CANio = STM_FDCAN;   // Use CANio as an alias for STM_FDCAN 
    #elif defined(CAN1_BASE)
        #if !defined(HAL_CAN_MODULE_ENABLED)
            #define HAL_CAN_MODULE_ENABLED
        #endif
        #include "stm/can/CAN.h"
        using CANio = STM_CAN;   // Use CANio as an alias for STM_CAN 
    #else
        #error "This STM32 family does not support CAN or FDCAN"
    #endif
#elif defined(ARDUINO_PORTENTA_H7_M7) 
    #if !defined(HAL_FDCAN_MODULE_ENABLED)
        #define HAL_FDCAN_MODULE_ENABLED 
    #endif
    #include "stm/fdcan/CAN.h"
    using CANio = STM_FDCAN;   // Use CANio as an alias for STM_FDCAN
#elif defined(ARDUINO_ARCH_GD32)
#include "gd/can2b/CAN.h"
using CANio = GD_CAN;   // Use CANio as an alias for GD_CAN
#else
#error "No CAN module is enabled, expecting a define for ARDUINO_ARCH_ESP32 | ARDUINO_ARCH_STM32 |ARDUINO_PORTENTA_H7_M7| ARDUINO_ARCH_GD32"
#endif

#include "generic/GenericCAN.h"

#endif