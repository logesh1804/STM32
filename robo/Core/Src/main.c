/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Standalone FreeRTOS Application for STM32F411RET6
  *                   Controls motor speed/dir with Joystick X-axis, stops motor
  *                   on PIR detection, and updates SSD1306 OLED telemetry.
  *                   Compatible with runtime health metrics scanning.
  ******************************************************************************
  * @attention
  *
  * All custom variables, peripheral initializations, and FreeRTOS tasks are
  * placed inside STM32CubeMX USER CODE blocks. This ensures they are preserved
  * when you regenerate code from the robo.ioc tool.
  *
  * HARDWARE CONNECTIONS:
  *   - Joystick X-Axis: PA0 (ADC1_IN0)
  *   - PIR Motion Sensor: PA1 (GPIO_Input, active High)
  *   - Motor Driver (L298N H-Bridge):
  *       - ENA (Speed Control): PA6 (TIM3_CH1 PWM)
  *       - IN1 (Direction 1): PA2 (GPIO_Output)
  *       - IN2 (Direction 2): PA3 (GPIO_Output)
  *   - SSD1306 OLED Display (I2C):
  *       - SCL: PB6 (I2C1_SCL)
  *       - SDA: PB7 (I2C1_SDA)
  *   - Heartbeat Status LED: PA5 (Nucleo User LED)
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>

/* FreeRTOS Kernel Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    int32_t motor_speed;       /* Range: 0 to 100 (%) */
    int8_t motor_direction;    /* 0: Stopped, 1: Forward, -1: Backward */
    uint8_t pir_status;        /* 0: Safe/No Motion, 1: Motion Detected */
    uint32_t timestamp;        /* Tick Count */
} SystemStatus_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define JOYSTICK_CENTER   2048
#define JOYSTICK_DEADBAND 150
#define TIM3_PERIOD_ARR   1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
/* FreeRTOS Queue Handle */
QueueHandle_t xStatusQueue = NULL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */
void vMotorControlTask(void *pvParameters);
void vDisplayTask(void *pvParameters);
void vHeartbeatTask(void *pvParameters);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* 5x7 ASCII Font Table (Chars 0x20 Space to 0x7E ~) */
const uint8_t SSD1306_Font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // (space)
    {0x00, 0x00, 0x5f, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1c, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1c, 0x00}, // )
    {0x14, 0x08, 0x3e, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3e, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0
    {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3e}, // @
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, // A
    {0x7f, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3e, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, // D
    {0x7f, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7f, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3e, 0x41, 0x49, 0x49, 0x7a}, // G
    {0x7f, 0x08, 0x08, 0x08, 0x7f}, // H
    {0x00, 0x41, 0x7f, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3f, 0x01}, // J
    {0x7f, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7f, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // M
    {0x7f, 0x04, 0x08, 0x10, 0x7f}, // N
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, // O
    {0x7f, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3e, 0x41, 0x51, 0x21, 0x5e}, // Q
    {0x7f, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7f, 0x01, 0x01}, // T
    {0x3f, 0x40, 0x40, 0x40, 0x3f}, // U
    {0x1f, 0x20, 0x40, 0x20, 0x1f}, // V
    {0x3f, 0x40, 0x38, 0x40, 0x3f}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x1c, 0x22, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // \
    {0x00, 0x41, 0x22, 0x1c, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7f, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7f}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7e, 0x09, 0x01, 0x02}, // f
    {0x0c, 0x52, 0x52, 0x52, 0x3e}, // g
    {0x7f, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7d, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3d, 0x00}, // j
    {0x7f, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7f, 0x40, 0x00}, // l
    {0x7c, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7c, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7c, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7c}, // q
    {0x7c, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3f, 0x44, 0x40, 0x20}, // t
    {0x3c, 0x40, 0x40, 0x20, 0x7c}, // u
    {0x1c, 0x20, 0x40, 0x20, 0x1c}, // v
    {0x3c, 0x40, 0x30, 0x40, 0x3c}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0c, 0x50, 0x50, 0x50, 0x3c}, // y
    {0x44, 0x64, 0x54, 0x4c, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7f, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x08, 0x08, 0x2a, 0x1c, 0x08}, // ~
};

