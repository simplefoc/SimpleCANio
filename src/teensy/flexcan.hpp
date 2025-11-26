#pragma once

#include "Arduino.h"
#if defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY32)

#include "HardwareCAN.h"
#include <FlexCAN_T4.h>

// Template macros for cleaner code
#define FLXCAN_CLASS template <CAN_DEV_TABLE CAN_BUS>
#define FLXCAN_T4 FlexCAN_T4<CAN_BUS, RX_SIZE_256, TX_SIZE_16>

// a helper macro for easier instatiation of the FlexCAN_T4 class
#define TEENSY_FLEXCAN(can) FlexCAN_T4<can, RX_SIZE_256, TX_SIZE_16>()

FLXCAN_CLASS
class TEENSY_CAN : public HardwareCAN
{
public:
	TEENSY_CAN(FLXCAN_T4& flexcan_ref, uint16_t shdn_pin = NC, uint16_t enable_pin = NC) 
		: HardwareCAN(), available_rx_msg({}), filter_(CanFilter(FilterType::MASK_ACCEPT_ALL)), flexcan(flexcan_ref)
	{
		init(NC, NC, shdn_pin, enable_pin);
	}

	bool init(uint16_t pinRX = NC, uint16_t pinTX = NC, uint16_t pinSHDN = NC, uint16_t pinEnable = NC) override;
	bool begin(int can_bitrate) override;
	void end() override;
	void filter(CanFilter filter) override;

	int write(CanMsg const &msg) override;
	CanMsg read() override;
	size_t available() override;

	CanStatus subscribe(void (*_messageReceiveCallback)() = nullptr) override;
	CanStatus unsubscribe() override;

private:
	CAN_message_t available_rx_msg = {};
	CanFilter filter_;
	FLXCAN_T4& flexcan;  // Reference to FlexCAN instance
	bool started_ = false;

	void applyFilter();
	static void flexcan_callback(const CAN_message_t &msg);
	CanStatus logStatus(char op, int status);
};


#define FLXCAN_OPT TEENSY_CAN<CAN_BUS>
#define FLXCAN_FUNC FLXCAN_CLASS


// ============================================================================
// Template Implementation (must be in header for templates)
// ============================================================================

FLXCAN_FUNC bool FLXCAN_OPT::init(uint16_t pinRX, uint16_t pinTX, uint16_t pinSHDN, uint16_t pinEnable)
{

	// Teensy 3.1/3.2: CAN0 on pins 3 (TX) and 4 (RX)
	// Teensy 3.6: CAN0 on pins 3/4, CAN1 on pins 33/34
    // Determine which CAN bus and set appropriate pins
	//
	// Teensy 4.x:
	// FlexCAN uses fixed hardware pins based on CAN_BUS template parameter
	// CAN1: TX=22, RX=23
	// CAN2: TX=1, RX=0
	// CAN3: TX=31, RX=30
	if constexpr (CAN_BUS == CAN0) {
        pinRX_ = 4;
        pinTX_ = 3;
	}else if constexpr (CAN_BUS == CAN1) {
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY32)
		pinRX_ = 33;
		pinTX_ = 34;
#else // teensy 4.x
		pinRX_ = 23;
		pinTX_ = 22;
#endif
	}else if constexpr (CAN_BUS == CAN2) {
		pinRX_ = 0;
		pinTX_ = 1;
    }else if constexpr (CAN_BUS == CAN3) {
		pinRX_ = 30;
		pinTX_ = 31;
	}else{
        return false; // Unsupported CAN bus    
    }

	pinSHDN_ = pinSHDN;
	pinEnable_ = pinEnable;

	if (pinEnable_ != NC) {
		pinMode(pinEnable_, OUTPUT);
	}
	if (pinSHDN_ != NC) {
		pinMode(pinSHDN_, OUTPUT);
	}

	mode = CAN_NORMAL;
	return true;
}

FLXCAN_FUNC bool FLXCAN_OPT::begin(int can_bitrate)
{

	_Serial->print("TEENSY_CAN: using RX pin ");
	_Serial->print(pinRX_);
	_Serial->print(" and TX pin ");
	_Serial->println(pinTX_);

	flexcan.begin();
	flexcan.setBaudRate(can_bitrate);
	flexcan.enableFIFO(true);
#ifdef CAN_DEBUG
	flexcan.mailboxStatus();
#endif
	
	if (pinSHDN_ != NC) {
		digitalWrite(pinSHDN_, LOW);
	}
	if (pinEnable_ != NC) {
		digitalWrite(pinEnable_, HIGH);
	}

	started_ = true;

	if (mode == CAN_LOOPBACK) {
		_Serial->println("TEENSY_CAN: enabling internal loopback mode");
		flexcan.enableLoopBack(true);
	}
	applyFilter();
	
	return true;
}

