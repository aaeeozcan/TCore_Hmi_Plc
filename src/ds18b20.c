/*
 * ds18b20.c
 * DS18B20 OneWire Temperature Sensor Driver Implementation
 */

#include <ds18b20.h>

// =============================================================================
// ONEWIRE COMMANDS
// =============================================================================

#define DS18B20_CMD_SEARCH_ROM      0xF0
#define DS18B20_CMD_READ_ROM        0x33
#define DS18B20_CMD_MATCH_ROM       0x55
#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_ALARM_SEARCH    0xEC

#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_WRITE_SCRATCH   0x4E
#define DS18B20_CMD_READ_SCRATCH    0xBE
#define DS18B20_CMD_COPY_SCRATCH    0x48
#define DS18B20_CMD_RECALL_E2       0xB8
#define DS18B20_CMD_READ_POWER      0xB4

// =============================================================================
// TIMING (microseconds)
// =============================================================================

#define ONEWIRE_DELAY_A     6       // Write 1 low time
#define ONEWIRE_DELAY_B     64      // Write 1 high time
#define ONEWIRE_DELAY_C     60      // Write 0 low time
#define ONEWIRE_DELAY_D     10      // Write 0 high time
#define ONEWIRE_DELAY_E     9       // Read sample time
#define ONEWIRE_DELAY_F     55      // Read release time
#define ONEWIRE_DELAY_G     0       // Reset high time (before)
#define ONEWIRE_DELAY_H     480     // Reset low time
#define ONEWIRE_DELAY_I     70      // Reset presence wait
#define ONEWIRE_DELAY_J     410     // Reset presence release

// =============================================================================
// PRIVATE FUNCTIONS - TIMING
// =============================================================================

/**
 * @brief Microsecond delay using DWT cycle counter
 */
static void delay_us(uint32_t us)
{
    // Simple busy-wait delay
    // For 72MHz clock, each cycle is ~13.9ns
    // 1us = ~72 cycles
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles);
}

/**
 * @brief Enable DWT cycle counter for microsecond delays
 */
