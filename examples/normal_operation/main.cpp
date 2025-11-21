
#include <Arduino.h>
#include "SimpleCANio.h"   // <- this is the only include required, it should be smart enough to find the correct subclass

#define CAN_ID 0x321

#define CAN_RX // TODO set your CAN RX and TX pins here
#define CAN_TX // TODO set your CAN RX and TX pins here
#define CAN_SHDN NC
#define CAN_ENABLE NC

CANio can = CANio(CAN_RX, CAN_TX, CAN_SHDN, CAN_ENABLE); // <- create SimpleCAN object

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
    // CanFilter filter = CanFilter(ACCEPT_ALL);
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