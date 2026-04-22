/*
 * ds18b20.h
 * DS18B20 OneWire Temperature Sensor Driver
 */

#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"

// =============================================================================
// CONFIGURATION
// =============================================================================

// Resolution settings
typedef enum {
    DS18B20_RESOLUTION_9BIT = 0,    // 0.5°C, 93.75ms
    DS18B20_RESOLUTION_10BIT = 1,   // 0.25°C, 187.5ms
    DS18B20_RESOLUTION_11BIT = 2,   // 0.125°C, 375ms
    DS18B20_RESOLUTION_12BIT = 3    // 0.0625°C, 750ms (default)
} DS18B20_Resolution;

// Error codes
typedef enum {
    DS18B20_OK = 0,
    DS18B20_ERROR_NO_DEVICE,
    DS18B20_ERROR_CRC,
    DS18B20_ERROR_TIMEOUT,
    DS18B20_ERROR_INIT
} DS18B20_Status;

// =============================================================================
// DATA STRUCTURES
// =============================================================================

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    DS18B20_Resolution resolution;
    uint8_t rom_code[8];        // 64-bit ROM code
    bool initialized;
    bool parasite_power;        // True if using parasite power mode
} DS18B20_Handle;

// =============================================================================
// API FUNCTIONS
// =============================================================================

/**
 * @brief Initialize DS18B20 sensor
 * @param handle: Pointer to DS18B20 handle
 * @return DS18B20_Status
 */
DS18B20_Status DS18B20_Init(DS18B20_Handle* handle);

/**
 * @brief Read temperature from DS18B20
 * @param handle: Pointer to DS18B20 handle
 * @param temperature: Pointer to store temperature in Celsius
 * @return DS18B20_Status
 */
DS18B20_Status DS18B20_ReadTemperature(DS18B20_Handle* handle, float* temperature);

/**
 * @brief Start temperature conversion (non-blocking)
 * @param handle: Pointer to DS18B20 handle
 * @return DS18B20_Status
 */
DS18B20_Status DS18B20_StartConversion(DS18B20_Handle* handle);

/**
 * @brief Check if conversion is complete
 * @param handle: Pointer to DS18B20 handle
 * @return true if conversion complete, false otherwise
 */
bool DS18B20_IsConversionComplete(DS18B20_Handle* handle);

/**
 * @brief Read temperature after conversion started
 * @param handle: Pointer to DS18B20 handle
 * @param temperature: Pointer to store temperature in Celsius
 * @return DS18B20_Status
 */
DS18B20_Status DS18B20_ReadTemperatureRaw(DS18B20_Handle* handle, float* temperature);

/**
 * @brief Set temperature resolution
 * @param handle: Pointer to DS18B20 handle
 * @param resolution: Resolution setting
 * @return DS18B20_Status
 */
DS18B20_Status DS18B20_SetResolution(DS18B20_Handle* handle, DS18B20_Resolution resolution);

/**
 * @brief Read ROM code (64-bit unique ID)
 * @param handle: Pointer to DS18B20 handle
 * @return DS18B20_Status
 */
DS18B20_Status DS18B20_ReadROM(DS18B20_Handle* handle);

/**
 * @brief Check if device is present on bus
 * @param handle: Pointer to DS18B20 handle
 * @return true if device present, false otherwise
 */
bool DS18B20_IsPresent(DS18B20_Handle* handle);

#endif // DS18B20_H
