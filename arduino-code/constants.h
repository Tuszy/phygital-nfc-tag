#pragma once

#define SERIAL_UART_INSTANCE 1
#define ENABLE_HWSERIAL1 1
#define USART1 1

#include <HardwareSerial.h>

#define DEBUG

#define PRIVATE_KEY_LENGTH 32
#define PUBLIC_KEY_LENGTH 64
#define SIGNATURE_LENGTH 65
#define LUKSO_ADDRESS_LENGTH 20
#define LUKSO_ADDRESS_AS_STRING_LENGTH 42

#define MAILBOX_LENGTH 256
#define PASSWORD_LENGTH 8

#define EEPROM_KEYS_INITIALIZED_MAGIC_VALUE 0xaa
#define EEPROM_KEYS_INITIALIZED_ADDRESS 0
#define EEPROM_PRIVATE_KEY_ADDRESS 1
#define EEPROM_PUBLIC_KEY_ADDRESS 33

#define EEPROM_NFC_TAG_INITIALIZED_MAGIC_VALUE 0xaa
#define EEPROM_NFC_TAG_INITIALIZED_ADDRESS 97

#define NDEF_URI_PREFIX_LENGTH 7
#define NDEF_URI_POSTFIX_LENGTH 1
#define NDEF_TEXT_PREFIX_LENGTH 7
#define NDEF_TEXT_LANGUAGE_CODE_PREFIX_LENGTH 3

extern HardwareSerial Serial1;
