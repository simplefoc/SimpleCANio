# SimpleCANio - A Simple CAN Abstraction for stm32 and esp32 devices

This library is an adapration fork of Owen Williams' SimpleCAN library: https://github.com/owennewo/SimpleCAN

[![GitHub release](https://img.shields.io/github/release/simplefoc/SimpleCANio.svg)](https://github.com/simplefoc/SimpleCANio/releases)

This library provides a simple CAN bus abstraction layer for stm32 and esp32 based boards. 
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
    - CAN and FDCAN peripherals supported
- ESP32 built-in CAN controller
    - tested on esp32, esp32s3 and esp32c6
- Allows for custom CAN controllers via `GenericCAN` class

## SimpleFOC Integration

SimpleFOC library builds on top of SimpleCANio to construct the CANCommander protocol (see SimpleFOCdrivers library for more details)
