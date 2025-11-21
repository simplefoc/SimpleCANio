
#include <Arduino.h>
#include "SimpleCAN.h"   

// configuration
bool isExtendedFrame = false;
uint32_t txIdentifier = 0x321;
bool isRtr = false;


// implement required callback functions
CanMsg readFunc(){ return CanMsg(); }           // TODO implement reading
int writeFunc(const CanMsg &msg){ return 0;}    // TODO implement writing
size_t availableFunc(){ return 1; }             // TODO implement available - default behavior returns 1
// optional functions
bool initFunc(){ return true; }                 // TODO implement if needed
bool begFunc(){ return true; }                  // TODO implement if needed
void endFunc(){}                                // TODO implement if needed


// Create GenericCAN object with function pointers
GenericCAN can(
    initFunc,
    readFunc,
    writeFunc,
    begFunc,
    availableFunc,
    endFunc
);

void setup()
{
    Serial.begin(230400);

    can.logTo(&Serial);
    delay(2000);
    Serial.println("Starting CAN");
    can.begin();
    delay(10);
}

uint8_t data[8] = {0};
uint8_t num = 0;
void loop()
{
    
    data[0] = num++;
    
    delay(50);
    CanMsg txMsg = CanMsg(
        isExtendedFrame ? CanExtendedId(txIdentifier, isRtr) : CanStandardId(txIdentifier, isRtr),
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