# Phygital NFC Tag

App-specific, custom-developed NFC tag that identifies a non-forgeable phygital.

## Remarks

Since I am just a plain software dev with zero experience in pcb design please keep in mind that this NFC Tag is just a prototype. I am a 100% sure, that it could have been done better, but I created it to the best of my knowledge and conscience. It does not work perfect all the time, but for a proof of concept it is good enough and with the gained knowledge I would have done it better. 

## Concept

The idea is to create a custom battery-less uniquely identifiable NFC tag that will not be forgeable. 
- The absence of the battery need will be implemented through NFC RF field [energy harvesting](https://de.wikipedia.org/wiki/Energy_Harvesting)
- The unforgeability will be reached by generating a random unique secp256k1 key pair on the first boot. It will be saved within the flash/eeprom of the µC (similar to a cold wallet). The public key will be visible as an [NDEF record](https://www.oreilly.com/library/view/beginning-nfc/9781449324094/ch04.html), but the private key will never leave the µC, it will only be used to sign keccak256 hashes coming from the [Phygital App](https://github.com/Tuszy/phygital-app). NFC tag signing opens up a wide range of possible applications. E.g. in our case it is used to mint the phygital on Lukso's blockchain (=> "Phygitalization").

### Challenges
- Finding a dual-interface NFC IC that can communicate with and power a µC through energy harvesting so that no battery is necessary
- Finding a ultra low power but performant µC that can be powered by max 2-3V with 2-3mA resulting from energy harvesting
- Implementing or finding a library that implements a small footprint secp256k1 elliptic curve with key pair generation and signing capability
- Implementing or finding a library that implements a small footprint keccak256 hashing algorithm
- Creating a custom PCB which incorporates a small 13.56MHz RF antenna, the ultra low power µC and the dual-interface NFC IC

### Expected Workflow/Usage
1. Flash µC with custom phygital firmware (arduino code)
2. First boot of µC generates secp256k1 key pair and saves it into flash/eeprom
3. Use [Phygital App](https://github.com/Tuszy/phygital-app) with NFC capable mobile phone:
   1.  to read public key (= Lukso address) of NFC tag
   2.  to write and read [LSP7](https://docs.lukso.tech/standards/nft-2.0/LSP7-Digital-Asset/)/[LSP8](https://docs.lukso.tech/standards/nft-2.0/LSP8-Identifiable-Digital-Asset) contract address of phygital
   3.  to sign keccak256 hashed message with private key of NFC tag (used for minting and verification of phygital)

## Software for Dev

- [Arduino IDE](https://www.arduino.cc/) - Used for developing the phygital firmware
- [STM32Duino](https://github.com/stm32duino) - Used for integrating the STM32 µC into the Arduino IDE
- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) - Used for setting the clock frequency of the STM32 µC (ultra low power)

### Libraries

- [micro-ecc](https://github.com/kmackay/micro-ecc/tree/static) - Used for secp256k1 elliptic curve asymmetric key pair generation and signing of keccak256 hashes (Actually I had to extend the signing algorithm to be able to recover addresses from the signatures since they were only 64 bytes long)
- [keccak](https://github.com/kvhnuke/Ethereum-Arduino/blob/master/Ethereum-Arduino/keccak.cpp) - Used for computing keccak256 hashes
- [SparkFun ST25DV64KC Arduino Library](https://github.com/sparkfun/SparkFun_ST25DV64KC_Arduino_Library/tree/main) - Used for interacting with the [SparkFun Qwiic Dynamic NFC/RFID Tag](https://www.sparkfun.com/products/21274)
- [Serial](https://www.arduino.cc/reference/en/language/functions/communication/serial/) - Used for debug logs
- [Wire](https://www.arduino.cc/reference/en/language/functions/communication/wire/) - Used for I2C communication between the STM32 µC and ST25DV64KC chip 

## Electronic components

### Dev and test
- [Nucleo-32 L432KC](https://www.st.com/en/evaluation-tools/nucleo-l432kc.html) - Ultra low power µC devboard based on the [STM32L432KC](https://www.st.com/en/microcontrollers-microprocessors/stm32l432kc.html)
- [SparkFun Qwiic Dynamic NFC/RFID Tag](https://www.sparkfun.com/products/21274) - Dynamic NFC/RFID tag based on the [ST25DV64KC](https://www.st.com/en/nfc/st25dv64kc.html)

### Production
- [STLINK-V3MINIE](https://www.st.com/en/development-tools/stlink-v3minie.html) - In-circuit debugger and programmer for STM32 µC
- [STM32L432KC](https://www.st.com/en/microcontrollers-microprocessors/stm32l432kc.html) - Ultra low power STM32L4 µC - [Purchase link](https://jlcpcb.com/partdetail/Stmicroelectronics-STM32L432KCU6/C1337280) 
- [ST25DV64KC](https://www.st.com/en/nfc/st25dv64kc.html) - Dual-interface dynamic NFC/RFID tag with energy harvesting capability - [Purchase link](https://jlcpcb.com/partdetail/Stmicroelectronics-ST25DV64KCIE8T3/C3304589)

## PCB services and tools
- [JLCPCB](https://jlcpcb.com/) - PCB manufacturing and assembly service
- [EasyEDA](https://easyeda.com/) - PCB designer