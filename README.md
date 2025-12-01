# SimpleCANio - A Simple CAN Abstraction for stm32, esp32 and Teensy devices

This library is an adapration fork of Owen Williams' SimpleCAN library: https://github.com/owennewo/SimpleCAN

[![GitHub release](https://img.shields.io/github/release/simplefoc/SimpleCANio.svg)](https://github.com/simplefoc/SimpleCANio/releases)

This library provides a simple CAN bus abstraction layer for stm32, esp32 and Teensy based boards. 
It supports multiple CAN controllers and makes it easy to switch between them.

It is designed to work well with the SimpleFOC ecosystem, but can be used independently in any project requiring CAN bus communication.

:warning: This library is still in early development. 

## Features 

- **Multi-Architecture Support**: Works on STM32, ESP32, and Teensy boards with a unified API
- **CAN 2.0 Protocol**: Standard (11-bit) and Extended (29-bit) identifiers supported
- **Flexible Filtering**: Accept all, reject all, or mask-based filtering (standard/extended)
- **Message Transmission**: Write CAN messages with automatic retry handling
- **Message Reception**: Polling-based read with FIFO buffering
- **Loopback Mode**: Internal loopback for testing without external hardware
- **Interrupt Callbacks**: Subscribe to receive events (platform-dependent)
- **Generic Interface**: Easy to extend for unsupported CAN controllers via `GenericCAN` class
- **SimpleFOC Ready**: Designed for seamless integration with SimpleFOC ecosystem

## Supported CAN Controllers

### Architecture-Specific Classes

Each platform uses a dedicated CAN driver class that inherits from the `HardwareCAN` base class:

| Architecture | Class Name | CAN Hardware | Header File |
|--------------|------------|--------------|-------------|
| **STM32** | `STM_CAN` <br>(or `CANio`) | CAN (CAN2.0B) | `stm/can/CAN.h` |
| **STM32** | `STM_FDCAN`<br>(or `CANio`)  | FDCAN (CAN-FD, runs in CAN2.0 mode) | `stm/fdcan/CAN.h` |
| **ESP32** | `ESP_TWAI_CAN`<br>(or `CANio`)  | TWAI (CAN2.0) | `esp/twai/CAN.h` |
| **Teensy 3.x/4.x** | `TEENSY_CAN<BUS>`<br>(or `CANio`)  | FlexCAN ( template-based, runns in  CAN2.0 mode ) | `teensy/flexcan.hpp` |
| **GD32** ⚠️ | `GD_CAN` | CAN2B (**untested**) | `gd/can2b/CAN.h` |
| **Generic** | `GenericCAN` | User-implemented | `generic/GenericCAN.h` |

### Platform Details

#### STM32 (CAN and FDCAN)
- **Tested on**: STM32F4, STM32G4, STM32H7 series
- **Peripherals**: Automatically selects CAN or FDCAN based on hardware
- **Filters**: Single hardware filter (standard or extended)
- **Interrupts**: RX FIFO interrupts supported via `subscribe()`
- **Features**: ✅ Standard ID, ✅ Extended ID, ✅ RTR frames, ✅ Filtering, ✅ Callbacks

#### ESP32 (TWAI)
- **Tested on**: ESP32, ESP32-S3, ESP32-C6
- **Hardware**: TWAI controller (Two-Wire Automotive Interface, compatible with CAN2.0)
- **Filters**: Single hardware filter (standard or extended)
- **Interrupts**: ❌ Callback mode not implemented (use polling)
- **Features**: ✅ Standard ID, ✅ Extended ID, ✅ RTR frames, ✅ Filtering, ❌ Callbacks

#### Teensy (FlexCAN)
- **Tested on**: Teensy 3.5, 4.1
- **Hardware**: FlexCAN module
- **Buffers**: 256 RX messages, 16 TX messages
- **Filters**: FIFO filters with mask support (up to 8 individual filters)
- **Template Design**: Requires specifying CAN bus at compile time (e.g., `TEENSY_CAN<CAN1>`)
- **Multiple Buses**: Teensy 3.6 supports CAN0/CAN1, Teensy 4.x supports CAN1/CAN2/CAN3
- **Features**: ✅ Standard ID, ✅ Extended ID, ✅ RTR frames, ✅ Filtering, ✅ Callbacks

#### GD32
- **Untested** - Community contributions welcome!
- **Hardware**: GD32 CAN2B controller
- **Status**: Basic support implemented
- **Features**: ✅ Standard ID, ✅ Extended ID, ⚠️ No testing

### Feature Matrix

| Feature | STM32 (CAN/FDCAN) | ESP32 (TWAI) | Teensy (FlexCAN) | GD32 |
|---------|---------------------|--------------|------------------|------|
| Standard ID (11-bit) | ✅ | ✅ | ✅ | ⚠️ |
| Extended ID (29-bit) | ✅ | ✅ | ✅ | ⚠️ |
| RTR Frames | ✅ | ✅ | ✅ | ⚠️ |
| Filter Masks (Standard/Extended) | ✅ | ✅ | ✅ | ⚠️ |
| Multiple Filters | ❌ | ❌ | ❌ | ⚠️ |
| Accept/Reject All Mode | ✅ | ✅ | ✅ | ⚠️ |
| Internal Loopback | ✅ | ✅ | ✅ | ⚠️ |
| Interrupt Callbacks | ✅ `subscribe()` | ❌ |  ❌ |  ❌ |
| Polling `available()` | ✅ | ✅ | ✅ | ⚠️ |
| Debugging Info (`CAN_DEBUG` flag) | ✅ | ✅ | ✅ | ⚠️ |

### CAN Bitrates Supported
All platforms support these common bitrates up to 1Mbps

### Teensy Note
Teensy boards use the FlexCAN_T4 library for CAN communication. <br>**Important**: Teensy requires a template parameter specifying which CAN bus to use.

#### Template Instantiation
```cpp
#include "SimpleCANio.h"

// Instantiate FlexCAN first
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> flexcan();

// Pass reference to TEENSY_CAN wrapper
TEENSY_CAN<CAN1> can(flexcan);

can.begin(1000000); // 1 Mbps
```

Or use the helper macro:
```cpp
auto can_bus = TEENSY_FLEXCAN(CAN_BUS);
CANio can(can_bus, CAN_SHDN, CAN_ENABLE); 
```

#### Pin Mappings

| Board        | CAN bus|  CAN RX Pin | CAN TX Pin | Alternate Pins |
|--------------|--------|-------------|------------|----------------|
| Teensy 3.2/3.5/3.6   | CAN0   | 4           | 3          | RX=30, TX=29   |
| Teensy 3.6   | CAN1   | 34          | 33         | -              |
| Teensy 4.0/4.1   | CAN1   | 23          | 22         | RX=13, TX=11   |
| Teensy 4.0/4.1   | CAN2   | 0           | 1          | -              |
| Teensy 4.0/4.1   | CAN3   | 30          | 31         | -              |

**Note**: Pin selection is handled by FlexCAN_T4 library based on the template parameter. The wrapper does not require explicit pin arguments in the constructor. 

**Note**: If using alternate pins, ensure to configure them in FlexCAN_T4 using the `setRX(ALT)` and/or `setTX(ALT)` methods.

**Note**: Library will output the detected CAN configuration (including pins) to Serial during `begin()`.

## SimpleFOC Integration

SimpleFOC library builds on top of SimpleCANio to construct the CANCommander protocol (see SimpleFOCdrivers library for more details)

## API Reference

### Basic Usage

```cpp
#include "SimpleCANio.h"

// Platform-specific instantiation
#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
  auto flexcan = TEENSY_FLEXCAN(CAN1);
  CANio can(flexcan);
#else
  CANio can(CAN_RX_PIN, CAN_TX_PIN); // STM32, ESP32, GD32
#endif

void setup() {
  // Initialize at 1 Mbps
  can.begin(1000000);
  
  // Optional: Set filter (default accepts all)
  CanFilter filter = CanFilter(MASK_STANDARD, 0x123, 0x7FF);
  can.filter(filter);
}

void loop() {
  // Transmit
  uint8_t data[8] = {0x01, 0x02, 0x03};
  CanMsg txMsg = CanMsg(CanStandardId(0x123), 3, data);
  can.write(txMsg);
  
  // Receive (polling)
  if (can.available()) {
    CanMsg rxMsg = can.read();
    // Process message
  }
}
```

### Core Methods

| Method | Description | Returns |
|--------|-------------|---------|
| `begin(bitrate)` | Initialize CAN at specified bitrate | `bool` - true on success |
| `end()` | Disable CAN peripheral | `void` |
| `write(CanMsg)` | Transmit a CAN message | `int` - 1 on success, 0 on failure |
| `available()` | Check for received messages | `size_t` - number of messages in RX buffer |
| `read()` | Read next message from RX buffer | `CanMsg` |
| `filter(CanFilter)` | Apply message filter | `void` |
| `subscribe(callback)` | Register RX interrupt callback | `CanStatus` (platform-dependent) |
| `enableInternalLoopback()` | Enable loopback mode for testing | `int` |

### Filter Types

```cpp
// Accept all messages
CanFilter filter = CanFilter(FilterType::MASK_ACCEPT_ALL);

// Reject all messages
CanFilter filter = CanFilter(FilterType::MASK_REJECT_ALL);

// Standard ID with mask (11-bit)
CanFilter filter = CanFilter(FilterType::MASK_STANDARD, id, mask);

// Extended ID with mask (29-bit)
CanFilter filter = CanFilter(FilterType::MASK_EXTENDED, id, mask);
```

### Message Construction

```cpp
// Standard ID (11-bit)
CanMsg msg = CanMsg(CanStandardId(0x123), data_length, data_array);

// Extended ID (29-bit)
CanMsg msg = CanMsg(CanExtendedId(0x12345678), data_length, data_array);

// RTR (Remote Transmission Request)
CanMsg msg = CanMsg(CanStandardId(0x123, true), 0, nullptr); // RTR flag = true
```

## Extending for Custom Hardware

To add support for new CAN controllers, inherit from `HardwareCAN`:

```cpp
class MyCustomCAN : public HardwareCAN {
public:
  bool init(uint16_t rx, uint16_t tx, uint16_t shdn, uint16_t en) override;
  bool begin(int bitrate) override;
  void end() override;
  void filter(CanFilter filter) override;
  int write(CanMsg const &msg) override;
  CanMsg read() override;
  size_t available() override;
};
```
There is also an option to use the `GenericCAN` class if you want to implement all methods within your main.cpp file. <br>
`GenericCAN` implements the `HardwareCAN` interface directly and allows you to define the callbacks in your main application code.

```cpp
#import "SimpleCANio.h"

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

void setup() {
  can.begin(500000); // 500 kbps
}
void loop() {
  if (can.available()) {
    CanMsg msg = can.read();
    // Process message
  }
}
```

## Known Limitations

- **ESP32** and **Teensy**: Interrupt-based callbacks (`subscribe()`) not implemented - use polling with `available()`
- **All Platforms**: Only CAN 2.0 supported (CAN-FD data phase disabled on FDCAN hardware)
- **STM32F4** - `subsribe()` method is called if messages available in the buffer rather than on message receipt - Make sure to read messages on interrupt to avoid infinite callback loop 

## Troubleshooting

**CAN not working?**
1. Check RX/TX pin connections and transceiver wiring
2. Do you have a 120-ohm termination resistor at each end of the CAN bus?
2. Verify both nodes use the same bitrate
3. Enable loopback mode to test without external hardware: `can.enableInternalLoopback()`
4. Check Serial output during `begin()` for initialization errors (enable `CAN_DEBUG` flag for more details)

**Teensy compile errors?**
- Ensure you specify the CAN bus template: `TEENSY_CAN<CAN1>` not just `TEENSY_CAN`
- Verify the pins are correct for your Teensy model, see the Serial output during `begin()`

**Messages not filtering?**
- Call `filter()` **before** `begin()`
- Check mask calculation: bits set to 1 in mask must match exactly in ID

**No messages received?**
- Maybe your filter is too restrictive - try using `MASK_ACCEPT_ALL` to verify communication or just dont set any filter at all

