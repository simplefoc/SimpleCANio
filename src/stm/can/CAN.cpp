
#include "CAN.h"
#if defined(HAL_CAN_MODULE_ENABLED) && defined(ARDUINO_ARCH_STM32)

extern "C" void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan);
extern "C" void CAN1_RX0_IRQHandler(void);  
extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

void (*HardwareCAN::_callbackFunction)() = nullptr;
CAN_RxHeaderTypeDef rxHeader_ = {};
CAN_TxHeaderTypeDef txHeader_ = {};
CAN_HandleTypeDef STM_CAN::hcan_ = {};


bool STM_CAN::init(uint16_t pinRX, uint16_t pinTX, uint16_t pinSHDN, uint16_t pinEnable) 
{
  hcan_.Instance = CAN1;
  pinRX_ = pinRX;
  pinTX_ = pinTX;
  pinSHDN_ = pinSHDN;
  pinEnable_ = pinEnable;


  //pinMode(pinRX_, INPUT_PULLUP); // <- on some boards like the stm32f403rg_disc1, this is required for CAN_LOOPBACK mode to work !?!

  // TWO APPROACHES
  // APPROACH 1: use pinmaps (this is the recommended approach and uses PeripheralPins.c)
  pin_function((PinName)pinRX_, pinmap_function((PinName)pinRX_, PinMap_CAN_RD));
  pin_function((PinName)pinTX_, pinmap_function((PinName)pinTX_, PinMap_CAN_TD));


  if (pinEnable != NC) 
  {
    pinMode(pinEnable, OUTPUT);
  }

  if (pinSHDN != NC)
  {
    pinMode(pinSHDN, OUTPUT);
  }

  mode = CAN_NORMAL;
  return true;
}

bool STM_CAN::begin(int bitrate)
{
  if (bitrate > 1000000)
  {
    failAndBlink(CAN_ERROR_BITRATE_TOO_HIGH);
  }

  uint32_t clockFreq = HAL_RCC_GetPCLK1Freq();
  // uint32_t clockFreq = HAL_RCC_GetSysClockFreq();

  CanTiming timing = solveCanTiming(clockFreq, bitrate);
  CAN_InitTypeDef *init = &(hcan_.Init);

  init->Prescaler = (uint16_t)timing.prescaler;
  init->Mode = mode == CanMode::CAN_LOOPBACK ? CAN_MODE_LOOPBACK : CAN_MODE_NORMAL;
  init->SyncJumpWidth = CAN_SJW_3TQ; // <- Maybe we should make this configurable? 3 is fairly safe.
  init->TimeSeg1 = (timing.tseg1 - 1) << CAN_BTR_TS1_Pos;
  init->TimeSeg2 = (timing.tseg2 - 1) << CAN_BTR_TS2_Pos;
  init->TimeTriggeredMode = DISABLE;
  init->AutoBusOff = DISABLE;
  init->AutoWakeUp = DISABLE;
  init->AutoRetransmission = DISABLE;
  init->ReceiveFifoLocked = DISABLE;
  init->TransmitFifoPriority = DISABLE;

  logStatus('i',
            HAL_CAN_Init(&hcan_));
  if (pinSHDN_ != NC)
  {
    digitalWrite(pinSHDN_, LOW);
  }
  if (pinEnable_ != NC)
  {
    digitalWrite(pinEnable_, HIGH);
  }

  started_ = true;
  applyFilter();

  return logStatus('s',
                   HAL_CAN_Start(&hcan_));
}

void STM_CAN::end()
{
  if (pinSHDN_ != NC)
  {
    digitalWrite(pinSHDN_, HIGH);
  }
  if (pinEnable_ != NC)
  {
    digitalWrite(pinEnable_, LOW);
  }
  logStatus('x',
            HAL_CAN_Stop(&hcan_));

  logStatus('d',
            HAL_CAN_DeInit(&hcan_));
  started_ = false;
}

void STM_CAN::filter(CanFilter filter)
{
  filter_ = filter;
}

