#include "hal_conf_extra.h"
#include "constants.h"
#include "Wallet.h"
#include "NFCTag.h"
#include "crypto-util.h"
#include "uECC.h"

#define GPO_PIN A1

extern "C" void SystemClock_Config(void);
extern "C" void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng);
extern "C" void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng);

RNG_HandleTypeDef hrng;

int trueRandomNumberGenerator(uint8_t *dest, unsigned size)
{
  uint32_t aRandom32bit;
  for (unsigned i = 0; i < size; i += 4)
  {
    while (HAL_RNG_GenerateRandomNumber(&hrng, &aRandom32bit) != HAL_OK)
      delay(1);
    for (uint8_t j = 0; j < 4 && i + j < size; j++)
    {
      *dest = (uint8_t)(aRandom32bit >> (j * 8));
      ++dest;
    }
  }

  return 1;
}

Wallet wallet;
NFCTag nfcTag(wallet);

// Interrupt Service Routine
void handleMessage()
{
  nfcTag.handleMessage();
}

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Booting...");
#endif

  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
#ifdef DEBUG
    Serial.println("Booting failed. Could not init random number generator. Freezing...");
#endif
    while (true)
      delay(1); // Infinite loop
  }

  if (!wallet.init())
  {
#ifdef DEBUG
    Serial.println("Booting failed. Could not init wallet. Freezing...");
#endif
    while (true)
      delay(1); // Infinite loop
  }

  if (!nfcTag.init())
  {
#ifdef DEBUG
    Serial.println("Booting failed. Could not init nfc tag. Freezing...");
#endif
    while (true)
      delay(1); // Infinite loop
  }

  pinMode(GPO_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(GPO_PIN), handleMessage, CHANGE);

#ifdef DEBUG
  Serial.println("Booting succeeded.");
#endif
}

void loop()
{
  delay(1);  
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
     in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if (hrng->Instance == RNG)
  {
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RNG;
    PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_PLLSAI1;
    PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
    PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
    PeriphClkInit.PLLSAI1.PLLSAI1N = 16;
    PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
    PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
    PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
    PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_RNG_CLK_ENABLE();
  }
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
  if (hrng->Instance == RNG)
  {
    /* Peripheral clock disable */
    __HAL_RCC_RNG_CLK_DISABLE();

    /* Enable RNG reset state */
    __HAL_RCC_RNG_FORCE_RESET();

    /* Release RNG from reset state */
    __HAL_RCC_RNG_RELEASE_RESET();
  }
}
