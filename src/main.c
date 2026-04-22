/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <main.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <watchdog.h>
#include <hc595.h>
#include <hc595_pins.h>
#include <max31865.h>
#include <ds18b20.h>
#include <sst25vf016b.h>  // SST25VF016B 2MB SPI Flash (replaces W25Q128JW)
#include <flash_fs.h>
#include <ymodem_flash.h>


// External function declarations
extern void uart_printf(const char* format, ...);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    uint8_t relay_states;      // 8 relay outputs
    uint8_t digital_inputs;     // 4 digital inputs
    uint16_t analog_inputs[4]; // 4 analog inputs
    uint8_t node_id;           // From DIP switch
    bool can_active;           // CAN bus status
} SystemIO_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Digital Input Pins (Optocoupler isolated)
#define DIN_1_Pin       GPIO_PIN_3
#define DIN_1_Port      GPIOA
#define DIN_2_Pin       GPIO_PIN_4
#define DIN_2_Port      GPIOA
#define DIN_3_Pin       GPIO_PIN_5
#define DIN_3_Port      GPIOA
#define DIN_4_Pin       GPIO_PIN_6
#define DIN_4_Port      GPIOA

// Relay Output Pins (ULN2003 driven)
#define RELAY_1_Pin     GPIO_PIN_1
#define RELAY_1_Port    GPIOB
#define RELAY_2_Pin     GPIO_PIN_2
#define RELAY_2_Port    GPIOB
#define RELAY_3_Pin     GPIO_PIN_3
#define RELAY_3_Port    GPIOB
#define RELAY_4_Pin     GPIO_PIN_4
#define RELAY_4_Port    GPIOB
#define RELAY_5_Pin     GPIO_PIN_5
#define RELAY_5_Port    GPIOB
#define RELAY_6_Pin     GPIO_PIN_6
#define RELAY_6_Port    GPIOB
#define RELAY_7_Pin     GPIO_PIN_7
#define RELAY_7_Port    GPIOB
#define RELAY_8_Pin     GPIO_PIN_8
#define RELAY_8_Port    GPIOB

// LED Pins
#define STATUS_LED_Pin  GPIO_PIN_0
#define STATUS_LED_Port GPIOB

// Relay Status LEDs
#define RELAY_LED1_Pin  GPIO_PIN_0
#define RELAY_LED1_Port GPIOC
#define RELAY_LED2_Pin  GPIO_PIN_1
#define RELAY_LED2_Port GPIOC
#define RELAY_LED3_Pin  GPIO_PIN_2
#define RELAY_LED3_Port GPIOC
#define RELAY_LED4_Pin  GPIO_PIN_3
#define RELAY_LED4_Port GPIOC
#define RELAY_LED5_Pin  GPIO_PIN_4
#define RELAY_LED5_Port GPIOC
#define RELAY_LED6_Pin  GPIO_PIN_5
#define RELAY_LED6_Port GPIOC
#define RELAY_LED7_Pin  GPIO_PIN_6
#define RELAY_LED7_Port GPIOC
#define RELAY_LED8_Pin  GPIO_PIN_7
#define RELAY_LED8_Port GPIOC

// ID Switch Pins
#define ID_SW_1_Pin     GPIO_PIN_14
#define ID_SW_1_Port    GPIOB
#define ID_SW_2_Pin     GPIO_PIN_15
#define ID_SW_2_Port    GPIOB
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define RELAY_ON(n)     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_##n, GPIO_PIN_SET)
#define RELAY_OFF(n)    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_##n, GPIO_PIN_RESET)
#define RELAY_LED_ON(n) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_##n, GPIO_PIN_SET)
#define RELAY_LED_OFF(n) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_##n, GPIO_PIN_RESET)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

static CAN_RxHeaderTypeDef sCanRxHeader;
static uint8_t sCanRxData[8];
static volatile uint32_t sCanRxCount = 0;
static volatile uint32_t sCanRxLastId = 0;


SystemIO_t system_io = {0};
volatile uint32_t system_tick_ms = 0;
volatile bool plc_running = true;
volatile bool debug_mode = false;
//extern volatile bool debug_mode;  // Share with interpreter

// UART receive buffers
uint8_t uart1_rx_buffer[1];  // UART1 for Modbus
uint8_t uart3_rx_buffer[1];  // UART3 for commands
volatile char command_ready = 0;  // Command ready flag for main loop
volatile char pending_command = 0;  // Command to process

// Performance monitoring
uint32_t last_status_time = 0;
uint32_t last_led_update = 0;

// EEPROM addresses for 24LC02
#define EEPROM_ADDR     0xA0
#define EEPROM_CONFIG   0x00
#define EEPROM_RELAY    0x10

// =============================================================================
// 74HC595 SHIFT REGISTER CONFIGURATION
// =============================================================================
// HC595 HANDLES
// Pin definitions are in hc595_pins.h
// =============================================================================

// LED grubu (4 chip): ST_LED = PB3
HC595_Handle hc595_outputs;
#define HC595_LED_NUM_CHIPS     4        // 32 LED (4 chip x 8 pin)

// Kontrol grubu (2 chip): ST = PC4
HC595_Handle hc595_control;
#define HC595_CTRL_NUM_CHIPS    2        // 16 kontrol çıkışı (2 chip x 8 pin)

// =============================================================================
// MAX31865 PT100 SENSOR CONFIGURATION
// =============================================================================
MAX31865_Handle pt100_sensor1;  // First PT100
MAX31865_Handle pt100_sensor2;  // Second PT100

// PT100 SPI Pin Definitions (Shared SPI pins - PA5, PA6, PA7)
#define PT_SCLK_PORT        GPIOA
#define PT_SCLK_PIN         GPIO_PIN_5
#define PT_MOSI_PORT        GPIOA
#define PT_MOSI_PIN         GPIO_PIN_7
#define PT_MISO_PORT        GPIOA
#define PT_MISO_PIN         GPIO_PIN_6

// PT100 Configuration
#define PT100_REF_RESISTOR  430.0f   // Reference resistor (ohms)
#define PT100_WIRE_MODE     MAX31865_2WIRE  // 2-wire, 3-wire veya 4-wire

// PT100 temperature storage
float pt100_temperature1 = 0.0f;
float pt100_resistance1 = 0.0f;
uint8_t pt100_fault1 = 0;

float pt100_temperature2 = 0.0f;
float pt100_resistance2 = 0.0f;
uint8_t pt100_fault2 = 0;

// =============================================================================
// DS18B20 ONEWIRE TEMPERATURE SENSOR
// =============================================================================
DS18B20_Handle ds18b20_sensor;

// DS18B20 Pin Definition (PA4)
#define DS18B20_PORT        GPIOA
#define DS18B20_PIN         GPIO_PIN_4

// DS18B20 temperature storage
float ds18b20_temperature = 0.0f;
bool ds18b20_connected = false;

// =============================================================================
// SST25VF016B SPI FLASH CONFIGURATION (2MB)
// =============================================================================
SST25_Handle spi_flash;
bool spi_flash_connected = false;

// =============================================================================
// FLASH FILE SYSTEM & YMODEM CONFIGURATION
// =============================================================================
FFS_Handle flash_fs;
YMODEM_Flash_Handle ymodem_flash;
bool ffs_initialized = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_IWDG_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_CAN_Init(void);
static void MX_I2C1_Init(void);
static void MX_UART4_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);

/* USER CODE BEGIN PFP */
static void plc_watchdog_timeout_handler(void);
static void plc_watchdog_warning_handler(uint32_t remaining_ms);
void Write_74HC595_Outputs(void);
void Set_Status_LED(uint8_t led_index, bool state);
void Init_PT100(void);
void Read_PT100_Temperature(void);
void Read_Digital_Inputs(void);
void Read_Analog_Inputs(void);
void Send_HMI_Status(void);

// Status LED indices for U4 (Chip 3)
#define STATUS_LED_A_O1     0
#define STATUS_LED_A_O2     1
#define STATUS_LED_SYS      2
#define STATUS_LED_ERR      3
#define STATUS_LED_CON      4
#define STATUS_LED_EX1      5
#define STATUS_LED_EX2      6


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t last_status_print = 0;
uint32_t main_loop_elapsed_ms = 0;  // Main loop execution time in ms
uint32_t last_pt100_read = 0;       // PT100 read timer (read every 500ms)

/**
  * @brief  Send HMI status packet over UART
  * @note   Format: $PLC,DI:xxxx,AI:a0,a1,a2,a3,DO:xxxx,HC:xxxxxxxx,RY:xx,T1:temp,T2:temp,ST:s,CY:us,MC:us,ML:ms*CS\r\n
  *         DI = Digital Inputs (16 bits as 4 hex chars)
  *         AI = Analog Inputs (4 x 16-bit values, comma separated)
  *         DO = Digital Outputs (16 bits as 4 hex chars)
  *         HC = HC595 outputs/LEDs (32 bits as 8 hex chars)
  *         RY = Relay outputs (8 bits as 2 hex chars)
  *         T1 = PT100 #1 temperature (x10, integer, -9999 if fault)
  *         T2 = PT100 #2 temperature (x10, integer, -9999 if fault)
  *         ST = PLC status (R=running, S=stopped)
  *         CY = PLC cycle time in microseconds
  *         MC = Max PLC cycle time in microseconds
  *         ML = Main loop elapsed time in milliseconds
  *         CS = Checksum (XOR of all bytes between $ and *)
  */
