#pragma once

#include "Arduino.h"
#include "HardwareCAN.h"

#if defined(HAL_CAN_MODULE_ENABLED) && defined(ARDUINO_ARCH_STM32)


class STM_CAN : public HardwareCAN
{

public:

	STM_CAN(uint16_t rx_pin, uint16_t tx_pin, uint16_t shdn_pin = NC, uint16_t enable_pin = NC)
		: HardwareCAN(), filter_(CanFilter(FilterType::ACCEPT_ALL))
	{
		init(rx_pin, tx_pin, shdn_pin, enable_pin);
	}
	
	bool init(uint16_t pinRX, uint16_t pinTX, uint16_t pinSHDN = NC, uint16_t pinEnable = NC) override;

	bool begin(int can_bitrate) override;
	void end() override;

	void filter(CanFilter filter) override;


	int write(CanMsg const &msg) override;
	CanMsg read() override;
	size_t available() override;

	static CAN_HandleTypeDef hcan_;

	CanStatus subscribe(void (*_messageReceiveCallback)() = nullptr) override;
	CanStatus unsubscribe() override;

private:
	bool started_ = false;
	void applyFilter(); // filter is applied after begin() is called
	CanFilter filter_{CanFilter(FilterType::ACCEPT_ALL)};
	CAN_RxHeaderTypeDef rxHeader_;
	CAN_TxHeaderTypeDef txHeader_;
	CanStatus logStatus(char op, HAL_StatusTypeDef status);
};

#endif