void STM_CAN::applyFilter()
{

  // Unimplemented: the ability to filter on IDE or RTR bits (personally not that useful?!)

  static uint32_t filterIdHigh = 0xffff;
  static uint32_t filterIdLow = 0xffff; // <-- all digits must match
  static uint32_t filterMaskHigh = 0xffff;
  static uint32_t filterMaskLow = 0xffff;
  uint8_t filterIndex = 0;

  if (filter_.getType() == FilterType::MASK_STANDARD)
  {
    filterIdHigh = filter_.getIdentifier() << 5; // make room for IDE, RTR bits (+ 3 unused)
    filterMaskHigh = filter_.getMask() << 5;
  }
  else if (filter_.getType() == FilterType::MASK_EXTENDED)
  {
    filterIdLow = (filter_.getIdentifier() & 0x0000ffff) << 3; // make room for IDE, RTR bit (+ 1 unused bit)
    filterIdHigh = filter_.getIdentifier() >> 16;
    filterMaskLow = (filter_.getMask() & 0x0000ffff) << 3;
    filterMaskHigh = filter_.getMask() >> 16;
  }
  else if (filter_.getType() == FilterType::ACCEPT_ALL)
  {
    filterIdLow = 0xffff;
    filterIdHigh = 0x0000; //<- no digits have to match
    filterMaskLow = 0xffff;
    filterMaskHigh = 0x0000;
  }

#ifdef CAN_DEBUG
  _Serial->println("###### FILTER ######");
  _Serial->print("filterType: ");
  _Serial->print(filter_.getType());
  _Serial->print(" (identifier: ");
  _Serial->print(filter_.getIdentifier(), HEX);
  _Serial->print(",  mask: ");
  _Serial->print(filter_.getMask(), HEX);
  _Serial->println(")");
  _Serial->print("registers (filterIdLow: ");
  _Serial->print(filterIdLow, HEX);
  _Serial->print(", filterIdHigh: ");
  _Serial->print(filterIdHigh, HEX);
  _Serial->print(", filterMaskLow: ");
  _Serial->print(filterMaskLow, HEX);
  _Serial->print(", filterMaskHigh: ");
  _Serial->print(filterMaskHigh, HEX);
  _Serial->println(")");
#endif

  CAN_FilterTypeDef filterDef = {
      .FilterIdHigh = filterIdHigh,
      .FilterIdLow = filterIdLow,
      .FilterMaskIdHigh = filterMaskHigh,
      .FilterMaskIdLow = filterMaskLow,
      .FilterFIFOAssignment = CAN_FILTER_FIFO0,
      .FilterBank = filterIndex,
      .FilterMode = CAN_FILTERMODE_IDMASK,
      .FilterScale = filter_.getType() == FilterType::MASK_EXTENDED ? CAN_FILTERSCALE_32BIT : CAN_FILTERSCALE_16BIT,
      .FilterActivation = filter_.getType() == FilterType::REJECT_ALL ? DISABLE : ENABLE,
  };
  logStatus('f',
            HAL_CAN_ConfigFilter(&hcan_, &filterDef));
}

CanStatus STM_CAN::subscribe(void (*_messageReceiveCallback)())
{
  HardwareCAN::_callbackFunction = _messageReceiveCallback;
  return logStatus('a',
                   HAL_CAN_ActivateNotification(&hcan_, CAN_IT_RX_FIFO0_MSG_PENDING));
}

CanStatus STM_CAN::unsubscribe()
{
  HardwareCAN::_callbackFunction = nullptr;
  return logStatus('u',
                   HAL_CAN_DeactivateNotification(&hcan_, CAN_IT_RX_FIFO0_MSG_PENDING));
}

int STM_CAN::write(CanMsg const &txMsg)
{
#ifdef CAN_DEBUG
  _Serial->print("tx: ");
  txMsg.printTo(*_Serial);
  _Serial->println();

#endif

  txHeader_ = {
      .StdId = txMsg.isExtendedId() ? 0 : txMsg.getStandardId(),
      .ExtId = txMsg.isExtendedId() ? txMsg.getExtendedId() : 0,
      .IDE = txMsg.isExtendedId() ? CAN_ID_EXT : CAN_ID_STD,
      .RTR = txMsg.isRTR() ? CAN_RTR_REMOTE : CAN_RTR_DATA,
      .DLC = txMsg.data_length,
  };
  uint32_t usedMailbox;

  return logStatus('t',
                   HAL_CAN_AddTxMessage(&hcan_, &txHeader_, (uint8_t *)txMsg.data, &usedMailbox));
}

CanMsg STM_CAN::read()
{

  memset(&rxHeader_, 0, sizeof(rxHeader_)); // <-zero before reusing rxHeader_

  uint8_t data[MAX_DATA_LENGTH];

  if (logStatus('r',
                HAL_CAN_GetRxMessage(&hcan_, CAN_RX_FIFO0, &rxHeader_, data)) != CAN_OK)
  {
    return CanMsg();
  }
  else
  {
    CanMsg const rxMsg(
        (rxHeader_.IDE == CAN_ID_EXT) ? CanExtendedId(rxHeader_.ExtId, rxHeader_.RTR)
                                      : CanStandardId(rxHeader_.StdId, rxHeader_.RTR),
        rxHeader_.DLC,
        data);

#ifdef CAN_DEBUG
    _Serial->print("rx: ");
    rxMsg.printTo(*_Serial);
    _Serial->println();
#endif
    return rxMsg;
  }
}

size_t STM_CAN::available()
{
  return HAL_CAN_GetRxFifoFillLevel(&hcan_, CAN_RX_FIFO0);
}

/*
#####################
HAL CALBACK FUNCTIONS
#####################
*/
void HAL_CAN_MspInit(CAN_HandleTypeDef *hfdcan)
{

  __HAL_RCC_CAN1_CLK_ENABLE();
  HAL_NVIC_SetPriority(CAN1_RX0_IRQn,  9, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
}

void CAN1_RX0_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&STM_CAN::hcan_);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  
  if (HardwareCAN::_callbackFunction == nullptr)
  {
    return;
  }
  HardwareCAN::_callbackFunction();
}

CanStatus STM_CAN::logStatus(char op, HAL_StatusTypeDef status)
{
#ifdef CAN_DEBUG
  if (status != HAL_OK)
  {
    _Serial->print("ERROR (");
    _Serial->print(op);
    _Serial->print(") ");
    _Serial->print(status);
    _Serial->print(", can_state: ");
    _Serial->print(HAL_CAN_GetState(&hcan_), HEX); // google HAL_CAN_StateTypeDef e.g. 5 = HAL_CAN_STATE_ERROR
    _Serial->print(", can_error: ");
    _Serial->println(HAL_CAN_GetError(&hcan_), HEX); // google CAN_HandleTypeDef::ErrorCode  e.g. 0x00020000U = HAL_CAN_ERROR_TIMEOUT
  }
#endif
  return status == HAL_OK ? CAN_OK : CAN_ERROR;
}

#endif