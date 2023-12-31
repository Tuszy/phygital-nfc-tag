#include "NFCTag.h"
#include "crypto-util.h"
#include <Wire.h>
#include <EEPROM.h>

uint8_t NFCTag::correctPassword[PASSWORD_LENGTH] = {0x0};
uint8_t NFCTag::wrongPassword[PASSWORD_LENGTH] = {0x1};

NFCTag::NFCTag(Wallet &wallet)
    : initialized(false), wallet(wallet), message(), messageLength(0)
{
}

bool NFCTag::initConfiguration()
{
#ifdef DEBUG
  Serial1.println("Initializing ST25DV configuration...");
#endif

  // open security session with correct password
  if (!st25.openI2CSession(correctPassword))
  {
#ifdef DEBUG
    Serial1.println("Failed to open security session with correct password");
#endif
    return false;
  }

  if (!st25.setLockCCFile(false))
  {
#ifdef DEBUG
    Serial1.println("Failed to lock cc file");
#endif
    return false;
  }

#if CC_FILE_SIZE == 4
  if (!st25.writeCCFile4Byte())
#else
  if (!st25.writeCCFile8Byte())
#endif
  {
#ifdef DEBUG
    Serial1.println("Failed to write cc file");
#endif
    return false;
  }

  if (!st25.setLockCCFile(true))
  {
#ifdef DEBUG
    Serial1.println("Failed to lock cc file");
#endif
    return false;
  }

  // enable energy harvesting
  if (!st25.setEH_MODEBit(false))
  {
#ifdef DEBUG
    Serial1.println("Failed to enable energy harvesting");
#endif
    return false;
  }

  // enable Mailbox (Fast transfer mode)
  if (!st25.enableMailbox())
  {
#ifdef DEBUG
    Serial1.println("Failed to enable mailbox");
#endif
    return false;
  }

  // Disable writing through RF
  if (!st25.setAreaRfRwProtection(1, SF_ST25DV_RF_RW_PROTECTION::RF_RW_READ_SECURITY_WRITE_NEVER))
  {
#ifdef DEBUG
    Serial1.println("Failed to disable writing through RF");
#endif
    return false;
  }

  // Disable all interrupts except for 'rf puts a message into the mailbox' (interrupt service service)
  if (!st25.setGPO1Bit(BIT_GPO1_FIELD_CHANGE_EN | BIT_GPO1_RF_USER_EN | BIT_GPO1_RF_ACTIVITY_EN | BIT_GPO1_RF_INTERRUPT_EN | BIT_GPO1_RF_GET_MSG_EN | BIT_GPO1_RF_WRITE_EN, false))
  {
#ifdef DEBUG
    Serial1.println("Failed to disable GPO1 bits: BIT_GPO1_FIELD_CHANGE_EN | BIT_GPO1_RF_USER_EN | BIT_GPO1_RF_ACTIVITY_EN | BIT_GPO1_RF_INTERRUPT_EN | BIT_GPO1_RF_GET_MSG_EN | BIT_GPO1_RF_WRITE_EN");
#endif
    return false;
  }

  if (!st25.setGPO1Bit(BIT_GPO1_RF_PUT_MSG_EN | BIT_GPO1_GPO_EN, true))
  {
#ifdef DEBUG
    Serial1.println("Failed to enable GPO1 bits: BIT_GPO1_RF_PUT_MSG_EN | BIT_GPO1_GPO_EN");
#endif
    return false;
  }

  if (!st25.lockConfiguration())
  {
#ifdef DEBUG
    Serial1.println("Failed to lock configuration");
#endif
    return false;
  }

  // close security session with incorrect password
  st25.openI2CSession(wrongPassword);

  updateNDEFRecords(nullptr);

  EEPROM.write(EEPROM_NFC_TAG_INITIALIZED_ADDRESS, EEPROM_NFC_TAG_INITIALIZED_MAGIC_VALUE);

#ifdef DEBUG
  Serial1.println("Initialization done.");
#endif

  return true;
}

bool NFCTag::init()
{
  if (!begin())
    return false;

  if (EEPROM.read(EEPROM_NFC_TAG_INITIALIZED_ADDRESS) != EEPROM_NFC_TAG_INITIALIZED_MAGIC_VALUE)
  {
    if (!initConfiguration())
    {
#ifdef DEBUG
      Serial1.println("Failed to initialize configuration");
#endif
      return false;
    }
  }

  if (!st25.setMailboxActive(true))
  {
#ifdef DEBUG
    Serial1.println("Failed to activate mailbox");
#endif
    return false;
  }

  initialized = true;
  return true;
}