/* SSD1306 OLED Controller Functions */
void SSD1306_WriteCommand(uint8_t cmd) {
    HAL_I2C_Mem_Write(&hi2c1, 0x78, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, HAL_MAX_DELAY);
}

void SSD1306_WriteData(uint8_t* data, uint16_t size) {
    HAL_I2C_Mem_Write(&hi2c1, 0x78, 0x40, I2C_MEMADD_SIZE_8BIT, data, size, HAL_MAX_DELAY);
}

void SSD1306_Clear(void) {
    uint8_t zero[128] = {0};
    for (uint8_t i = 0; i < 8; i++) {
        SSD1306_WriteCommand(0xB0 + i);
        SSD1306_WriteCommand(0x00);
        SSD1306_WriteCommand(0x10);
        SSD1306_WriteData(zero, 128);
    }
}

void SSD1306_GotoXY(uint8_t x, uint8_t y) {
    SSD1306_WriteCommand(0xB0 + y);
    SSD1306_WriteCommand(x & 0x0F);
    SSD1306_WriteCommand(0x10 | ((x >> 4) & 0x0F));
}

void SSD1306_Putc(char ch) {
    if (ch < 32 || ch > 126) {
        ch = ' ';
    }
    uint8_t idx = ch - 32;
    uint8_t buf[6];
    memcpy(buf, SSD1306_Font5x7[idx], 5);
    buf[5] = 0x00;
    SSD1306_WriteData(buf, 6);
}

void SSD1306_Puts(const char* str) {
    while (*str) {
        SSD1306_Putc(*str++);
    }
}

void SSD1306_Init(void) {
    HAL_Delay(100);
    static const uint8_t SSD1306_InitSeq[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6,
        0xAF
    };
    HAL_I2C_Mem_Write(&hi2c1, 0x78, 0x00, I2C_MEMADD_SIZE_8BIT, (uint8_t*)SSD1306_InitSeq, sizeof(SSD1306_InitSeq), HAL_MAX_DELAY);
    SSD1306_Clear();
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
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  /* Start Timer 3 Base and PWM generation on Channel 1 (Motor ENA) */
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

  /* Initialize SSD1306 OLED Screen */
  SSD1306_Init();
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add timers, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* Create status tracking queue */
  xStatusQueue = xQueueCreate(5, sizeof(SystemStatus_t));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* Create tasks */
  xTaskCreate(vMotorControlTask, "MotorCtrl", 256, NULL, 3, NULL);
  xTaskCreate(vDisplayTask, "OledDisp", 256, NULL, 2, NULL);
  xTaskCreate(vHeartbeatTask, "Heartbeat", 128, NULL, 1, NULL);
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Fallback blink */
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(100);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief  vMotorControlTask
  * @details Reads Joystick (PA0 ADC) and PIR (PA1 GPIO). Maps X-axis to L298N speed/direction.
  *          Forces immediate safety stop if PIR sensor detects motion.
  */
void vMotorControlTask(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(50); /* Run at 20 Hz */
    SystemStatus_t status;

    for (;;) {
        /* 1. Read PIR Sensor */
        uint8_t pir_val = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET) ? 1 : 0;
        status.pir_status = pir_val;

        /* 2. Read Joystick X-Axis using ADC1 Channel 0 */
        uint32_t adc_val = JOYSTICK_CENTER;
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            adc_val = HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);

        /* 3. Determine Speed and Direction */
        int32_t speed_pct = 0;
        int8_t dir = 0; /* 0: stopped, 1: forward, -1: backward */

        if (pir_val == 1) {
            /* SAFETY INTERLOCK: Stop motor immediately */
            speed_pct = 0;
            dir = 0;
        } else {
            /* Map ADC Joystick X values to Speed and Direction */
            if (adc_val > (JOYSTICK_CENTER + JOYSTICK_DEADBAND)) {
                /* Forward direction */
                dir = 1;
                uint32_t offset = adc_val - (JOYSTICK_CENTER + JOYSTICK_DEADBAND);
                uint32_t max_range = 4095 - (JOYSTICK_CENTER + JOYSTICK_DEADBAND);
                speed_pct = (offset * 100) / max_range;
                if (speed_pct > 100) speed_pct = 100;
            } else if (adc_val < (JOYSTICK_CENTER - JOYSTICK_DEADBAND)) {
                /* Backward direction */
                dir = -1;
                uint32_t offset = (JOYSTICK_CENTER - JOYSTICK_DEADBAND) - adc_val;
                uint32_t max_range = JOYSTICK_CENTER - JOYSTICK_DEADBAND;
                speed_pct = (offset * 100) / max_range;
                if (speed_pct > 100) speed_pct = 100;
            } else {
                /* Stopped */
                dir = 0;
                speed_pct = 0;
            }
        }

        status.motor_speed = speed_pct;
        status.motor_direction = dir;
        status.timestamp = xTaskGetTickCount();

        /* 4. Drive L298N Direction Pins (IN1 = PA2, IN2 = PA3) */
        if (dir == 1) {
            /* Forward */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
        } else if (dir == -1) {
            /* Backward */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
        } else {
            /* Stopped / Override */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
        }

        /* Update Duty Cycle on TIM3 CH1 (ENA Speed Control) */
        uint32_t pulse = (speed_pct * (htim3.Instance->ARR)) / 100;
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);

        /* 5. Post telemetry data to display queue */
        xQueueSend(xStatusQueue, &status, 0);

        vTaskDelay(xFrequency);
    }
}