void Send_HMI_Status(void)
{
    char packet[200];
    int len = 0;

    // Read current I/O states
    Read_Digital_Inputs();
    Read_Analog_Inputs();

    // Build 16-bit digital input state (direct hardware read)
    uint16_t din_state = 0;
    // TODO: Read from hardware directly when PLC module restored

    // Build 16-bit digital output state (direct hardware read)
    uint16_t dout_state = 0;
    // TODO: Read from hardware directly when PLC module restored

    // Build HC595 output state (32 bits) - for LED indicators
    uint32_t hc595_state = 0;
    for (int i = 0; i < 32 && i < HC595_LED_NUM_CHIPS * 8; i++) {
        if (HC595_GetPin(&hc595_outputs, i)) {
            hc595_state |= (1UL << i);
        }
    }

    // Get PLC status character
    char plc_status = 'S';  // Stopped (PLC module removed)

    // Get temperatures (x10 to avoid float in packet)
    int16_t temp1 = (pt100_fault1 == 0) ? (int16_t)(pt100_temperature1 * 10) : -9999;
    int16_t temp2 = (pt100_fault2 == 0) ? (int16_t)(pt100_temperature2 * 10) : -9999;

    // Build packet (without checksum)
    // DI: 4 hex chars for 16 digital inputs
    // DO: 4 hex chars for 16 digital outputs
    // HC: 8 hex chars for 32 HC595 outputs (LEDs)
    // CY: PLC cycle time in microseconds, MC: max cycle time
    // ML: main loop elapsed time in milliseconds
    len = snprintf(packet, sizeof(packet),
        "$PLC,DI:%04X,AI:%d,%d,%d,%d,DO:%04X,HC:%08lX,RY:%02X,T1:%d,T2:%d,ST:%c,CY:%lu,MC:%lu,ML:%lu*",
        din_state,
        system_io.analog_inputs[0],
        system_io.analog_inputs[1],
        system_io.analog_inputs[2],
        system_io.analog_inputs[3],
        dout_state,
        (unsigned long)hc595_state,
        system_io.relay_states,
        temp1, temp2,
        plc_status,
        0UL,  // cycle_time_us - PLC module removed
        0UL,  // max_cycle_time_us - PLC module removed
        (unsigned long)main_loop_elapsed_ms);

    // Calculate checksum (XOR of bytes between $ and *)
    uint8_t checksum = 0;
    for (int i = 1; i < len - 1; i++) {  // Skip $ and *
        checksum ^= packet[i];
    }

    // Append checksum and newline
    snprintf(packet + len, sizeof(packet) - len, "%02X\r\n", checksum);

    // Send packet
    uart_printf("%s", packet);
}

/**
  * @brief  Read optocoupler isolated digital inputs
  */
void Read_Digital_Inputs(void)
{
    system_io.digital_inputs = 0;
    
    // Read optocoupler inputs (active low due to optocoupler)
    if (HAL_GPIO_ReadPin(DIN_1_Port, DIN_1_Pin) == GPIO_PIN_RESET)
        system_io.digital_inputs |= 0x01;
    if (HAL_GPIO_ReadPin(DIN_2_Port, DIN_2_Pin) == GPIO_PIN_RESET)
        system_io.digital_inputs |= 0x02;
    if (HAL_GPIO_ReadPin(DIN_3_Port, DIN_3_Pin) == GPIO_PIN_RESET)
        system_io.digital_inputs |= 0x04;
    if (HAL_GPIO_ReadPin(DIN_4_Port, DIN_4_Pin) == GPIO_PIN_RESET)
        system_io.digital_inputs |= 0x08;
}

/**
  * @brief  Write to relay outputs through ULN2003 and 74HC595
  */
void Write_Relay_Outputs(void)
{
    // Write each relay based on system_io.relay_states
    for (int i = 0; i < 8; i++) {
        GPIO_PinState state = (system_io.relay_states & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1 << i, state);
    }

    // Also update 74HC595 outputs
    Write_74HC595_Outputs();
}

/**
  * @brief  Read analog inputs with voltage divider protection
  */
void Read_Analog_Inputs(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    // Channel 0 - PA0 (A_IN_1)
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    system_io.analog_inputs[0] = HAL_ADC_GetValue(&hadc1);
    
    // Channel 1 - PA1 (A_IN_2)
    sConfig.Channel = ADC_CHANNEL_1;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    system_io.analog_inputs[1] = HAL_ADC_GetValue(&hadc1);
    
    // Channel 2 - PA2 (A_IN_3)
    sConfig.Channel = ADC_CHANNEL_2;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    system_io.analog_inputs[2] = HAL_ADC_GetValue(&hadc1);
    
    // Channel 7 - PA7 (A_IN_4)
    sConfig.Channel = ADC_CHANNEL_7;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    system_io.analog_inputs[3] = HAL_ADC_GetValue(&hadc1);
}

/**
  * @brief  Update relay status LEDs
  */
void Update_Status_LEDs(void)
{
    // Update relay indicator LEDs
    for (int i = 0; i < 8; i++) {
        GPIO_PinState state = (system_io.relay_states & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 << i, state);
    }
}

/**
  * @brief  Read node ID from DIP switch
  */
void Read_Node_ID(void)
{
    system_io.node_id = 0;

    // Read DIP switch (pull-up resistors, so inverted)
    if (HAL_GPIO_ReadPin(ID_SW_1_Port, ID_SW_1_Pin) == GPIO_PIN_RESET)
        system_io.node_id |= 0x01;
    if (HAL_GPIO_ReadPin(ID_SW_2_Port, ID_SW_2_Pin) == GPIO_PIN_RESET)
        system_io.node_id |= 0x02;
}

/**
  * @brief  Set PT100 CS via 74HC595
  * @param  sensor_num: 1 or 2
  * @param  state: true=HIGH (deselect), false=LOW (select)
  */
void PT100_SetCS(uint8_t sensor_num, bool state)
{
    // CS pins are active LOW
    // When state=true, CS is HIGH (chip deselected)
    // When state=false, CS is LOW (chip selected)

    if (sensor_num == 1) {
        HC595_SetPin(&hc595_control, PT_CS1_BIT, state);
    } else if (sensor_num == 2) {
        HC595_SetPin(&hc595_control, PT_CS2_BIT, state);
    }
    HC595_Update(&hc595_control);
}

/**
  * @brief  Switch SPI1 to Mode 1 for MAX31865 (CPOL=0, CPHA=1)
  */
void SPI1_SetMode1_PT100(void)
{
    // Disable SPI first
    __HAL_SPI_DISABLE(&hspi1);

    // Change to Mode 1 (CPOL=0, CPHA=1) for MAX31865
    hspi1.Instance->CR1 &= ~SPI_CR1_CPOL;  // CPOL = 0
    hspi1.Instance->CR1 |= SPI_CR1_CPHA;   // CPHA = 1

    // Re-enable SPI
    __HAL_SPI_ENABLE(&hspi1);
}

/**
  * @brief  Switch SPI1 to Mode 0 for SST25VF016B Flash (CPOL=0, CPHA=0)
  */
void SPI1_SetMode0_Flash(void)
{
    // Disable SPI first
    __HAL_SPI_DISABLE(&hspi1);

    // Change to Mode 0 (CPOL=0, CPHA=0) for SST25VF016B
    hspi1.Instance->CR1 &= ~SPI_CR1_CPOL;  // CPOL = 0
    hspi1.Instance->CR1 &= ~SPI_CR1_CPHA;  // CPHA = 0

    // Re-enable SPI
    __HAL_SPI_ENABLE(&hspi1);
}

/**
  * @brief  CS callback for PT100 sensor 1 (used by MAX31865 driver)
  */
void PT100_CS1_Callback(bool state)
{
    HC595_SetPin(&hc595_control, PT_CS1_BIT, state);
    HC595_Update(&hc595_control);
}

/**
  * @brief  CS callback for PT100 sensor 2 (used by MAX31865 driver)
  */
void PT100_CS2_Callback(bool state)
{
    HC595_SetPin(&hc595_control, PT_CS2_BIT, state);
    HC595_Update(&hc595_control);
}

/**
  * @brief  CS callback for PT100 sensor 1 (used by MAX31865 driver)
  */
void ETH_CS2_Set(bool state)
{
    HC595_SetPin(&hc595_control, CTRL_SPI2_CS, state);
    HC595_Update(&hc595_control);
}

/**
  * @brief  CS callback for SPI Flash (W25Q128JW) - uses EEP1 pin on HC595
  */
void Flash_CS_Callback(bool state)
{
    HC595_SetPin(&hc595_control, EEP2_BIT, state);
		HC595_SetPin(&hc595_control, EEP3_BIT, state);
    HC595_Update(&hc595_control);
}

/**
  * @brief  Initialize SST25VF016B SPI Flash (2MB)
  */
