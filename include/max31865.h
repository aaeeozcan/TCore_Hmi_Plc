/*
 * max31865.h
 * MAX31865 RTD to Digital Converter Driver
 * For PT100/PT1000 Temperature Sensors
 */

#ifndef MAX31865_H
#define MAX31865_H

#include <stdint.h>
#include <stdbool.h>
#pragma once
#include <stm32f1xx_hal.h>




// =============================================================================
// FEATURE CONFIGURATION
// =============================================================================

// Set to 1 to enable SPI hardware mode support
// Set to 0 to use only software (bit-bang) mode
// NOTE: SPI1 is shared with SST25VF016B flash. Mode is switched dynamically:
//       - Flash: Mode 0 (CPOL=0, CPHA=0)
//       - MAX31865: Mode 1 (CPOL=0, CPHA=1)
#define MAX31865_ENABLE_SPI_MODE    1

#if MAX31865_ENABLE_SPI_MODE
// Forward declaration for SPI - only needed if SPI mode is enabled
//typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
#endif

// =============================================================================
// CONFIGURATION
// =============================================================================

// RTD Type
typedef enum {
    MAX31865_RTD_PT100 = 0,   // 100 ohm RTD
    MAX31865_RTD_PT1000 = 1   // 1000 ohm RTD
} MAX31865_RTDType;

// Wire Configuration
typedef enum {
    MAX31865_2WIRE = 0,
    MAX31865_3WIRE = 1,
    MAX31865_4WIRE = 0
} MAX31865_WireMode;

// Filter Mode
typedef enum {
    MAX31865_FILTER_60HZ = 0,
    MAX31865_FILTER_50HZ = 1
} MAX31865_FilterMode;

// Error Codes
typedef enum {
    MAX31865_OK = 0,
    MAX31865_ERROR_INIT,
    MAX31865_ERROR_SPI,
    MAX31865_ERROR_FAULT,
    MAX31865_ERROR_TIMEOUT
} MAX31865_Status;

// Fault Flags
typedef enum {
    MAX31865_FAULT_NONE = 0x00,
    MAX31865_FAULT_HIGHTHRESH = 0x80,
    MAX31865_FAULT_LOWTHRESH = 0x40,
    MAX31865_FAULT_REFINLOW = 0x20,
    MAX31865_FAULT_REFINHIGH = 0x10,
    MAX31865_FAULT_RTDINLOW = 0x08,
    MAX31865_FAULT_OVUV = 0x04
} MAX31865_Fault;

// =============================================================================
// DATA STRUCTURES
// =============================================================================

// CS control callback function type
typedef void (*MAX31865_CS_Callback)(bool state);

typedef struct {
    // GPIO pins (software SPI mode)
    GPIO_TypeDef* sclk_port;
    uint16_t sclk_pin;

    GPIO_TypeDef* mosi_port;
    uint16_t mosi_pin;

    GPIO_TypeDef* miso_port;
    uint16_t miso_pin;

    GPIO_TypeDef* cs_port;
    uint16_t cs_pin;

    GPIO_TypeDef* drdy_port;   // Optional
    uint16_t drdy_pin;
    bool drdy_enabled;

    // CS callback for external control (e.g., via HC595)
    // If set, this callback is used instead of direct GPIO
    MAX31865_CS_Callback cs_callback;

    // Hardware SPI (if used)
#if MAX31865_ENABLE_SPI_MODE
    SPI_HandleTypeDef* hspi;
#else
    void* hspi;                // Placeholder (not used in software mode)
#endif
    bool use_hardware_spi;

    // Configuration
    MAX31865_RTDType rtd_type;
    MAX31865_WireMode wire_mode;
    MAX31865_FilterMode filter_mode;
    float ref_resistor;        // Reference resistor value (ohms)

    // State
    bool initialized;
    uint8_t fault_status;

} MAX31865_Handle;

// =============================================================================
// API FUNCTIONS
// =============================================================================

/**
 * @brief Initialize MAX31865
 * @param handle: Pointer to MAX31865 handle
 * @return MAX31865_Status
 */
MAX31865_Status MAX31865_Init(MAX31865_Handle* handle);

/**
 * @brief Read RTD resistance
 * @param handle: Pointer to MAX31865 handle
 * @param resistance: Pointer to store resistance value (ohms)
 * @return MAX31865_Status
 */
MAX31865_Status MAX31865_ReadResistance(MAX31865_Handle* handle, float* resistance);

/**
 * @brief Read temperature (Celsius)
 * @param handle: Pointer to MAX31865 handle
 * @param temperature: Pointer to store temperature value (°C)
 * @return MAX31865_Status
 */
MAX31865_Status MAX31865_ReadTemperature(MAX31865_Handle* handle, float* temperature);

/**
 * @brief Read raw RTD value
 * @param handle: Pointer to MAX31865 handle
 * @param rtd_value: Pointer to store raw 15-bit RTD value
 * @return MAX31865_Status
 */
MAX31865_Status MAX31865_ReadRawRTD(MAX31865_Handle* handle, uint16_t* rtd_value);

/**
 * @brief Check for faults
 * @param handle: Pointer to MAX31865 handle
 * @return Fault status byte (0 = no fault)
 */
uint8_t MAX31865_GetFault(MAX31865_Handle* handle);

/**
 * @brief Clear fault status
 * @param handle: Pointer to MAX31865 handle
 * @return MAX31865_Status
 */
MAX31865_Status MAX31865_ClearFault(MAX31865_Handle* handle);

/**
 * @brief Enable/disable bias voltage
 * @param handle: Pointer to MAX31865 handle
 * @param enable: true to enable bias, false to disable
 * @return MAX31865_Status
 */
MAX31865_Status MAX31865_SetBias(MAX31865_Handle* handle, bool enable);

/**
 * @brief Trigger one-shot conversion
 * @param handle: Pointer to MAX31865 handle
 * @return MAX31865_Status
 */
MAX31865_Status MAX31865_TriggerConversion(MAX31865_Handle* handle);

/**
 * @brief Verify SPI communication by reading config register
 * @param handle: Pointer to MAX31865 handle
 * @param config_value: Pointer to store read config value (can be NULL)
 * @return MAX31865_OK if communication successful, error otherwise
 */
MAX31865_Status MAX31865_VerifyCommunication(MAX31865_Handle* handle, uint8_t* config_value);

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Convert resistance to temperature using Callendar-Van Dusen equation
 * @param resistance: RTD resistance in ohms
 * @param rtd_type: RTD type (PT100 or PT1000)
 * @return Temperature in Celsius
 */
float MAX31865_RTD_to_Temperature(float resistance, MAX31865_RTDType rtd_type);

#endif // MAX31865_H
