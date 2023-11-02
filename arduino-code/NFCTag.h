#pragma once

#include <string.h>
#include <stdint.h>
#include "SparkFun_ST25DV64KC_NDEF.h"
#include "keccak.h"
#include "constants.h"
#include "Wallet.h"

class NFCTag
{
public:
  enum MessageId
  {
    SIGN = 0x00,
    CONTRACT_ADDRESS = 0x01,

    INVALID_MESSAGE_FORMAT = 0xFC,
    INVALID_MESSAGE_LENGTH = 0xFD,
    UNKOWN_ERROR = 0xFE,
    UNKOWN_MESSAGE = 0xFF,
  };

  NFCTag(Wallet &wallet);

  bool init();
  bool isInitialized();

  bool handleMessage();

private:
  static uint8_t correctPassword[PASSWORD_LENGTH];
  static uint8_t wrongPassword[PASSWORD_LENGTH];

  bool initConfiguration();
  bool begin();
  bool fetchMessage();
  bool writeMessage(uint8_t *message, uint16_t messageLength);

  bool processSignMessage();

  bool processContractAddress();

  void updateNDEFRecords(const char *contractAddress);

  bool initialized;

  Wallet &wallet;
  SFE_ST25DV64KC_NDEF st25;

  uint8_t message[MAILBOX_LENGTH];
  uint16_t messageLength;

  uint8_t messageReply[MAILBOX_LENGTH];
  uint16_t messageReplyLength;
};