void Init_SPI_Flash(void)
{
    uart_printf("  Initializing SPI Flash (SST25VF016B)...\r\n");

    // Configure flash handle
    spi_flash.hspi = &hspi1;
    spi_flash.use_hardware_spi = true;
    spi_flash.cs_callback = Flash_CS_Callback;
    spi_flash.cs_port = NULL;  // Not used - CS via callback
    spi_flash.cs_pin = 0;

    // Software SPI pins (not used when hardware SPI enabled)
    spi_flash.sclk_port = PT_SCLK_PORT;
    spi_flash.sclk_pin = PT_SCLK_PIN;
    spi_flash.mosi_port = PT_MOSI_PORT;
    spi_flash.mosi_pin = PT_MOSI_PIN;
    spi_flash.miso_port = PT_MISO_PORT;
    spi_flash.miso_pin = PT_MISO_PIN;

    // Ensure CS is high before init
    Flash_CS_Callback(true);
    HAL_Delay(1);

    // Initialize flash
    W25Q_Status status = W25Q_Init(&spi_flash);
    if (status == W25Q_OK) {
        spi_flash_connected = true;
        uart_printf("    SST25VF016B detected!\r\n");
        uart_printf("    Manufacturer: 0x%02X\r\n", spi_flash.manufacturer_id);
        uart_printf("    Memory Type:  0x%02X\r\n", spi_flash.memory_type);
        uart_printf("    Capacity:     0x%02X (2MB)\r\n", spi_flash.capacity_id);

        // Initialize Flash File System
        uart_printf("  Initializing Flash File System...\r\n");
        if (FFS_Init(&flash_fs, &spi_flash) == FFS_OK) {
            // Check if FFS is valid, format if not
            if (!FFS_IsValid(&flash_fs)) {
                uart_printf("    FFS not valid, formatting...\r\n");
                FFS_Format(&flash_fs);
            }
            ffs_initialized = true;
            uart_printf("    FFS OK (Files: %lu, Used: %lu bytes)\r\n",
                       FFS_GetFileCount(&flash_fs), FFS_GetUsedSpace(&flash_fs));

            // Initialize YMODEM with FFS
            if (YMODEM_Flash_InitWithFFS(&ymodem_flash, &huart3, &spi_flash, &flash_fs) == YMODEM_OK) {
                uart_printf("    YMODEM ready on UART3\r\n");
                // Start listening for YMODEM transfers
                YMODEM_Flash_StartListen(&ymodem_flash, 1048576, FFS_TYPE_PLC_PROGRAM);  // 1MB max
                uart_printf("    YMODEM listening for files...\r\n");
            }
        } else {
            ffs_initialized = false;
            uart_printf("    FFS init failed!\r\n");
        }
    } else {
        spi_flash_connected = false;
        uart_printf("    SST25VF016B init failed (error: %d)\r\n", status);
    }
}

/**
  * @brief  Initialize PT100 Temperature Sensors (both sensors)
  */
void Init_PT100(void)
{


    // Set both CS pins HIGH (deselected) initially
    PT100_SetCS(1, true);
    PT100_SetCS(2, true);

    // Configure PT100 sensor 1
    pt100_sensor1.sclk_port = PT_SCLK_PORT;
    pt100_sensor1.sclk_pin = PT_SCLK_PIN;
    pt100_sensor1.mosi_port = PT_MOSI_PORT;
    pt100_sensor1.mosi_pin = PT_MOSI_PIN;
    pt100_sensor1.miso_port = PT_MISO_PORT;
    pt100_sensor1.miso_pin = PT_MISO_PIN;
    pt100_sensor1.cs_port = NULL;  // Not used - CS via callback
    pt100_sensor1.cs_pin = 0;
    pt100_sensor1.drdy_enabled = false;
    pt100_sensor1.cs_callback = PT100_CS1_Callback;  // CS via HC595
    pt100_sensor1.hspi = &hspi1;
    pt100_sensor1.use_hardware_spi = true;  // Hardware SPI with dynamic mode switching
    pt100_sensor1.rtd_type = MAX31865_RTD_PT100;
    pt100_sensor1.wire_mode = PT100_WIRE_MODE;
    pt100_sensor1.filter_mode = MAX31865_FILTER_50HZ;
    pt100_sensor1.ref_resistor = PT100_REF_RESISTOR;

    // Configure PT100 sensor 2 (same SPI pins, different CS callback)
    pt100_sensor2.sclk_port = PT_SCLK_PORT;
    pt100_sensor2.sclk_pin = PT_SCLK_PIN;
    pt100_sensor2.mosi_port = PT_MOSI_PORT;
    pt100_sensor2.mosi_pin = PT_MOSI_PIN;
    pt100_sensor2.miso_port = PT_MISO_PORT;
    pt100_sensor2.miso_pin = PT_MISO_PIN;
    pt100_sensor2.cs_port = NULL;  // Not used - CS via callback
    pt100_sensor2.cs_pin = 0;
    pt100_sensor2.drdy_enabled = false;
    pt100_sensor2.cs_callback = PT100_CS2_Callback;  // CS via HC595
    pt100_sensor2.hspi = &hspi1;
    pt100_sensor2.use_hardware_spi = true;  // Hardware SPI with dynamic mode switching
    pt100_sensor2.rtd_type = MAX31865_RTD_PT100;
    pt100_sensor2.wire_mode = PT100_WIRE_MODE;
    pt100_sensor2.filter_mode = MAX31865_FILTER_50HZ;
    pt100_sensor2.ref_resistor = PT100_REF_RESISTOR;

    // Note: SPI1 pins (PA5=SCK, PA6=MISO, PA7=MOSI) are configured by MX_SPI1_Init()
    // SPI mode is switched dynamically before PT100 read (Mode 1) and after (Mode 0 for flash)
    // CS pins are controlled via 74HC595 (PT_CS1_BIT, PT_CS2_BIT)

    // Mark sensors as initialized (we'll do custom CS control)
    pt100_sensor1.initialized = true;
    pt100_sensor2.initialized = true;

    // Configure MAX31865 registers for both sensors
    // Sensor 1
    PT100_SetCS(1, false);  // Select
    HAL_Delay(1);
    // Write config: wire mode + 50Hz filter
    uint8_t config = (PT100_WIRE_MODE == MAX31865_3WIRE) ? 0x10 : 0x00;
    config |= 0x01;  // 50Hz filter
    // Simple config write
    PT100_SetCS(1, true);   // Deselect

    // Sensor 2
    PT100_SetCS(2, false);
    HAL_Delay(1);
    PT100_SetCS(2, true);

    uart_printf("► PT100: 2 sensors initialized\r\n");
    uart_printf("  SPI Mode: Hardware (SPI1, dynamic Mode 0/1 switching)\r\n");
    uart_printf("  SPI Pins: SCLK=PA5, MOSI=PA7, MISO=PA6\r\n");
    uart_printf("  CS1: HC595 bit %d, CS2: HC595 bit %d\r\n", PT_CS1_BIT, PT_CS2_BIT);
    uart_printf("  Config: %s, %dΩ reference, 50Hz filter\r\n",
               (PT100_WIRE_MODE == MAX31865_2WIRE) ? "2-wire" :
               (PT100_WIRE_MODE == MAX31865_3WIRE) ? "3-wire" : "4-wire",
               (int)PT100_REF_RESISTOR);

    // Verify SPI communication with each sensor
    SPI1_SetMode1_PT100();
    uint8_t cfg1 = 0, cfg2 = 0;
    MAX31865_Status st1 = MAX31865_VerifyCommunication(&pt100_sensor1, &cfg1);
    MAX31865_Status st2 = MAX31865_VerifyCommunication(&pt100_sensor2, &cfg2);
    SPI1_SetMode0_Flash();

    uart_printf("  Sensor 1: %s (config=0x%02X)\r\n",
               (st1 == MAX31865_OK) ? "OK" : "NOT DETECTED", cfg1);
    uart_printf("  Sensor 2: %s (config=0x%02X)\r\n",
               (st2 == MAX31865_OK) ? "OK" : "NOT DETECTED", cfg2);
}

/**
  * @brief  Read PT100 Temperature from specific sensor
  * @param  sensor_num: 1 or 2
  */
void Read_PT100_Sensor(uint8_t sensor_num)
{
    MAX31865_Handle* sensor;
    float* temp_ptr;
    float* res_ptr;
    uint8_t* fault_ptr;

    if (sensor_num == 1) {
        sensor = &pt100_sensor1;
        temp_ptr = &pt100_temperature1;
        res_ptr = &pt100_resistance1;
        fault_ptr = &pt100_fault1;
    } else {
        sensor = &pt100_sensor2;
        temp_ptr = &pt100_temperature2;
        res_ptr = &pt100_resistance2;
        fault_ptr = &pt100_fault2;
    }

    // Switch SPI1 to Mode 1 for MAX31865
    SPI1_SetMode1_PT100();

    // CS control is automatic via callback in MAX31865 driver
    // Enable bias voltage
    MAX31865_SetBias(sensor, true);

    // Trigger conversion and read
    MAX31865_TriggerConversion(sensor);

    MAX31865_Status status = MAX31865_ReadTemperature(sensor, temp_ptr);

    if (status == MAX31865_OK) {
        MAX31865_ReadResistance(sensor, res_ptr);
        *fault_ptr = 0;
    } else {
        *fault_ptr = MAX31865_GetFault(sensor);
        MAX31865_ClearFault(sensor);
    }

    // Disable bias to save power
    MAX31865_SetBias(sensor, false);

    // Switch SPI1 back to Mode 0 for Flash
    SPI1_SetMode0_Flash();
}

/**
  * @brief  Read both PT100 Temperatures
  */
void Read_PT100_Temperature(void)
{
    Read_PT100_Sensor(1);
    Read_PT100_Sensor(2);
}

/**
  * @brief  Initialize DS18B20 OneWire sensor
  */
