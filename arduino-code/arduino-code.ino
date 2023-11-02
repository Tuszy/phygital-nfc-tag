#include "constants.h"
#include "Wallet.h"
#include "NFCTag.h"
#include "crypto-util.h"

#define GPO_PIN A1

extern "C" void SystemClock_Config(void);

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

/**
    @brief System Clock Configuration
    @retval None
*/
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
  * in the RCC_OscInitTypeDef structure.
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}
