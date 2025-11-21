#pragma once

#include "Arduino.h"
#include "HardwareCAN.h"

#if defined(HAL_FDCAN_MODULE_ENABLED) && (defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_PORTENTA_H7_M7))


class STM_FDCAN : public HardwareCAN
{

public:

	STM_FDCAN(uint16_t rx_pin, uint16_t tx_pin, uint16_t shdn_pin = NC, uint16_t enable_pin = NC)
		: HardwareCAN(), filter_(CanFilter(FilterType::ACCEPT_ALL))
	{
		init(rx_pin, tx_pin, shdn_pin, enable_pin);
	}

	bool init(uint16_t rx_pin, uint16_t tx_pin, uint16_t shdn_pin, uint16_t enable_pin) override;

	bool begin(int can_bitrate) override;
	void end() override;

	void filter(CanFilter filter) override;

	int write(CanMsg const &msg) override;
	CanMsg read() override;
	size_t available() override;


	CanStatus subscribe(void (*_messageReceiveCallback)() = nullptr) override;
	CanStatus unsubscribe() override;

	static FDCAN_HandleTypeDef hcan_;

private:
	CanFilter filter_{CanFilter(FilterType::ACCEPT_ALL)};
	void applyFilter(); // filter is applied after begin() is called
	bool started_ = false;
	uint32_t lengthToDLC(uint32_t length);
	uint32_t dlcToLength(uint32_t dlc);
	FDCAN_RxHeaderTypeDef rxHeader_;
	FDCAN_TxHeaderTypeDef txHeader_;
	CanStatus logStatus(char op, HAL_StatusTypeDef status);
};

#endif