void Init_DS18B20(void)
{
    ds18b20_sensor.port = DS18B20_PORT;
    ds18b20_sensor.pin = DS18B20_PIN;
    ds18b20_sensor.resolution = DS18B20_RESOLUTION_12BIT;
    ds18b20_sensor.parasite_power = false;
    ds18b20_sensor.initialized = false;

    DS18B20_Status status = DS18B20_Init(&ds18b20_sensor);

    if (status == DS18B20_OK) {
        ds18b20_connected = true;
        uart_printf("► DS18B20: Initialized on PA4\r\n");
        uart_printf("  ROM: %02X-%02X%02X%02X%02X%02X%02X-%02X\r\n",
                   ds18b20_sensor.rom_code[0],
                   ds18b20_sensor.rom_code[1],
                   ds18b20_sensor.rom_code[2],
                   ds18b20_sensor.rom_code[3],
                   ds18b20_sensor.rom_code[4],
                   ds18b20_sensor.rom_code[5],
                   ds18b20_sensor.rom_code[6],
                   ds18b20_sensor.rom_code[7]);
        uart_printf("  Resolution: 12-bit (0.0625 C)\r\n");
    } else {
        ds18b20_connected = false;
        uart_printf("► DS18B20: Not found on PA4 (error: %d)\r\n", status);
    }
}

/**
  * @brief  Read DS18B20 Temperature
  */
void Read_DS18B20_Temperature(void)
{
    if (!ds18b20_connected) {
        return;
    }

    DS18B20_Status status = DS18B20_ReadTemperature(&ds18b20_sensor, &ds18b20_temperature);

    if (status != DS18B20_OK) {
        ds18b20_connected = false;
    }
}

/**
  * @brief  Initialize 74HC595 shift registers
  */
void Init_74HC595(void)
{
    // Configure LED 74HC595 handle (4 chips for LEDs)
    // Ortak DATA ve CLOCK pinleri, LATCH = PB3 (ST_LED)
    hc595_outputs.data_port = HC595_SHARED_DATA_PORT;
    hc595_outputs.data_pin = HC595_SHARED_DATA_PIN;
    hc595_outputs.clock_port = HC595_SHARED_CLOCK_PORT;
    hc595_outputs.clock_pin = HC595_SHARED_CLOCK_PIN;
    hc595_outputs.latch_port = HC595_LED_LATCH_PORT;
    hc595_outputs.latch_pin = HC595_LED_LATCH_PIN;
    hc595_outputs.oe_active = false;  // OE GND'ye bağlı, kullanmıyoruz
    hc595_outputs.mode = HC595_MODE_SOFTWARE;  // Bit-bang mode (no SPI)
    hc595_outputs.hspi = NULL;  // SPI kullanmıyoruz
    hc595_outputs.num_chips = HC595_LED_NUM_CHIPS;

    // Initialize 74HC595
    HC595_Status status = HC595_Init(&hc595_outputs);
    if (status == HC595_OK) {
        uart_printf("► 74HC595 LED: %d chip(s) initialized (software mode)\n", HC595_LED_NUM_CHIPS);
        uart_printf("  Pins: DS=PB4, SHCP=PB5, STCP=PB3\n");
        uart_printf("  Total outputs: %d LEDs\n", HC595_LED_NUM_CHIPS * 8);

        // Test pattern (all outputs low initially)
        HC595_Clear(&hc595_outputs);
        HC595_Enable(&hc595_outputs);
    } else {
        uart_printf("ERROR: 74HC595 LED initialization failed (code: %d)\n", status);
    }
		
    // First, initialize control HC595 for CS pins
    // Ortak DATA ve CLOCK pinleri, farklı LATCH pini (PC4)
    hc595_control.data_port = HC595_SHARED_DATA_PORT;
    hc595_control.data_pin = HC595_SHARED_DATA_PIN;
    hc595_control.clock_port = HC595_SHARED_CLOCK_PORT;
    hc595_control.clock_pin = HC595_SHARED_CLOCK_PIN;
    hc595_control.latch_port = HC595_CTRL_LATCH_PORT;
    hc595_control.latch_pin = HC595_CTRL_LATCH_PIN;
    hc595_control.oe_active = false;
    hc595_control.mode = HC595_MODE_SOFTWARE;
    hc595_control.hspi = &hspi1;
    hc595_control.num_chips = HC595_CTRL_NUM_CHIPS;

    HC595_Status hc_status = HC595_Init(&hc595_control);
    if (hc_status != HC595_OK) {
        uart_printf("ERROR: Control HC595 init failed\r\n");
        return;
    }		
}

// =============================================================================
// LED INDEX DEFINITIONS (32 LEDs across 4x 74HC595)
// =============================================================================

// U1 (Chip 0): Digital Input LEDs
#define LED_D_IN1       0
#define LED_D_IN2       1
#define LED_D_IN3       2
#define LED_D_IN4       3
#define LED_D_IN5       4
#define LED_D_IN6       5
#define LED_D_IN7       6
#define LED_D_IN8       7

// U2 (Chip 1): Digital Input LEDs (continued)
#define LED_D_IN9       8
#define LED_D_IN10      9
#define LED_D_IN11      10
#define LED_D_IN12      11
#define LED_D_IN13      12
#define LED_D_IN14      13
#define LED_D_IN15      14
#define LED_D_IN16      15

// U3 (Chip 2): Digital Output LEDs
#define LED_D_O1        16
#define LED_D_O2        17
#define LED_D_O3        18
#define LED_D_O4        19
#define LED_D_O5        20
#define LED_D_O6        21
#define LED_D_O7        22
#define LED_D_O8        23

// U4 (Chip 3): Status LEDs
#define LED_A_O1        24
#define LED_POWER       25
#define LED_A_O2        26
#define LED_SYS         27
#define LED_ERR         28
#define LED_CON         29
#define LED_EX1         30
#define LED_EX2         31


// Bit 31 unused

// Status LED aliases (for backward compatibility)
#define STATUS_LED_A_O1     0
#define STATUS_LED_A_O2     1
#define STATUS_LED_SYS      2
#define STATUS_LED_ERR      3
#define STATUS_LED_CON      4
#define STATUS_LED_EX1      5
#define STATUS_LED_EX2      6

/**
  * @brief  Set any LED by global index (0-31)
  * @param  led_index: LED index (0-31)
  * @param  state: true=ON, false=OFF
  */
void Set_LED(uint8_t led_index, bool state)
{
    if (led_index >= 32) return;
    HC595_SetPin(&hc595_outputs, led_index, state);
}

/**
  * @brief  Set status LED (U4 chip, for backward compatibility)
  * @param  led_index: LED index (0-6: A_O1, A_O2, SYS, ERR, CON, EX1, EX2)
  * @param  state: true=ON, false=OFF
  */
void Set_Status_LED(uint8_t led_index, bool state)
{
    if (led_index > 6) return;
    // U4 is chip 3, so add 24 (3 * 8)
    HC595_SetPin(&hc595_outputs, 24 + led_index, state);
}

/**
  * @brief  Set Digital Input LED (U1-U2)
  * @param  input_num: Input number (1-16)
  * @param  state: true=ON, false=OFF
  */
void Set_Input_LED(uint8_t input_num, bool state)
{
    if (input_num < 1 || input_num > 16) return;
    HC595_SetPin(&hc595_outputs, input_num - 1, state);
}

/**
  * @brief  Set Digital Output LED (U3)
  * @param  output_num: Output number (1-8)
  * @param  state: true=ON, false=OFF
  */
void Set_Output_LED(uint8_t output_num, bool state)
{
    if (output_num < 1 || output_num > 8) return;
    // U3 is chip 2, so add 16 (2 * 8)
    HC595_SetPin(&hc595_outputs, 16 + output_num - 1, state);
}

/**
  * @brief  Update all Input LEDs from hardware digital inputs
  */
void Update_Input_LEDs(void)
{
    // U1: D_IN1-8 from system_io.digital_inputs
    for (int i = 0; i < 8; i++) {
        HC595_SetPin(&hc595_outputs, i, (system_io.digital_inputs >> i) & 1);
    }
    // U2: D_IN9-16 (currently only 4 inputs, rest are off)
    for (int i = 8; i < 16; i++) {
        HC595_SetPin(&hc595_outputs, i, 0);
    }
}

/**
  * @brief  Update all Output LEDs from relay states
  */
void Update_Output_LEDs(void)
{
    // U3: D_O1-8 from relay states (PLC module removed)
    for (int i = 0; i < 8; i++) {
        HC595_SetPin(&hc595_outputs, 16 + i, (system_io.relay_states >> i) & 1);
    }
}

/**
  * @brief  Write outputs to 74HC595 shift registers
  *
  * LED Mapping (based on schematic):
  * U1 (Chip 0): D_IN1 to D_IN8   (Digital Inputs)
  * U2 (Chip 1): D_IN9 to D_IN16  (Digital Inputs cont.)
  * U3 (Chip 2): D_O1 to D_O8     (Digital Outputs)
  * U4 (Chip 3): A_O1, A_O2, SYS, ERR, CON, EX1, EX2 (Analog + Status)
  */
void Write_74HC595_Outputs(void)
{
    // Update Input LEDs from system state
    Update_Input_LEDs();

    // Update Output LEDs from relay states
    Update_Output_LEDs();

    // Latch all data to outputs
    HC595_Update(&hc595_outputs);
}


/**
  * @brief  System status report
  */