FLXCAN_FUNC void FLXCAN_OPT::end()
{
	if (pinSHDN_ != NC) {
		digitalWrite(pinSHDN_, HIGH);
	}
	if (pinEnable_ != NC) {
		digitalWrite(pinEnable_, LOW);
	}
	
	started_ = false;
}

FLXCAN_FUNC void FLXCAN_OPT::filter(CanFilter filter)
{
	filter_ = filter;
	if (started_) {
		applyFilter();
	}
}

FLXCAN_FUNC void FLXCAN_OPT::applyFilter()
{
	if (filter_.getType() == FilterType::MASK_REJECT_ALL) {
		flexcan.setFIFOFilter(REJECT_ALL);
	}
	else if (filter_.getType() == FilterType::MASK_ACCEPT_ALL) {
		flexcan.setFIFOFilter(ACCEPT_ALL);
	}
	else if (filter_.getType() == FilterType::MASK_STANDARD) { 
		flexcan.setFIFOUserFilter(filter_.getIdentifier(), 
								 filter_.getIdentifier() | (~filter_.getMask() & 0x7FF), STD, STD);
	} 
	else if (filter_.getType() == FilterType::MASK_EXTENDED) {
		// For extended ID filtering with mask
		uint32_t id_min = filter_.getIdentifier() & filter_.getMask();
		uint32_t id_max = filter_.getIdentifier() | (~filter_.getMask() & 0x1FFFFFFF);
		flexcan.setFIFOUserFilter(id_min, id_max, EXT, EXT);
	}
}

FLXCAN_FUNC int FLXCAN_OPT::write(CanMsg const &msg)
{
	CAN_message_t tx_msg;
	
#ifdef CAN_DEBUG
	_Serial->print("tx >> ");
	msg.printTo(*_Serial);
	_Serial->println();
#endif

	tx_msg.id = msg.isExtendedId() ? msg.getExtendedId() : msg.getStandardId();
	tx_msg.flags.extended = msg.isExtendedId() ? 1 : 0;
	tx_msg.flags.remote = msg.isRTR() ? 1 : 0;
	tx_msg.len = msg.data_length;
	memcpy(tx_msg.buf, msg.data, msg.data_length);
	return logStatus('w', flexcan.write(tx_msg));
}

FLXCAN_FUNC CanMsg FLXCAN_OPT::read()
{
	CAN_message_t rx_msg;
	// if available has been called before and it has read a message
	if (available_rx_msg.timestamp != 0) {
		rx_msg = available_rx_msg;
		available_rx_msg = {};
	} else if (logStatus('r', flexcan.read(rx_msg)) != CAN_OK) {
		return CanMsg(); // Empty message
	}

	uint32_t can_id = rx_msg.id;
	
	if (rx_msg.flags.extended) {
		can_id |= CanMsg::CAN_EFF_FLAG;
	}
	if (rx_msg.flags.remote) {
		can_id |= CanMsg::CAN_RTR_FLAG;
	}
	CanMsg msg(can_id, rx_msg.len, rx_msg.buf);
		
#ifdef CAN_DEBUG
		_Serial->print("rx >> ");
		msg.printTo(*_Serial);
		_Serial->println();
#endif
	
	return msg;
}

FLXCAN_FUNC size_t FLXCAN_OPT::available()
{
	// FlexCAN_T4 doesn't have a direct available() method
	// Try to peek at the mailbox
	return flexcan.read(available_rx_msg) ? 1 : 0;

}

FLXCAN_FUNC CanStatus FLXCAN_OPT::subscribe(void (*_messageReceiveCallback)())
{
	if (_messageReceiveCallback == nullptr) {
		return CAN_ERROR;
	}
	
	// HardwareCAN::_callbackFunction = _messageReceiveCallback;
	
	// Set up FlexCAN_T4 callback
	flexcan.onReceive(flexcan_callback);
	
	return CAN_OK;
}

FLXCAN_FUNC CanStatus FLXCAN_OPT::unsubscribe()
{
	// HardwareCAN::_callbackFunction = nullptr;
	// FlexCAN_T4 doesn't have a direct way to remove callback
	return CAN_OK;
}

FLXCAN_FUNC void FLXCAN_OPT::flexcan_callback(const CAN_message_t &msg)
{
	// if (HardwareCAN::_callbackFunction != nullptr) {
	// 	HardwareCAN::_callbackFunction();
	// }
}

FLXCAN_FUNC CanStatus FLXCAN_OPT::logStatus(char op, int status)
{
	if (status > 0) {
		return CAN_OK;
	}

#ifdef CAN_DEBUG
	_Serial->print("FlexCAN ");
	_Serial->print(op);
	_Serial->print(" error: ");
	_Serial->println(status);
#endif
	
	return CAN_ERROR;
}

#endif


