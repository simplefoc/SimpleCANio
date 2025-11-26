# SimpleCANio - A Simple CAN Abstraction for stm32, esp32 and Teensy devices

This library is an adapration fork of Owen Williams' SimpleCAN library: https://github.com/owennewo/SimpleCAN

[![GitHub release](https://img.shields.io/github/release/simplefoc/SimpleCANio.svg)](https://github.com/simplefoc/SimpleCANio/releases)

This library provides a simple CAN bus abstraction layer for stm32, esp32 and Teensy based boards. 
It supports multiple CAN controllers and makes it easy to switch between them.

It is designed to work well with the SimpleFOC ecosystem, but can be used independently in any project requiring CAN bus communication.

:warning: This library is still in early development. 

## Features 

- Support for multiple CAN controllers (see list below)
- Simple API for sending and receiving CAN messages
- Designed for easy integration with SimpleFOC projects
- Compatible with stm32 and esp32 based boards
- Allows an easy implementation of new CAN interfaces for unsupported controllers (`GenericCAN` class)

## Supported CAN Controllers
- STM32 built-in CAN controller 
    - CAN and FDCAN peripherals supported (both configured to run CAN2.0)
- ESP32 built-in CAN controller
    - tested on esp32, esp32s3 and esp32c6
- Teensy FlexCAN_T4 controller
    - tested on Teensy 4.x and 3.x boards
- Allows for custom CAN controllers via `GenericCAN` class


### Teensy Note
Teensy boards use the FlexCAN_T4 library for CAN communication. And will use the default Rx/Tx pins. The list of default pins can be found in the [Teensy docs](https://www.pjrc.com/teensy/pinout.html). Here is a quick reference:

| Board        | CAN bus|  CAN RX Pin | CAN TX Pin |
|--------------|------------|------------|------------|
| Teensy 3.2   |CAN0 | 4         | 3         |
| Teensy 3.5   |CAN0 | 4         | 3         |
| Teensy 3.6   |CAN0 | 4         | 3         |
| Teensy 3.6   |CAN1 | 34         | 33         |
| Teensy 4.0   |CAN1 | 23          | 22          |
| Teensy 4.1   |CAN2 | 0          | 1          |
| Teensy 4.1   |CAN3 | 30          | 31          |    

> Library will output the used CAN pins to Serial during initialization `can.begin()`

## SimpleFOC Integration

SimpleFOC library builds on top of SimpleCANio to construct the CANCommander protocol (see SimpleFOCdrivers library for more details)