//void System_Status_Report(void)
//{
//    uint32_t current_time = HAL_GetTick();
//    
//    if (current_time - last_status_time >= 5000) {  // Every 5 seconds
//        last_status_time = current_time;
//        
//        uart_printf("\r\n+--------------------------------+\r\n");
//        uart_printf("�     PLC STATUS REPORT          �\r\n");
//        uart_printf("�--------------------------------�\r\n");
//        uart_printf("� Uptime: %lu seconds            \r\n", current_time / 1000);
//        uart_printf("� Node ID: %d                    \r\n", system_io.node_id);
//        uart_printf("� PLC State: %s                  \r\n", plc_running ? "RUNNING" : "STOPPED");
//        uart_printf("� Program: %s                    \r\n", PLC_IsProgramLoaded() ? "LOADED" : "NOT LOADED");
//        uart_printf("� Cycles: %lu                    \r\n", PLC_GetCycleCount());
//        uart_printf("� Max Cycle: %lu us              \r\n", PLC_GetMaxCycleTime());
//        uart_printf("�--------------------------------�\r\n");
//        uart_printf("� Digital Inputs (DIN1-4):       �\r\n");
//        uart_printf("�   ");
//        for(int i = 0; i < 4; i++) {
//            uart_printf("DIN%d:%d ", i+1, (system_io.digital_inputs >> i) & 1);
//        }
//        uart_printf("\r\n");
//        uart_printf("� Relay Outputs (1-8):           �\r\n");
//        uart_printf("�   ");
//        for(int i = 0; i < 8; i++) {
//            uart_printf("R%d:%d ", i+1, (system_io.relay_states >> i) & 1);
//        }
//        uart_printf("\r\n");
//        uart_printf("� Analog Inputs:                 �\r\n");
//        uart_printf("�   A1:%4d A2:%4d             \r\n", system_io.analog_inputs[0], system_io.analog_inputs[1]);
//        uart_printf("�   A3:%4d A4:%4d             \r\n", system_io.analog_inputs[2], system_io.analog_inputs[3]);
//        uart_printf("+--------------------------------+\r\n");
//    }
//}

/**
  * @brief  Process UART command
  */