static void DWT_Init(void)
{
    // Enable TRC
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // Reset cycle counter
    DWT->CYCCNT = 0;
    // Enable cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// =============================================================================
// PRIVATE FUNCTIONS - GPIO
// =============================================================================

/**
 * @brief Set pin as output (push-pull)
 */
static void OneWire_SetOutput(DS18B20_Handle* handle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = handle->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  // Open-drain for OneWire
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(handle->port, &GPIO_InitStruct);
}

/**
 * @brief Set pin as input
 */
static void OneWire_SetInput(DS18B20_Handle* handle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = handle->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(handle->port, &GPIO_InitStruct);
}

/**
 * @brief Write pin state
 */
static void OneWire_WriteBit(DS18B20_Handle* handle, uint8_t bit)
{
    HAL_GPIO_WritePin(handle->port, handle->pin, bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief Read pin state
 */
static uint8_t OneWire_ReadBit(DS18B20_Handle* handle)
{
    return HAL_GPIO_ReadPin(handle->port, handle->pin) == GPIO_PIN_SET ? 1 : 0;
}

// =============================================================================
// PRIVATE FUNCTIONS - ONEWIRE PROTOCOL
// =============================================================================

/**
 * @brief OneWire reset pulse
 * @return true if presence pulse detected, false otherwise
 */
static bool OneWire_Reset(DS18B20_Handle* handle)
{
    uint8_t presence;

    OneWire_SetOutput(handle);
    OneWire_WriteBit(handle, 0);        // Pull low
    delay_us(ONEWIRE_DELAY_H);          // Hold low for 480us

    OneWire_WriteBit(handle, 1);        // Release
    delay_us(ONEWIRE_DELAY_I);          // Wait for presence

    OneWire_SetInput(handle);
    presence = !OneWire_ReadBit(handle); // Read presence (low = present)

    delay_us(ONEWIRE_DELAY_J);          // Wait for release

    return presence;
}

/**
 * @brief Write a bit to OneWire bus
 */
static void OneWire_WriteBitRaw(DS18B20_Handle* handle, uint8_t bit)
{
    OneWire_SetOutput(handle);

    if (bit) {
        // Write 1
        OneWire_WriteBit(handle, 0);
        delay_us(ONEWIRE_DELAY_A);
        OneWire_WriteBit(handle, 1);
        delay_us(ONEWIRE_DELAY_B);
    } else {
        // Write 0
        OneWire_WriteBit(handle, 0);
        delay_us(ONEWIRE_DELAY_C);
        OneWire_WriteBit(handle, 1);
        delay_us(ONEWIRE_DELAY_D);
    }
}

/**
 * @brief Read a bit from OneWire bus
 */
static uint8_t OneWire_ReadBitRaw(DS18B20_Handle* handle)
{
    uint8_t bit;

    OneWire_SetOutput(handle);
    OneWire_WriteBit(handle, 0);
    delay_us(ONEWIRE_DELAY_A);

    OneWire_WriteBit(handle, 1);
    OneWire_SetInput(handle);
    delay_us(ONEWIRE_DELAY_E);

    bit = OneWire_ReadBit(handle);
    delay_us(ONEWIRE_DELAY_F);

    return bit;
}

/**
 * @brief Write a byte to OneWire bus
 */
static void OneWire_WriteByte(DS18B20_Handle* handle, uint8_t data)
{
    for (int i = 0; i < 8; i++) {
        OneWire_WriteBitRaw(handle, data & 0x01);
        data >>= 1;
    }
}

/**
 * @brief Read a byte from OneWire bus
 */
static uint8_t OneWire_ReadByte(DS18B20_Handle* handle)
{
    uint8_t data = 0;

    for (int i = 0; i < 8; i++) {
        if (OneWire_ReadBitRaw(handle)) {
            data |= (1 << i);
        }
    }

    return data;
}

// =============================================================================
// PRIVATE FUNCTIONS - CRC
// =============================================================================

/**
 * @brief Calculate CRC8 for OneWire
 */
static uint8_t OneWire_CRC8(uint8_t* data, uint8_t len)
{
    uint8_t crc = 0;

    for (int i = 0; i < len; i++) {
        uint8_t byte = data[i];
        for (int j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ byte) & 0x01;
            crc >>= 1;
            if (mix) {
                crc ^= 0x8C;  // Polynomial
            }
            byte >>= 1;
        }
    }

    return crc;
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

DS18B20_Status DS18B20_Init(DS18B20_Handle* handle)
{
    if (handle == NULL) {
        return DS18B20_ERROR_INIT;
    }

    // Initialize DWT for microsecond delays
    DWT_Init();

    // Initialize GPIO as open-drain with pull-up
    OneWire_SetOutput(handle);
    OneWire_WriteBit(handle, 1);  // Release bus

    // Check if device is present
    if (!OneWire_Reset(handle)) {
        return DS18B20_ERROR_NO_DEVICE;
    }

    // Set default resolution (12-bit)
    handle->resolution = DS18B20_RESOLUTION_12BIT;
    handle->parasite_power = false;
    handle->initialized = true;

    // Read ROM code
    DS18B20_ReadROM(handle);

    return DS18B20_OK;
}

DS18B20_Status DS18B20_ReadROM(DS18B20_Handle* handle)
{
    if (!handle->initialized && !OneWire_Reset(handle)) {
        return DS18B20_ERROR_NO_DEVICE;
    }

    OneWire_Reset(handle);
    OneWire_WriteByte(handle, DS18B20_CMD_READ_ROM);

    // Read 8 bytes (family code + serial + CRC)
    for (int i = 0; i < 8; i++) {
        handle->rom_code[i] = OneWire_ReadByte(handle);
    }

    // Verify CRC
    if (OneWire_CRC8(handle->rom_code, 7) != handle->rom_code[7]) {
        return DS18B20_ERROR_CRC;
    }

    return DS18B20_OK;
}

bool DS18B20_IsPresent(DS18B20_Handle* handle)
{
    return OneWire_Reset(handle);
}

DS18B20_Status DS18B20_StartConversion(DS18B20_Handle* handle)
{
    if (!handle->initialized) {
        return DS18B20_ERROR_INIT;
    }

    if (!OneWire_Reset(handle)) {
        return DS18B20_ERROR_NO_DEVICE;
    }

    OneWire_WriteByte(handle, DS18B20_CMD_SKIP_ROM);
    OneWire_WriteByte(handle, DS18B20_CMD_CONVERT_T);

    return DS18B20_OK;
}

bool DS18B20_IsConversionComplete(DS18B20_Handle* handle)
{
    OneWire_SetInput(handle);
    return OneWire_ReadBit(handle) == 1;
}

DS18B20_Status DS18B20_ReadTemperatureRaw(DS18B20_Handle* handle, float* temperature)
{
    uint8_t scratchpad[9];

    if (!handle->initialized) {
        return DS18B20_ERROR_INIT;
    }

    if (!OneWire_Reset(handle)) {
        return DS18B20_ERROR_NO_DEVICE;
    }

    // Read scratchpad
    OneWire_WriteByte(handle, DS18B20_CMD_SKIP_ROM);
    OneWire_WriteByte(handle, DS18B20_CMD_READ_SCRATCH);

    for (int i = 0; i < 9; i++) {
        scratchpad[i] = OneWire_ReadByte(handle);
    }

    // Verify CRC
    if (OneWire_CRC8(scratchpad, 8) != scratchpad[8]) {
        return DS18B20_ERROR_CRC;
    }

    // Calculate temperature
    int16_t raw_temp = (scratchpad[1] << 8) | scratchpad[0];

    // Apply resolution mask
    switch (handle->resolution) {
        case DS18B20_RESOLUTION_9BIT:
            raw_temp &= ~0x07;  // 0.5°C resolution
            break;
        case DS18B20_RESOLUTION_10BIT:
            raw_temp &= ~0x03;  // 0.25°C resolution
            break;
        case DS18B20_RESOLUTION_11BIT:
            raw_temp &= ~0x01;  // 0.125°C resolution
            break;
        case DS18B20_RESOLUTION_12BIT:
        default:
            // Full 0.0625°C resolution
            break;
    }

    *temperature = (float)raw_temp / 16.0f;

    return DS18B20_OK;
}

DS18B20_Status DS18B20_ReadTemperature(DS18B20_Handle* handle, float* temperature)
{
    DS18B20_Status status;

    // Start conversion
    status = DS18B20_StartConversion(handle);
    if (status != DS18B20_OK) {
        return status;
    }

    // Wait for conversion based on resolution
    uint16_t delay_ms;
    switch (handle->resolution) {
        case DS18B20_RESOLUTION_9BIT:
            delay_ms = 94;
            break;
        case DS18B20_RESOLUTION_10BIT:
            delay_ms = 188;
            break;
        case DS18B20_RESOLUTION_11BIT:
            delay_ms = 375;
            break;
        case DS18B20_RESOLUTION_12BIT:
        default:
            delay_ms = 750;
            break;
    }

    HAL_Delay(delay_ms);

    // Read temperature
    return DS18B20_ReadTemperatureRaw(handle, temperature);
}

DS18B20_Status DS18B20_SetResolution(DS18B20_Handle* handle, DS18B20_Resolution resolution)
{
    if (!handle->initialized) {
        return DS18B20_ERROR_INIT;
    }

    if (!OneWire_Reset(handle)) {
        return DS18B20_ERROR_NO_DEVICE;
    }

    // Write to scratchpad (TH, TL, Config)
    uint8_t config;
    switch (resolution) {
        case DS18B20_RESOLUTION_9BIT:
            config = 0x1F;  // R0=0, R1=0
            break;
        case DS18B20_RESOLUTION_10BIT:
            config = 0x3F;  // R0=1, R1=0
            break;
        case DS18B20_RESOLUTION_11BIT:
            config = 0x5F;  // R0=0, R1=1
            break;
        case DS18B20_RESOLUTION_12BIT:
        default:
            config = 0x7F;  // R0=1, R1=1
            break;
    }

    OneWire_WriteByte(handle, DS18B20_CMD_SKIP_ROM);
    OneWire_WriteByte(handle, DS18B20_CMD_WRITE_SCRATCH);
    OneWire_WriteByte(handle, 0x00);    // TH
    OneWire_WriteByte(handle, 0x00);    // TL
    OneWire_WriteByte(handle, config);  // Config

    handle->resolution = resolution;

    return DS18B20_OK;
}
