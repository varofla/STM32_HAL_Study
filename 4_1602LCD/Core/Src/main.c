/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//----- us���� ��� �Լ� (https://slt.pw/iyFFwF1 ����)
void delay_us(uint16_t time) {
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while((__HAL_TIM_GET_COUNTER(&htim1))<time);
}

//----- EN�� �޽� ���� �Լ�
void pulse_en() {
    HAL_GPIO_WritePin(EN_1602_GPIO_Port, EN_1602_Pin, 1);
    delay_us(20);
    HAL_GPIO_WritePin(EN_1602_GPIO_Port, EN_1602_Pin, 0);
    delay_us(20);
}

//----- LCD ���÷��̷� 4bit ������ ����
void lcd_send_4bit(char data) {
    HAL_GPIO_WritePin(D7_1602_GPIO_Port, D7_1602_Pin, ((data >> 3) & 0x01)); // 4��Ʈ �� 4��° ��Ʈ ����Ʈ, ����ŷ
    HAL_GPIO_WritePin(D6_1602_GPIO_Port, D6_1602_Pin, ((data >> 2) & 0x01)); // 4��Ʈ �� 3��° ��Ʈ ����Ʈ, ����ŷ
    HAL_GPIO_WritePin(D5_1602_GPIO_Port, D5_1602_Pin, ((data >> 1) & 0x01)); // 4��Ʈ �� 2��° ��Ʈ ����Ʈ, ����ŷ
    HAL_GPIO_WritePin(D4_1602_GPIO_Port, D4_1602_Pin, (data & 0x01)); // 4��Ʈ �� 1��° ��Ʈ ����ŷ
    pulse_en();
}

//----- LCD ���÷��̷� 8bit ������ ����
void lcd_send_8bit(char data) {
	lcd_send_4bit((data >> 4) & 0x0F); // ���� 4��Ʈ ����
	lcd_send_4bit(data & 0x0F);        // ���� 4��Ʈ ����
}

//----- LCD ���÷��� Ŀ�� ����
void lcd_set_cur(int row, int col) {
	HAL_GPIO_WritePin(RS_1602_GPIO_Port, RS_1602_Pin, 0); // ���� ���
    switch (row) {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }
    lcd_send_8bit(col); // DDRAM AD Set ���
}

//----- LCD ���÷��� ���� ����
void lcd_clear() {
	HAL_GPIO_WritePin(RS_1602_GPIO_Port, RS_1602_Pin, 0); // ���� ���
	lcd_send_8bit(0b00000001); // Screen Clear ��� -> ���÷��� ���� ����
	HAL_Delay(2);              // ��� ����ð��� 1.64ms �̻� ���
}

//----- LCD ���÷��̷� ���ڿ� ����
void lcd_send_string(char *str) {
	HAL_GPIO_WritePin(RS_1602_GPIO_Port, RS_1602_Pin, 1); // ���� ���� ���
	// ���� string �ּҰ�(������)�� �޾� ���� �ϳ��� lcd_send_8bit �Լ��� ������ ������ 1�� �ø�(=���� ���� ����)
	while (*str) {
		lcd_send_8bit(*str++);
	}
}

//----- LCD ���÷��� �ʱ�ȭ (�����ͽ�Ʈ INITIALIZATION SEQUENCE ����)
void lcd_init(void) {
	HAL_GPIO_WritePin(RS_1602_GPIO_Port, RS_1602_Pin, 0); // ���� ���

    // 4 bit ���� ���� (�����ͽ�Ʈ�� ����)
    HAL_Delay(50);         // ���ýð��� 15ms �̻� ���

    lcd_send_4bit(0b0011); // ���� �ݺ� 1
    HAL_Delay(5);          // ��� ����ð��� 4.1ms �̻� ���
    lcd_send_4bit(0b0011); // ���� �ݺ� 2
    HAL_Delay(1);          // ��� ����ð��� 0.1ms �̻� ��� (���� ����)
    lcd_send_4bit(0b0011); // ���� �ݺ� 3
    HAL_Delay(1);
    lcd_send_4bit(0b0010); // 4��Ʈ ���� ����
    HAL_Delay(1);

    // ���÷��� Init (�����ͽ�Ʈ�� ����)
    lcd_send_8bit(0b00101000); // Function set ��� -> DL=0, N=1, F=0 -> 4��Ʈ, 2��, 5x7 ��Ʈ
    HAL_Delay(1);              // ��� ����ð��� 0.04ms �̻� ��� (���� ����)
    lcd_send_8bit(0b00001000); // Display Switch ��� -> D=0, C=0, B=0  -> ���÷��� OFF, Ŀ�� OFF, ��ũ OFF
    HAL_Delay(1);
    lcd_send_8bit(0b00000001); // Screen Clear ��� -> ���÷��� ���� ����
    HAL_Delay(2);
    lcd_send_8bit(0b00000110); // Input Set ��� -> I/D = 1, S = 0 -> Ŀ�� ���� ���ϱ�, ����Ʈ X
    HAL_Delay(1);

    // Init ��. ����� ���� ���÷��� ����.
    lcd_send_8bit(0b00001100); // Display Switch ��� -> D=0, C=0, B=0  -> ���÷��� ON, Ŀ�� OFF, ��ũ OFF
    HAL_Delay(1);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim1);  // delay_ms() �Լ��� ���� Ÿ�̸� ����
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  lcd_init();                      // LCD init
  lcd_set_cur(0, 0);               // Ŀ�� 0, 0 �̵�
  lcd_send_string("Hello World!"); // LCD Hello World! ���ڿ� ǥ��

  int cnt = 0;     // ī��Ʈ�� ���� ���� ����
  char buffer[16]; // ���ڿ� ������ ���� ����
  while (1) {
	  lcd_set_cur(1, 0);               // Ŀ�� 1, 0 �̵�
	  sprintf(buffer, "cnt: %d", cnt); // ī��Ʈ�� ǥ���ϴ� ���ڿ� ����
	  lcd_send_string(buffer);         // LCD ���ڿ� ǥ��
	  cnt++;                           // cmd 1 ����
	  HAL_Delay(1000);                 // 1�� ���
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xffff-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|D5_1602_Pin|D6_1602_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, D4_1602_Pin|RS_1602_Pin|EN_1602_Pin|RW_1602_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(D7_1602_GPIO_Port, D7_1602_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin D5_1602_Pin D6_1602_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|D5_1602_Pin|D6_1602_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : D4_1602_Pin RS_1602_Pin EN_1602_Pin RW_1602_Pin */
  GPIO_InitStruct.Pin = D4_1602_Pin|RS_1602_Pin|EN_1602_Pin|RW_1602_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : D7_1602_Pin */
  GPIO_InitStruct.Pin = D7_1602_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(D7_1602_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