void Process_UART_Command(char cmd)
{
    switch(cmd) {
        case 'S':  // Stop PLC
        case 's':
            plc_running = false;
            uart_printf("\r\n? PLC STOPPED\r\n");
            // Turn off all relays for safety
            system_io.relay_states = 0x00;
            Write_Relay_Outputs();
            Update_Status_LEDs();
            break;
            
        case 'R':  // Run PLC
        case 'r':
            plc_running = true;
            uart_printf("\r\n? PLC RUNNING\r\n");
            break;
            
        case 'D':  // Toggle debug
        case 'd':
            debug_mode = !debug_mode;
            uart_printf("\r\n? Debug mode: %s\r\n", debug_mode ? "ON" : "OFF");
            break;
            
        case 'T':  // Test relays
        case 't':
            uart_printf("\r\n? Testing relays...\r\n");
            for(int i = 0; i < 8; i++) {
                system_io.relay_states = 1 << i;
                Write_Relay_Outputs();
                Update_Status_LEDs();
                uart_printf("  Relay %d ON\r\n", i+1);
                HAL_Delay(200);
            }
            system_io.relay_states = 0x00;
            Write_Relay_Outputs();
            Update_Status_LEDs();
            uart_printf("  Test complete\r\n");
            break;
            
        case 'I':  // Show inputs
        case 'i':
            Read_Digital_Inputs();
            Read_Analog_Inputs();
            uart_printf("\r\n? Input Status:\r\n");
            uart_printf("  Digital: ");
            for(int i = 0; i < 4; i++) {
                uart_printf("DIN%d=%d ", i+1, (system_io.digital_inputs >> i) & 1);
            }
            uart_printf("\r\n  Analog: ");
            for(int i = 0; i < 4; i++) {
                uart_printf("AIN%d=%4d ", i+1, system_io.analog_inputs[i]);
            }
            uart_printf("\r\n");
            break;
            
        case 'O':  // Show outputs
        case 'o':
            uart_printf("\r\n? Output Status:\r\n");
            uart_printf("  Relays: ");
            for(int i = 0; i < 8; i++) {
                uart_printf("R%d=%d ", i+1, (system_io.relay_states >> i) & 1);
            }
            uart_printf("\r\n");
            // Show 74HC595 outputs
            uart_printf("  HC595: ");
            for(int i = 0; i < HC595_LED_NUM_CHIPS * 8; i++) {
                uart_printf("O%d=%d ", i, HC595_GetPin(&hc595_outputs, i));
            }
            uart_printf("\r\n");
            break;

        case 'J':  // Send HMI status packet (machine-readable format)
        case 'j':
            Send_HMI_Status();
            break;

        case 'C':  // HSC (High-Speed Counter) - DISABLED (module removed)
        case 'c':
            uart_printf("\r\n  HSC module not available (removed)\r\n");
            break;

        case '1':  // HSC_1 - DISABLED
        case '2':  // HSC_2 - DISABLED
        case '0':  // Stop HSC - DISABLED
            uart_printf("\r\n  HSC module not available (removed)\r\n");
            break;

        case 'P':  // Read PT100
        case 'p':
            uart_printf("\r\n  Reading PT100 Temperature Sensors...\r\n");
            Read_PT100_Temperature();

            // Sensor 1
            uart_printf("\r\n  PT100 #1:\r\n");
            if (pt100_fault1 == 0) {
                uart_printf("    Temperature: %.2f C\r\n", pt100_temperature1);
                uart_printf("    Resistance:  %.2f Ohm\r\n", pt100_resistance1);
            } else {
                uart_printf("    FAULT DETECTED (0x%02X):\r\n", pt100_fault1);
                if (pt100_fault1 & MAX31865_FAULT_HIGHTHRESH)
                    uart_printf("      - RTD High Threshold\r\n");
                if (pt100_fault1 & MAX31865_FAULT_LOWTHRESH)
                    uart_printf("      - RTD Low Threshold\r\n");
                if (pt100_fault1 & MAX31865_FAULT_REFINLOW)
                    uart_printf("      - REFIN- > 0.85 x V_BIAS\r\n");
                if (pt100_fault1 & MAX31865_FAULT_REFINHIGH)
                    uart_printf("      - REFIN- < 0.85 x V_BIAS (FORCE- open)\r\n");
                if (pt100_fault1 & MAX31865_FAULT_RTDINLOW)
                    uart_printf("      - RTDIN- < 0.85 x V_BIAS (FORCE- open)\r\n");
                if (pt100_fault1 & MAX31865_FAULT_OVUV)
                    uart_printf("      - Overvoltage/Undervoltage\r\n");
            }

            // Sensor 2
            uart_printf("\r\n  PT100 #2:\r\n");
            if (pt100_fault2 == 0) {
                uart_printf("    Temperature: %.2f C\r\n", pt100_temperature2);
                uart_printf("    Resistance:  %.2f Ohm\r\n", pt100_resistance2);
            } else {
                uart_printf("    FAULT DETECTED (0x%02X):\r\n", pt100_fault2);
                if (pt100_fault2 & MAX31865_FAULT_HIGHTHRESH)
                    uart_printf("      - RTD High Threshold\r\n");
                if (pt100_fault2 & MAX31865_FAULT_LOWTHRESH)
                    uart_printf("      - RTD Low Threshold\r\n");
                if (pt100_fault2 & MAX31865_FAULT_REFINLOW)
                    uart_printf("      - REFIN- > 0.85 x V_BIAS\r\n");
                if (pt100_fault2 & MAX31865_FAULT_REFINHIGH)
                    uart_printf("      - REFIN- < 0.85 x V_BIAS (FORCE- open)\r\n");
                if (pt100_fault2 & MAX31865_FAULT_RTDINLOW)
                    uart_printf("      - RTDIN- < 0.85 x V_BIAS (FORCE- open)\r\n");
                if (pt100_fault2 & MAX31865_FAULT_OVUV)
                    uart_printf("      - Overvoltage/Undervoltage\r\n");
            }
            break;

        case 'H':  // Test 74HC595 - Sequential LED test
        case 'h':
            uart_printf("\r\n  Testing 32 LEDs sequentially...\r\n");
            uart_printf("  U1: D_IN1-8, U2: D_IN9-16, U3: D_O1-8, U4: Status\r\n\r\n");

            // Clear all LEDs first
            HC595_Clear(&hc595_outputs);
            HC595_Update(&hc595_outputs);

            // Sequential ON test - each LED turns on one by one
            uart_printf("  Phase 1: Sequential ON\r\n");
            for(int i = 0; i < 32; i++) {
                Set_LED(i, true);
                HC595_Update(&hc595_outputs);

                // Show which LED
                if (i < 8) {
                    uart_printf("    LED %02d: D_IN%d\r\n", i, i + 1);
                } else if (i < 16) {
                    uart_printf("    LED %02d: D_IN%d\r\n", i, i + 1);
                } else if (i < 24) {
                    uart_printf("    LED %02d: D_O%d\r\n", i, i - 15);
                } else {
                    const char* status_names[] = {"A_O1", "A_O2", "SYS", "ERR", "CON", "EX1", "EX2", "N/A"};
                    uart_printf("    LED %02d: %s\r\n", i, status_names[i - 24]);
                }
                HAL_Delay(100);
            }

            // All LEDs ON for 1 second
            uart_printf("\r\n  Phase 2: All LEDs ON\r\n");
            HAL_Delay(1000);

            // Sequential OFF test - each LED turns off one by one
            uart_printf("  Phase 3: Sequential OFF\r\n");
            for(int i = 31; i >= 0; i--) {
                Set_LED(i, false);
                HC595_Update(&hc595_outputs);
                HAL_Delay(50);
            }

            uart_printf("\r\n  Test complete!\r\n");

            // Restore status LEDs
            Set_Status_LED(STATUS_LED_SYS, true);
            Set_Status_LED(STATUS_LED_CON, true);
            HC595_Update(&hc595_outputs);
            break;

        case 'L':  // All LEDs ON/OFF toggle
        case 'l':
            {
                static bool all_leds_on = false;
                all_leds_on = !all_leds_on;

                uart_printf("\r\n  All LEDs: %s\r\n", all_leds_on ? "ON" : "OFF");

                for(int i = 0; i < 32; i++) {
                    Set_LED(i, all_leds_on);
                }
                HC595_Update(&hc595_outputs);
            }
            break;

        case 'W':  // Read DS18B20
        case 'w':
            uart_printf("\r\n  Reading DS18B20 Temperature...\r\n");
            if (ds18b20_connected) {
                Read_DS18B20_Temperature();
                if (ds18b20_connected) {
                    uart_printf("    Temperature: %.2f C\r\n", ds18b20_temperature);
                    uart_printf("    ROM: %02X-%02X%02X%02X%02X%02X%02X-%02X\r\n",
                               ds18b20_sensor.rom_code[0],
                               ds18b20_sensor.rom_code[1],
                               ds18b20_sensor.rom_code[2],
                               ds18b20_sensor.rom_code[3],
                               ds18b20_sensor.rom_code[4],
                               ds18b20_sensor.rom_code[5],
                               ds18b20_sensor.rom_code[6],
                               ds18b20_sensor.rom_code[7]);
                } else {
                    uart_printf("    ERROR: Sensor disconnected\r\n");
                }
            } else {
                uart_printf("    DS18B20 not connected\r\n");
            }
            break;

        case 'U':  // UART1 Modbus test
        case 'u':
            uart_printf("\r\n  UART1 (Modbus) Test\r\n");
            uart_printf("  Sending test message to UART1...\r\n");
            {
                // Test message to UART1
                const char* test_msg = "UART1 Test OK\r\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)test_msg, strlen(test_msg), 100);
                uart_printf("  Message sent: %s", test_msg);

                // Show UART1 configuration
                uart_printf("  UART1 Config:\r\n");
                uart_printf("    Baudrate: %lu\r\n", huart1.Init.BaudRate);
                uart_printf("    WordLength: %s\r\n",
                    huart1.Init.WordLength == UART_WORDLENGTH_8B ? "8-bit" : "9-bit");
                uart_printf("    StopBits: %s\r\n",
                    huart1.Init.StopBits == UART_STOPBITS_1 ? "1" : "2");
                uart_printf("    Parity: %s\r\n",
                    huart1.Init.Parity == UART_PARITY_NONE ? "None" :
                    huart1.Init.Parity == UART_PARITY_EVEN ? "Even" : "Odd");
            }
            break;

        case 'M':
        case 'm':
            uart_printf("\r\n  Modbus module removed\r\n");
            break;

        case 'F':  // SPI Flash Test
        case 'f':
            uart_printf("\r\n  SST25VF016B SPI Flash Test\r\n");
            uart_printf("  ==========================\r\n");

            if (!spi_flash_connected) {
                uart_printf("  ERROR: Flash not connected!\r\n");
                uart_printf("  Attempting to reinitialize...\r\n");
                Init_SPI_Flash();
                if (!spi_flash_connected) {
                    uart_printf("  Flash initialization failed.\r\n");
                    break;
                }
            }

            // Read JEDEC ID
            {
                uint8_t mfr, type, cap;
                W25Q_Status status = W25Q_ReadJEDECID(&spi_flash, &mfr, &type, &cap);
                if (status == W25Q_OK) {
                    uart_printf("\r\n  JEDEC ID:\r\n");
                    uart_printf("    Manufacturer: 0x%02X (SST/Microchip)\r\n", mfr);
                    uart_printf("    Memory Type:  0x%02X\r\n", type);
                    uart_printf("    Capacity:     0x%02X (16Mbit/2MB)\r\n", cap);
                } else {
                    uart_printf("  ERROR: Failed to read JEDEC ID (err=%d)\r\n", status);
                    break;
                }
            }

            // Read Status Register (SST25 has only one SR)
            {
                uint8_t sr;
                SST25_ReadStatusReg(&spi_flash, &sr);
                uart_printf("\r\n  Status Register:\r\n");
                uart_printf("    SR: 0x%02X (BUSY=%d, WEL=%d, AAI=%d)\r\n",
                           sr, sr & 0x01, (sr >> 1) & 0x01, (sr >> 6) & 0x01);
            }

            // Read/Write Test
            {
                uart_printf("\r\n  Read/Write Test (Sector 0):\r\n");

                // Read first 16 bytes
                uint8_t read_buf[16];
                W25Q_Status status = W25Q_Read(&spi_flash, 0x000000, read_buf, 16);
                if (status == W25Q_OK) {
                    uart_printf("    Current data at 0x000000:\r\n    ");
                    for (int i = 0; i < 16; i++) {
                        uart_printf("%02X ", read_buf[i]);
                    }
                    uart_printf("\r\n");
                }

                // Ask user before erasing
                uart_printf("\r\n    Press 'Y' to erase sector 0 and write test data,\r\n");
                uart_printf("    or any other key to skip write test.\r\n");
            }
            break;

        case 'X':  // YMODEM File Upload
        case 'x':
            uart_printf("\r\n  === YMODEM FILE UPLOAD ===\r\n");
            if (!spi_flash_connected || !ffs_initialized) {
                uart_printf("  ERROR: Flash/FFS not ready!\r\n");
                break;
            }
            {
                uart_printf("  Start YMODEM transfer from terminal:\r\n");
                uart_printf("    Tera Term: File -> Transfer -> YMODEM -> Send\r\n");
                uart_printf("  Waiting for file...\r\n\r\n");

                // CRITICAL: Stop UART interrupt before YMODEM (polling mode)
                HAL_UART_AbortReceive_IT(&huart3);

                // Clear UART RX buffer and wait for terminal to be ready
                __HAL_UART_FLUSH_DRREGISTER(&huart3);
                while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE)) {
                    (void)huart3.Instance->DR;  // Discard any pending bytes
                }
                HAL_Delay(100);  // Give terminal time to prepare for YMODEM

                YMODEM_Flash_Result result;
                YMODEM_Status status = YMODEM_Flash_ReceiveToFFS(
                    &ymodem_flash,
                    1048576,  // 1MB max
                    FFS_TYPE_PLC_PROGRAM,
                    &result
                );

                if (status == YMODEM_OK) {
                    uart_printf("\r\n  SUCCESS!\r\n");
                    uart_printf("    Filename: %s\r\n", result.filename);
                    uart_printf("    Size: %lu bytes\r\n", result.filesize);
                    uart_printf("    Received: %lu bytes\r\n", result.bytes_received);

                    // Config file handling - DISABLED (modules removed)
                    // For now, restart system for any uploaded file
                    uart_printf("\r\n  Restarting system in 2 seconds...\r\n");
                    HAL_Delay(2000);
                    NVIC_SystemReset();
                } else {
                    uart_printf("\r\n  ERROR: %s\r\n", YMODEM_GetStatusString(status));
                }
                uart_printf("  ===========================\r\n");

                // Re-enable UART receive interrupt after YMODEM complete
                HAL_UART_Receive_IT(&huart3, uart3_rx_buffer, 1);
            }
            break;

        case 'G':  // List FFS Files
        case 'g':
            uart_printf("\r\n");
            if (!ffs_initialized) {
                uart_printf("  FFS not initialized!\r\n");
                break;
            }
            FFS_ListFiles(&flash_fs);
            break;

        case 'B':  // Read plc.bin from flash (test SST25_Read)
        case 'b':
            {
                uart_printf("\r\n  === PLC.BIN FLASH READ TEST ===\r\n");

                // Check FFS initialization
                if (!ffs_initialized) {
                    uart_printf("  ERROR: FFS not initialized!\r\n");
                    break;
                }
                uart_printf("  FFS initialized: OK\r\n");

                // Check if file exists
                const char* plc_filename = "plc.bin";
                int file_idx = FFS_FindFile(&flash_fs, plc_filename);
                if (file_idx < 0) {
                    uart_printf("  ERROR: File '%s' not found!\r\n", plc_filename);
                    uart_printf("  Use 'G' to list files, 'X' to upload.\r\n");
                    break;
                }
                uart_printf("  File exists: OK (index=%d)\r\n", file_idx);

                // Get file entry directly from table
                FFS_FileEntry* entry = &flash_fs.table.files[file_idx];
                uart_printf("  File size: %lu bytes\r\n", entry->file_size);
                uart_printf("  File CRC32: 0x%08lX\r\n", entry->crc32);
                uart_printf("  File type: %d\r\n", entry->file_type);
                uart_printf("  Flash address: 0x%06lX\r\n", entry->start_address);
                uart_printf("  Allocated size: %lu bytes\r\n", entry->allocated_size);

                // TEST 1: Direct SST25_Read (bypass FFS)
                uart_printf("\r\n  --- TEST 1: Direct SST25_Read ---\r\n");
                uint8_t read_buffer[64];
                memset(read_buffer, 0xAA, 64);  // Fill with pattern to detect if read happens

                uart_printf("  Reading 64 bytes from 0x%06lX...\r\n", entry->start_address);
                SST25_Status sst_status = SST25_Read(&spi_flash, entry->start_address, read_buffer, 64);
                uart_printf("  SST25_Read status: %d\r\n", sst_status);

                // Hex dump direct read
                uart_printf("  Direct read HEX:\r\n");
                for (int i = 0; i < 64; i++) {
                    if (i % 16 == 0) uart_printf("  %04X: ", i);
                    uart_printf("%02X ", read_buffer[i]);
                    if (i % 16 == 15) uart_printf("\r\n");
                }

                // TEST 2: Read from File Table address (0x000000)
                uart_printf("\r\n  --- TEST 2: Read File Table (0x000000) ---\r\n");
                uint8_t table_buf[32];
                SST25_Read(&spi_flash, 0x000000, table_buf, 32);
                uart_printf("  Table magic: %02X %02X %02X %02X",
                           table_buf[0], table_buf[1], table_buf[2], table_buf[3]);
                if (table_buf[0] == 0x31 && table_buf[1] == 0x53 &&
                    table_buf[2] == 0x46 && table_buf[3] == 0x46) {
                    uart_printf(" = 'FFS1' OK\r\n");
                } else if (table_buf[0] == 0xFF) {
                    uart_printf(" = ERASED (FF)\r\n");
                } else {
                    uart_printf(" = UNKNOWN\r\n");
                }

                // TEST 3: Read from Data Area start (0x001000)
                uart_printf("\r\n  --- TEST 3: Read Data Area (0x001000) ---\r\n");
                uint8_t data_buf[32];
                SST25_Read(&spi_flash, 0x001000, data_buf, 32);
                uart_printf("  Data area HEX:\r\n  ");
                for (int i = 0; i < 32; i++) {
                    uart_printf("%02X ", data_buf[i]);
                    if (i % 16 == 15) uart_printf("\r\n  ");
                }
                uart_printf("\r\n");

                // TEST 4: FFS_Read
                uart_printf("\r\n  --- TEST 4: FFS_Read ---\r\n");
                uint8_t ffs_buffer[64];
                uint32_t bytes_read = 0;
                memset(ffs_buffer, 0xBB, 64);
                FFS_Status ffs_status = FFS_Read(&flash_fs, plc_filename, ffs_buffer, 0, 64, &bytes_read);
                uart_printf("  FFS_Read status: %d (%s)\r\n", ffs_status, FFS_GetStatusString(ffs_status));
                uart_printf("  Bytes read: %lu\r\n", bytes_read);
                uart_printf("  FFS read HEX:\r\n");
                for (int i = 0; i < 64; i++) {
                    if (i % 16 == 0) uart_printf("  %04X: ", i);
                    uart_printf("%02X ", ffs_buffer[i]);
                    if (i % 16 == 15) uart_printf("\r\n");
                }

                // Compare direct vs FFS
                uart_printf("\r\n  --- COMPARISON ---\r\n");
                bool match = (memcmp(read_buffer, ffs_buffer, 64) == 0);
                uart_printf("  Direct vs FFS: %s\r\n", match ? "MATCH" : "DIFFERENT");

                uart_printf("  ==============================\r\n");
            }
            break;

        case 'Y':  // Confirm Flash Write Test
        case 'y':
            if (!spi_flash_connected) {
                uart_printf("\r\n  Flash not connected. Press 'F' first.\r\n");
                break;
            }
            {
                uart_printf("\r\n  Erasing Sector 0...\r\n");
                W25Q_Status status = W25Q_EraseSector(&spi_flash, 0);
                if (status != W25Q_OK) {
                    uart_printf("    ERROR: Erase failed (err=%d)\r\n", status);
                    break;
                }
                uart_printf("    Sector 0 erased.\r\n");

                // Verify erase (should be all 0xFF)
                uint8_t verify_buf[16];
                W25Q_Read(&spi_flash, 0x000000, verify_buf, 16);
                uart_printf("    After erase: ");
                for (int i = 0; i < 16; i++) {
                    uart_printf("%02X ", verify_buf[i]);
                }
                uart_printf("\r\n");

                // Write test pattern
                uint8_t test_data[16] = {0x48, 0x4D, 0x49, 0x5F, 0x50, 0x4C, 0x43, 0x00,  // "HMI_PLC\0"
                                         0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
                uart_printf("\r\n  Writing test data...\r\n");
                status = W25Q_Write(&spi_flash, 0x000000, test_data, 16);
                if (status != W25Q_OK) {
                    uart_printf("    ERROR: Write failed (err=%d)\r\n", status);
                    break;
                }

                // Read back and verify
                uint8_t read_back[16];
                W25Q_Read(&spi_flash, 0x000000, read_back, 16);
                uart_printf("    After write: ");
                for (int i = 0; i < 16; i++) {
                    uart_printf("%02X ", read_back[i]);
                }
                uart_printf("\r\n");
                uart_printf("    As string: \"%.8s\"\r\n", read_back);

                // Verify
                bool match = true;
                for (int i = 0; i < 16; i++) {
                    if (read_back[i] != test_data[i]) {
                        match = false;
                        break;
                    }
                }
                uart_printf("\r\n    Verification: %s\r\n", match ? "PASSED" : "FAILED");
            }
            break;

        case 'Z':  // Format Flash File System
        case 'z':
            {
                uart_printf("\r\n  === FORMAT FLASH FILE SYSTEM ===\r\n");
                uart_printf("  WARNING: This will ERASE ALL FILES!\r\n");
                uart_printf("  Press 'Y' to confirm, any other key to cancel...\r\n");

								HAL_UART_AbortReceive_IT(&huart3);
							
                // Wait for confirmation
                uint8_t confirm;
                HAL_StatusTypeDef hal_status = HAL_UART_Receive(&huart3, &confirm, 1, 5000);
                if (hal_status != HAL_OK || (confirm != 'Y' && confirm != 'y')) {
                    uart_printf("  Cancelled.\r\n");
                    break;
                }

                uart_printf("  Formatting...\r\n");

                if (!spi_flash_connected) {
                    uart_printf("  ERROR: Flash not connected!\r\n");
                    break;
                }

                FFS_Status status = FFS_Format(&flash_fs);
                if (status == FFS_OK) {
                    ffs_initialized = true;
                    uart_printf("  SUCCESS: Flash formatted!\r\n");
                    uart_printf("  File table initialized.\r\n");
                    uart_printf("  Free space: %lu bytes\r\n", FFS_GetFreeSpace(&flash_fs));
                } else {
                    uart_printf("  ERROR: Format failed (err=%d: %s)\r\n",
                               status, FFS_GetStatusString(status));
                }
                uart_printf("  ================================\r\n");
								
                // Re-enable UART receive interrupt after YMODEM complete
                HAL_UART_Receive_IT(&huart3, uart3_rx_buffer, 1);
								
            }
            break;

        case '?':  // Help
            uart_printf("\r\n+--------------------------------+\r\n");
            uart_printf("|       PLC COMMANDS             |\r\n");
            uart_printf("|--------------------------------|\r\n");
            uart_printf("| S - Stop PLC                   |\r\n");
            uart_printf("| R - Run PLC                    |\r\n");
            uart_printf("| D - Toggle debug mode          |\r\n");
            uart_printf("| T - Test relays                |\r\n");
            uart_printf("| H - Test 32 LEDs sequentially  |\r\n");
            uart_printf("| L - Toggle all LEDs ON/OFF     |\r\n");
            uart_printf("| P - Read PT100 temperature     |\r\n");
            uart_printf("| W - Read DS18B20 temperature   |\r\n");
            uart_printf("| U - UART1 (Modbus) test        |\r\n");
            uart_printf("| M - Modbus status              |\r\n");
            uart_printf("| I - Show inputs                |\r\n");
            uart_printf("| O - Show outputs               |\r\n");
            uart_printf("| J - HMI status packet          |\r\n");
            uart_printf("| C - HSC (High-Speed Counter)   |\r\n");
            uart_printf("| E - EEPROM test                |\r\n");
            uart_printf("| F - SPI Flash (SST25VF016B)    |\r\n");
            uart_printf("| X - YMODEM file upload         |\r\n");
            uart_printf("| G - List flash files (FFS)     |\r\n");
            uart_printf("| B - Read plc.bin (test read)   |\r\n");
            uart_printf("| Z - Format flash (ERASE ALL)   |\r\n");
            uart_printf("| ? - This help                  |\r\n");
            uart_printf("+--------------------------------+\r\n");
            break;
            
        case '\r':
        case '\n':
            // Ignore
            break;
            
        default:
            uart_printf("\r\n? Unknown command: '%c'\r\n", cmd);
    }
}