/**
  * @brief  vDisplayTask
  * @details Blocks on status queue. Updates OLED screen with speed and sensor status.
  */
void vDisplayTask(void *pvParameters) {
    SystemStatus_t rxStatus;
    char lineBuffer[22];

    /* Print persistent headers */
    SSD1306_GotoXY(10, 0);
    SSD1306_Puts("SYSTEM MONITOR");
    SSD1306_GotoXY(0, 1);
    SSD1306_Puts("--------------------");

    for (;;) {
        if (xQueueReceive(xStatusQueue, &rxStatus, portMAX_DELAY) == pdTRUE) {
            /* Row 3: Direction */
            SSD1306_GotoXY(0, 3);
            if (rxStatus.motor_direction == 1) {
                sprintf(lineBuffer, "DIR  : FORWARD   ");
            } else if (rxStatus.motor_direction == -1) {
                sprintf(lineBuffer, "DIR  : BACKWARD  ");
            } else {
                if (rxStatus.pir_status == 1) {
                    sprintf(lineBuffer, "DIR  : OVERRIDE  ");
                } else {
                    sprintf(lineBuffer, "DIR  : STOPPED   ");
                }
            }
            SSD1306_Puts(lineBuffer);

            /* Row 4: Speed */
            SSD1306_GotoXY(0, 4);
            sprintf(lineBuffer, "SPEED: %3d %%     ", (int)rxStatus.motor_speed);
            SSD1306_Puts(lineBuffer);

            /* Row 5: PIR alert status */
            SSD1306_GotoXY(0, 6);
            if (rxStatus.pir_status == 1) {
                sprintf(lineBuffer, "PIR  : MOTION DET!");
            } else {
                sprintf(lineBuffer, "PIR  : CLEAR      ");
            }
            SSD1306_Puts(lineBuffer);
        }
    }
}

/**
  * @brief  vHeartbeatTask
  * @details Toggles system LED on PA5.
  */
void vHeartbeatTask(void *pvParameters) {
    for (;;) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM5 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM5)
  {
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
