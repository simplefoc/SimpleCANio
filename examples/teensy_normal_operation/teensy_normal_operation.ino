/*
This code example shows normal operation of the SimpleCANio library.

This code is specific to Teensy boards using FlexCAN_T4.
*/
#include <Arduino.h>
#include "SimpleCANio.h"   // <- this is the only include required, it should be smart enough to find the correct subclass

#define CAN_ID 0x321

// | Board        | CAN bus|  CAN RX Pin | CAN TX Pin |
// |--------------|-------------|------------|------------|
// | Teensy 3.2   |CAN0         | 4          | 3          |
// | Teensy 3.5   |CAN0         | 4          | 3          |
// | Teensy 3.6   |CAN0         | 4          | 3          |
// | Teensy 3.6   |CAN1         | 34         | 33         |
// | Teensy 4.0   |CAN1         | 23         | 22         |
// | Teensy 4.1   |CAN2         | 0          | 1          |
// | Teensy 4.1   |CAN3         | 30         | 31         |    
#define CAN_BUS CAN0 // TODO set the bus you want to use , Teensy
#define CAN_SHDN NC
#define CAN_ENABLE NC

TEENSY_FLEXCAN(CAN_BUS) can_bus;
CANio can(can_bus, CAN_SHDN, CAN_ENABLE); 

void setup()
{

    Serial.begin(230400);

    can.logTo(&Serial);
    delay(2000);
    Serial.println("Starting CAN");
    // can.enableInternalLoopback();
    
    // choose one of the receive filters to apply
    CanFilter filter = CanFilter(MASK_EXTENDED, CAN_ID, CAN_ID, FILTER_ANY_FRAME);
    // CanFilter filter = CanFilter(MASK_STANDARD, CAN_ID, CAN_ID, FILTER_ANY_FRAME);
    // CanFilter filter = CanFilter(MASK_ACCEPT_ALL);
    can.filter(filter);

    // 1 mbps
    can.begin(1000000);
    delay(10);
}

uint8_t data[8] = {0};
uint8_t num = 0;

void loop()
{

    data[0] = num++;

    bool isExtendedFrame = false;
    bool isRtr = false;
    delay(50);
    CanMsg txMsg = CanMsg(
        isExtendedFrame ? CanExtendedId(CAN_ID, isRtr) : CanStandardId(CAN_ID, isRtr),
        4,
        data);
    delay(50);

    can.write(txMsg);
    delay(1);

    if (can.available() > 0)
    {
        CanMsg const rxMsg = can.read();

        Serial.print("polling read: ");
        if (rxMsg.isExtendedId())
        {
            Serial.print(rxMsg.getExtendedId(), HEX);
            Serial.println(" Extended ✅");
        }
        else
        {
            Serial.print(rxMsg.getStandardId(), HEX);
            Serial.println(" Standard ✅");
        }
    }

    delay(2000);
}