/**
  * @brief  UART receive callback
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        // UART1: Reserved
        HAL_UART_Receive_IT(&huart1, uart1_rx_buffer, 1);
    }
    else if (huart->Instance == USART3) {
        // UART3: Command interface
        char received = (char)uart3_rx_buffer[0];

        // Echo character back
        HAL_UART_Transmit(&huart3, uart3_rx_buffer, 1, 10);

        // Set flag for main loop to process (don't call Process_UART_Command here)
        if (received != '\r' && received != '\n') {
            pending_command = received;
            command_ready = 1;
        }

        HAL_UART_Receive_IT(&huart3, uart3_rx_buffer, 1);
    }
}

/**
  * @brief Watchdog timeout callback
  */
static void plc_watchdog_timeout_handler(void)
{
    uart_printf("\r\n!!! PLC WATCHDOG TIMEOUT !!!\r\n");

    // Emergency stop
    watchdog_emergency_stop();
    
    // Auto-reset if production mode
    #ifndef DEBUG
        HAL_Delay(1000);
        HAL_NVIC_SystemReset();
    #endif
}

/**
  * @brief Watchdog warning callback
  */
static void plc_watchdog_warning_handler(uint32_t remaining_ms)
{
    static uint32_t last_warning = 0;
    uint32_t now = HAL_GetTick();
    
    if (now - last_warning > 1000) {
        uart_printf("[WDG WARNING] %lums remaining\r\n", remaining_ms);
        last_warning = now;
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
	int status_led = 0;
	int i;
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
  MX_ADC1_Init();
//  MX_IWDG_Init();
  MX_USART3_UART_Init();
  MX_CAN_Init();
  MX_I2C1_Init();
  MX_UART4_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	
  uart_printf("\033[2J\033[H");  // Clear screen
  uart_printf("╔════════════════════════════════════════╗\r\n");
  uart_printf("║   STM32F103RCT6 PLC RUNTIME v3.0       ║\r\n");
  uart_printf("║   Custom Board with I/O Addressing     ║\r\n");
  uart_printf("║   Build: " __DATE__ " " __TIME__ "        ║\r\n");
  uart_printf("╚════════════════════════════════════════╝\r\n\r\n");
	
	
  // Check reset source
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
      uart_printf("!!! SYSTEM WAS RESET BY WATCHDOG !!!\r\n");
      __HAL_RCC_CLEAR_RESET_FLAGS();
  }

// Initialize Watchdog System
  WatchdogError wdg_err;
  #ifndef DEBUG
      wdg_err = watchdog_init(false, true, 500);  // HW disabled in debug, SW 500ms
      watchdog_sw_configure(500, false);  // No auto-reset
  #else
      wdg_err = watchdog_init(true, true, 200);  // Both enabled, 100ms timeout
      watchdog_sw_configure(200, true);   // Auto-reset on timeout
  #endif
  
  if (wdg_err != WDG_OK) {
      uart_printf("ERROR: Watchdog init failed (code: %d)\r\n", wdg_err);
  } else {
      watchdog_register_timeout_callback(plc_watchdog_timeout_handler);
      watchdog_register_warning_callback(plc_watchdog_warning_handler);
      uart_printf("► Watchdog system initialized\r\n");
  }
  
  // Start Timer 3 for watchdog service
  HAL_TIM_Base_Start_IT(&htim3);	
	
	// Revizyon
	
	CAN_FilterTypeDef filter = {0};

	filter.FilterBank = 0;
	filter.FilterMode = CAN_FILTERMODE_IDMASK;
	filter.FilterScale = CAN_FILTERSCALE_32BIT;

	/* Only Extended frames (IDE=1) */
	filter.FilterIdHigh     = 0x0000;
	filter.FilterIdLow      = 0x0004;  // IDE bit = 1
	filter.FilterMaskIdHigh = 0x0000;
	filter.FilterMaskIdLow  = 0x0004;  // mask IDE only

	filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter.FilterActivation = ENABLE;
	filter.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&hcan, &filter) != HAL_OK) Error_Handler();
	if (HAL_CAN_Start(&hcan) != HAL_OK) Error_Handler();
	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) Error_Handler();


	// Revizyon Sonu 
	
	
  // Initialize ADC calibration
  HAL_ADCEx_Calibration_Start(&hadc1);
  uart_printf("► ADC calibrated\r\n");

  // Initialize 74HC595 shift registers
  Init_74HC595();

  // Set initial status LEDs