bool NFCTag::begin()
{
  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  Wire.begin();

  if (!st25.begin(Wire))
  {
#ifdef DEBUG
    Serial1.println("NFC tag ST25 not detected. Freezing...");
#endif
    return false;
  }

#ifdef DEBUG
  Serial1.println("NFC tag ST25 connected.");
#endif

  return true;
}

bool NFCTag::writeMessage(uint8_t *message, uint16_t messageLength)
{
  if (!initialized)
    return false;
  if (messageLength > MAILBOX_LENGTH)
    return false;
  return st25.writeToMailbox(message, messageLength);
}

bool NFCTag::fetchMessage()
{
  if (!initialized)
    return false;
  uint16_t newMessageLength = st25.getMailboxMessageLength();
  if (newMessageLength == 0)
    return false;

  if (!st25.readFromMailbox(message, newMessageLength))
    return false;

  messageLength = newMessageLength;

  return true;
}

bool NFCTag::handleMessage()
{
  if (!initialized)
    return false;
  if (!fetchMessage() || messageLength == 0)
    return false;

  if (messageLength == 1)
  {
    messageReply[0] = INVALID_MESSAGE_LENGTH;
    writeMessage(messageReply, 1);
    return true;
  }

  switch (message[0])
  {
  case SIGN:
    processSignMessage();
    break;
  case CONTRACT_ADDRESS:
    processContractAddress();
    break;
  default:
    messageReply[0] = UNKOWN_MESSAGE;
    writeMessage(messageReply, 1);
  }

  return true;
}

bool NFCTag::processSignMessage()
{
  if (messageLength - 1 != KECCAK_HASH_LENGTH)
  {
    messageReply[0] = INVALID_MESSAGE_LENGTH;
    writeMessage(messageReply, 1);
    return false;
  }

  if (!wallet.signHashedMessage(&message[1], &messageReply[1]))
  {
    messageReply[0] = UNKOWN_ERROR;
    writeMessage(messageReply, 1);
    return false;
  }

  messageReply[0] = SIGN;
  messageReplyLength = SIGNATURE_LENGTH + 1;
  writeMessage(messageReply, messageReplyLength);

  return true;
}

bool NFCTag::processContractAddress()
{
  if (messageLength - 1 != LUKSO_ADDRESS_AS_STRING_LENGTH)
  {
    messageReply[0] = INVALID_MESSAGE_LENGTH;
    writeMessage(messageReply, 1);
    return false;
  }

  uint8_t newContractAddress[LUKSO_ADDRESS_AS_STRING_LENGTH + 1];
  message[LUKSO_ADDRESS_AS_STRING_LENGTH + 2] = 0;
  memcpy(newContractAddress, &message[1], LUKSO_ADDRESS_AS_STRING_LENGTH + 1);

  if (newContractAddress[0] != '0' || (newContractAddress[1] != 'X' && newContractAddress[1] != 'x'))
  {
    messageReply[0] = INVALID_MESSAGE_FORMAT;
    writeMessage(messageReply, 1);
    return false;
  }

  for (uint8_t i = 0; i < LUKSO_ADDRESS_AS_STRING_LENGTH; i++)
  {
    if (!((newContractAddress[i] >= 'A' && newContractAddress[i] <= 'Z') || (newContractAddress[i] >= 'a' && newContractAddress[i] <= 'z') || (newContractAddress[i] >= '0' && newContractAddress[i] <= '9')))
    {
      messageReply[0] = INVALID_MESSAGE_FORMAT;
      writeMessage(messageReply, 1);
      return false;
    }
  }

  updateNDEFRecords((const char *)newContractAddress);

  messageReply[0] = CONTRACT_ADDRESS;
  writeMessage(messageReply, 1);
  return true;
}

void NFCTag::updateNDEFRecords(const char *contractAddress)
{
  uint16_t memLoc = st25.getCCFileLen();
  st25.setMailboxActive(false);
  st25.writeNDEFURI("phygital.tuszy.com", SFE_ST25DV_NDEF_URI_ID_CODE_HTTPS_WWW, &memLoc, true, false);
  st25.writeNDEFText(wallet.getLuksoAddress(), &memLoc, false, contractAddress == nullptr);
  if (contractAddress != nullptr)
    st25.writeNDEFText(contractAddress, &memLoc, false, true);
  st25.setMailboxActive(true);
}

bool NFCTag::isInitialized()
{
  return initialized;
}
