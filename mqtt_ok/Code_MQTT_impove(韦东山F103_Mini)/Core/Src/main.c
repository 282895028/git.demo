/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "mqttclient.h"
#include "platform_mutex.h"


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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
#include <stdio.h>
#include <string.h>
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

mqtt_client_t *client = NULL;

static void smarthome_msg_handler(void* client, message_data_t* msg)
{
	printf("Get msg: %s\r\n", (char *)msg->message->payload);
	if (strstr((char *)msg->message->payload, "\"dev\":\"led\",\"status\":\"0\"")) 
	{
	   printf("led off\r\n");
	   HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	}
	else if (strstr((char *)msg->message->payload, "\"dev\":\"led\",\"status\":\"1\""))
	{
	   printf("led on\r\n");
	   HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	}
	else if (strstr((char *)msg->message->payload, "\"dev\":\"led\",\"status\":\"2\""))
	{
		printf("led switch\r\n");
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	}
}

void StartMyTask(void *argument)
{
//	uint8_t rx_data[200] = {0};        // 保存接收数据
    int err;
	int cnt = 0;
    char buf[40] = "this is a message for test";
    mqtt_message_t msg;
    memset(&msg, 0, sizeof(msg));

	mqtt_log_init();

	client = mqtt_lease();

	mqtt_set_port(client, "1883");
	//mqtt_set_host(client, "192.168.0.130"); /* 本地实验 */
	mqtt_set_host(client, "47.114.187.247"); /* iot.100ask.net: 47.114.187.247 */
	mqtt_set_client_id(client, random_string(10));
	mqtt_set_user_name(client, random_string(10));
	mqtt_set_password(client, random_string(10));
	mqtt_set_clean_session(client, 1);

	do {
		err = mqtt_connect(client);
	} while (err);

	printf("mqtt_connect err = %d\r\n", err);

    msg.payload = (void *) buf;
    msg.qos = QOS0;
	msg.payloadlen = strlen(buf);
	mqtt_publish(client, "home", &msg);

	err = mqtt_subscribe(client, "home", QOS0, smarthome_msg_handler);	  
	printf("subscribe err = %d\r\n", err);
  
  while (1)
  {
#if 1  	
  	  sprintf(buf, "TestMsg%d", cnt++);
	  msg.payload = (void *) buf;
	  msg.qos = QOS0;
	  msg.payloadlen = strlen(buf);

	  err = mqtt_publish(client, "home", &msg);	  
	  
	  //printf("Publish msg: %s, err = %d\r\n", buf, err);

	  //mqtt_list_subscribe_topic(client);
#endif  	
	 osDelay(10000);
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
//  uint8_t wifi_data[200] = {0}; 
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  //Key_Update_WIFI((char *)wifi_data, 500);
 
  //USART_StartRx();   //启动�?始接收数�?
  //USART2_Transmit(tx_data, sizeof(tx_data), 500); //向模块发送数�?
  EnableUART2IRQ();
  
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
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
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