//  Set_Status_LED(STATUS_LED_SYS, true);   // System LED ON
	Set_LED(LED_POWER, true);

//  Set_Status_LED(STATUS_LED_CON, true);   // Connected LED ON
  Write_74HC595_Outputs();  // Update immediately


	HC595_SetPin(&hc595_control, EEP2_BIT, true);
	HC595_SetPin(&hc595_control, EEP3_BIT, true);
	HC595_SetPin(&hc595_control, CTRL_PT_CS2, true);
	HC595_SetPin(&hc595_control, CTRL_SPI2_CS, true);
	
	for(i=0;i<16;i++)
			HC595_SetPin(&hc595_control, i, true);
	
	HC595_Update(&hc595_control);
	
  // Initialize PT100 temperature sensor
  Init_PT100();	
  // Initialize DS18B20 OneWire sensor
  Init_DS18B20();

  // Initialize SPI Flash (W25Q128JW)
  Init_SPI_Flash();

	uart_printf("► System Ready\r\n");

  // Start UART receive interrupts
  HAL_UART_Receive_IT(&huart1, uart1_rx_buffer, 1);  // UART1
  HAL_UART_Receive_IT(&huart3, uart3_rx_buffer, 1);  // UART3 for commands
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	uint32_t target_cycle_ms = 10;  // 10ms target cycle time	
	uint32_t heartLed = 0;
	
	
  while (1)
  {
		 uint32_t start = HAL_GetTick();

		 //HAL_TIM_Base_Start_IT(&htim3);

		 // Process UART3 command in main loop
		 if (command_ready) {
		     char cmd = pending_command;
		     command_ready = 0;
		     Process_UART_Command(cmd);
		 }

			Write_74HC595_Outputs();

			if( HAL_GetTick() - heartLed  > (rand() % 500) )
			{
				HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
				heartLed = HAL_GetTick();
				Set_LED(LED_SYS,status_led ? true : false);
				status_led = ~status_led;
			}

			// Read PT100 sensors periodically (every 500ms)
			if (HAL_GetTick() - last_pt100_read >= 500) {
				last_pt100_read = HAL_GetTick();
				Read_PT100_Temperature();
			}

			uint32_t elapsed = HAL_GetTick() - start;
			main_loop_elapsed_ms = elapsed;  // Store for HMI status
			if (elapsed < target_cycle_ms) {
					// plc_perf_idle_start();  // DISABLED - module removed
					HAL_Delay(target_cycle_ms - elapsed);
					// plc_perf_idle_end();    // DISABLED - module removed
			}
			
				
			
			
//			/* Process Modbus requests - ADD THIS */
//			plc_io_process_modbus();		
			
			
 

      // Print watchdog status every 10 seconds
//      static uint32_t last_wdg_status = 0;
//      if (HAL_GetTick() - last_wdg_status > 10000) {
//          watchdog_print_status();
//          last_wdg_status = HAL_GetTick();
//      }
			

  		
//    uint32_t current_time = HAL_GetTick();
//    
//    // PLC scan cycle (20ms)
//    if (current_time - last_cycle_time >= CYCLE_TIME_MS) {
//        last_cycle_time = current_time;
//        
//        if (plc_running) {
//					PLC_Cycle();  // Her şeyi içeride halleder
//					Update_Status_LEDs();  // Sadece görsel güncelleme
//        }
//    }
//    
//    // Heartbeat LED (1Hz)
//    if (current_time - last_heartbeat >= 500) {
//        last_heartbeat = current_time;
//        HAL_GPIO_TogglePin(STATUS_LED_Port, STATUS_LED_Pin);
//    }
//    
//    // System status report
//    System_Status_Report();
		
		    // PB2 pinini toggle yap
    //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
//    uart_printf("Program version:\r\n");
//    // 1 saniye bekle
//    HAL_Delay(1000);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
//  hcan.Instance = CAN1;
//  hcan.Init.Prescaler = 16;
//  hcan.Init.Mode = CAN_MODE_NORMAL;
//  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
//  hcan.Init.TimeSeg1 = CAN_BS1_1TQ;
//  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
//  hcan.Init.TimeTriggeredMode = DISABLE;
//  hcan.Init.AutoBusOff = DISABLE;
//  hcan.Init.AutoWakeUp = DISABLE;
//  hcan.Init.AutoRetransmission = DISABLE;
//  hcan.Init.ReceiveFifoLocked = DISABLE;
//  hcan.Init.TransmitFifoPriority = DISABLE;
	
	hcan.Instance = CAN1;
	hcan.Init.Prescaler = 12;
	hcan.Init.Mode = CAN_MODE_NORMAL;
	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
	hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
	hcan.Init.TimeTriggeredMode = DISABLE;
	hcan.Init.AutoBusOff = ENABLE;          // Önerilir
	hcan.Init.AutoWakeUp = ENABLE;          // Önerilir
	hcan.Init.AutoRetransmission = ENABLE;  // ZAPI için şart
	hcan.Init.ReceiveFifoLocked = DISABLE;
	hcan.Init.TransmitFifoPriority = DISABLE;

  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

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
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

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
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8
                          |GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC5 PC6 PC7 PC8
                           PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA8 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_8|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB12
                           PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 PB8
                           PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

	/* USER CODE BEGIN 4 */

	__weak void Can_OnRx(const CAN_RxHeaderTypeDef *hdr, const uint8_t data[8])
	{
		(void)hdr;
		(void)data;

		/* Burayı senin parser/handler akışınla dolduracağız.
			 Örn: StdId/ExtId’ye göre switch-case, payload decode, state update vs.
		*/
	}

	void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
	{
		if (hcan->Instance != CAN1) return;

		if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &sCanRxHeader, sCanRxData) != HAL_OK) {
			return; /* İstersen error counter artır */
		}

		/* Debug counters */
		sCanRxCount++;
		
		if (sCanRxHeader.IDE == CAN_ID_EXT) {
				sCanRxLastId = sCanRxHeader.ExtId;
		} else {
				sCanRxLastId = sCanRxHeader.StdId;
		}


		/* Burada profesyonel yaklaşım:
			 - ISR'da ağır iş yapma
			 - minimum kopyalama + bayrak/queue
			 Ama hızlı başlangıç için doğrudan handler çağırabilirsin.
		*/

		/* Örnek: kullanıcı handler fonksiyonu */
		Can_OnRx(&sCanRxHeader, sCanRxData);
	}	
	

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
     ex: uart_printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
