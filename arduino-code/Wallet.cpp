#include "Wallet.h"
#include "uECC.h" // https://github.com/kmackay/micro-ecc/tree/static
#include "crypto-util.h"
#include "random.h"
#include <Arduino.h>
#include <EEPROM.h>

static void printHex(const uint8_t buffer[], const uint8_t len)
{
  Serial.print("0x");
  for (uint8_t i = 0; i < len; i++)
    Serial.printf("%02X", buffer[i]);
  Serial.println("");
}

bool Wallet::initWithStringifiedPrivateKey(const char *privateKeyAsString)
{
  Serial.println("");
  Serial.println("Initializing keys with stringified private key...");

  hex2bin(privateKeyAsString, privKey);
  initialized = uECC_compute_public_key(privKey, pubKey) != 0;
  if (!initialized)
  {
    Serial.println("Failed.");
    return false;
  }
  calculateLuksoAddress();

  Serial.print("Private key: ");
  printHex(privKey, PRIVATE_KEY_LENGTH);
  Serial.print("Public key: ");
  printHex(pubKey, PUBLIC_KEY_LENGTH);
  Serial.print("Lukso address: ");
  Serial.println(luksoAddress.c_str());
  Serial.println("Done initializing.");

  return true;
}

Wallet::Wallet() : initialized(false), luksoAddress(""), privKey(), pubKey()
{
  uECC_set_rng(&trueRandomNumberGenerator);
}

bool Wallet::init()
{
  return (initialized = loadOrCreateKeys());
}

bool Wallet::createKeys()
{
#ifdef DEBUG
  Serial.println("");
  Serial.println("Creating keys...");
#endif

  if (uECC_make_key(pubKey, privKey) == 0)
  {
#ifdef DEBUG
    Serial.println("Failed.");
#endif
    return false;
  }

  calculateLuksoAddress();

#ifdef DEBUG
  Serial.print("Private key: ");
  printHex(privKey, PRIVATE_KEY_LENGTH);
  Serial.print("Public key: ");
  printHex(pubKey, PUBLIC_KEY_LENGTH);
  Serial.print("Lukso address: ");
  Serial.println(luksoAddress.c_str());
  Serial.println("Done creating.");
#endif
  return true;
}

void Wallet::loadKeys()
{
#ifdef DEBUG
  Serial.println("");
  Serial.println("Loading keys...");
#endif

  for (uint8_t i = 0; i < PRIVATE_KEY_LENGTH; i++)
    privKey[i] = EEPROM.read(EEPROM_PRIVATE_KEY_ADDRESS + i);
  for (uint8_t i = 0; i < PUBLIC_KEY_LENGTH; i++)
    pubKey[i] = EEPROM.read(EEPROM_PUBLIC_KEY_ADDRESS + i);

  calculateLuksoAddress();

#ifdef DEBUG
  Serial.print("Private key: ");
  printHex(privKey, PRIVATE_KEY_LENGTH);
  Serial.print("Public key: ");
  printHex(pubKey, PUBLIC_KEY_LENGTH);
  Serial.print("Lukso address: ");
  Serial.println(luksoAddress.c_str());
  Serial.println("Done loading.");
#endif
}

void Wallet::saveKeys()
{
#ifdef DEBUG
  Serial.println("");
  Serial.println("Saving keys...");
#endif

  EEPROM.write(EEPROM_KEYS_INITIALIZED_ADDRESS, EEPROM_KEYS_INITIALIZED_MAGIC_VALUE);
  for (uint8_t i = 0; i < PRIVATE_KEY_LENGTH; i++)
    EEPROM.write(EEPROM_PRIVATE_KEY_ADDRESS + i, privKey[i]);
  for (uint8_t i = 0; i < PUBLIC_KEY_LENGTH; i++)
    EEPROM.write(EEPROM_PUBLIC_KEY_ADDRESS + i, pubKey[i]);

  calculateLuksoAddress();

#ifdef DEBUG
  Serial.print("Private key: ");
  printHex(privKey, PRIVATE_KEY_LENGTH);
  Serial.print("Public key: ");
  printHex(pubKey, PUBLIC_KEY_LENGTH);
  Serial.print("Lukso address: ");
  Serial.println(luksoAddress.c_str());
  Serial.println("Done saving.");
#endif
}

bool Wallet::loadOrCreateKeys()
{
#ifdef DEBUG
  Serial.print("EEPROM STATE: ");
  Serial.printf("%02X\n", EEPROM.read(EEPROM_KEYS_INITIALIZED_ADDRESS));
#endif
  if (EEPROM.read(EEPROM_KEYS_INITIALIZED_ADDRESS) == EEPROM_KEYS_INITIALIZED_MAGIC_VALUE)
  {
    loadKeys();
    return true;
  }
  else
  {
    if (!createKeys())
      return false;

    saveKeys();
    return true;
  }
}

bool Wallet::signHashedMessage(const uint8_t messageHash[KECCAK_HASH_LENGTH], uint8_t signature[SIGNATURE_LENGTH])
{
#ifdef DEBUG
  Serial.print("Signing message hash: ");
  printHex(messageHash, KECCAK_HASH_LENGTH);
#endif
  if (!initialized)
  {
#ifdef DEBUG
    Serial.println("Failed. Keys are not initialized");
#endif
    return false;
  }

  if (uECC_sign(privKey, messageHash, signature, 0) == 0)
  {
#ifdef DEBUG
    Serial.println("Failed.");
#endif
    return false;
  }
#ifdef DEBUG
  Serial.print("Signature: ");
  printHex(signature, SIGNATURE_LENGTH);
#endif
  return true;
}

bool Wallet::signUnhashedMessage(const char* message, uint8_t hashedMessage[KECCAK_HASH_LENGTH], uint8_t signature[SIGNATURE_LENGTH])
{
#ifdef DEBUG
  Serial.print("Signing message: "); Serial.println(message);
#endif
  if (!initialized)
  {
#ifdef DEBUG
    Serial.println("Failed. Keys are not initialized");
#endif
    return false;
  }

  hex2bin(keccak256(message).c_str(), hashedMessage);
  return signHashedMessage(hashedMessage, signature);
}

void Wallet::calculateLuksoAddress()
{
  luksoAddress = keccak256((void *)pubKey, PUBLIC_KEY_LENGTH).substr(24);
  const char *checksumReference = keccak256((void *)luksoAddress.c_str(), 40).c_str();
  for (uint8_t i = 0; i < 40; i++)
  {
    if (checksumReference[i] > '7' && luksoAddress[i] >= 'a')
      luksoAddress[i] = luksoAddress[i] - ('a' - 'A');
  }
  luksoAddress = ("0x" + luksoAddress);
}

bool Wallet::isInitialized()
{
  return initialized;
}
