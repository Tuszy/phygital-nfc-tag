#pragma once

#include <string.h>
#include "keccak.h" // https://github.com/kvhnuke/Lukso-Arduino/blob/master/Lukso-Arduino/libs/keccak.h
#include "constants.h"

class Wallet
{
  public:
    Wallet();

    bool init();
    bool initWithStringifiedPrivateKey(const char *privateKeyAsString);
    bool isInitialized();

    bool signHashedMessage(const uint8_t messageHash[KECCAK_HASH_LENGTH], uint8_t signature[SIGNATURE_LENGTH]);
    bool signUnhashedMessage(const char* message, uint8_t hashedMessage[KECCAK_HASH_LENGTH], uint8_t signature[SIGNATURE_LENGTH]);

    inline const char *getLuksoAddress()
    {
      return luksoAddress.c_str();
    }

  private:
    bool initialized;

    bool loadOrCreateKeys();
    bool createKeys();
    void loadKeys();
    void saveKeys();

    void calculateLuksoAddress();

    Keccak keccak256;
    uint8_t privKey[PRIVATE_KEY_LENGTH];
    uint8_t pubKey[PUBLIC_KEY_LENGTH];
    std::string luksoAddress;
};
