/*
 * hc595.h
 * 74HC595 Shift Register Driver
 *
 * Features:
 * - Multiple 74HC595 cascade support
 * - Fast SPI-based or bit-bang operation
 * - Output enable/disable control
 * - Atomic register updates
 */

#ifndef HC595_H
#define HC595_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// =============================================================================
// FEATURE CONFIGURATION
// =============================================================================

// Set to 1 to enable SPI hardware mode support
// Set to 0 to use only software (bit-bang) mode
#define HC595_ENABLE_SPI_MODE    0

#if HC595_ENABLE_SPI_MODE
// Forward declaration for SPI - only needed if SPI mode is enabled
typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
#endif

// =============================================================================
// CONFIGURATION
// =============================================================================

// Maximum number of cascaded 74HC595 chips
#define HC595_MAX_CHIPS     4

// Total number of output pins (8 per chip)
#define HC595_TOTAL_PINS    (HC595_MAX_CHIPS * 8)

// =============================================================================
// DATA STRUCTURES
// =============================================================================

typedef enum {
    HC595_OK = 0,
    HC595_ERROR_INIT,
    HC595_ERROR_PARAM,
    HC595_ERROR_TIMEOUT
} HC595_Status;

typedef enum {
    HC595_MODE_SOFTWARE = 0  // Bit-bang mode
#if HC595_ENABLE_SPI_MODE
    ,HC595_MODE_HARDWARE     // Hardware SPI mode (only if enabled)
#endif
} HC595_Mode;

typedef struct {
    // Control pins
    GPIO_TypeDef* data_port;      // Serial Data (DS/SER)
    uint16_t data_pin;

    GPIO_TypeDef* clock_port;     // Shift Clock (SHCP/SRCLK)
    uint16_t clock_pin;

    GPIO_TypeDef* latch_port;     // Latch/Storage Clock (STCP/RCLK)
    uint16_t latch_pin;

    GPIO_TypeDef* oe_port;        // Output Enable (active low, optional)
    uint16_t oe_pin;
    bool oe_active;               // true if OE pin is connected

    // Configuration
    HC595_Mode mode;
#if HC595_ENABLE_SPI_MODE
    SPI_HandleTypeDef* hspi;      // Only used in hardware mode
#else
    void* hspi;                   // Placeholder (not used in software mode)
#endif
    uint8_t num_chips;            // Number of cascaded chips

    // State buffer
    uint8_t output_buffer[HC595_MAX_CHIPS];
    bool initialized;

} HC595_Handle;

// =============================================================================
// API FUNCTIONS
// =============================================================================

/**
 * @brief Initialize 74HC595 shift register(s)
 * @param handle: Pointer to HC595 handle structure
 * @return HC595_Status
 */
HC595_Status HC595_Init(HC595_Handle* handle);

/**
 * @brief Set a single output pin
 * @param handle: Pointer to HC595 handle
 * @param pin: Pin number (0 to HC595_TOTAL_PINS-1)
 * @param state: true = HIGH, false = LOW
 * @return HC595_Status
 */
HC595_Status HC595_SetPin(HC595_Handle* handle, uint8_t pin, bool state);

/**
 * @brief Get current state of a pin
 * @param handle: Pointer to HC595 handle
 * @param pin: Pin number
 * @return Current pin state (true/false)
 */
bool HC595_GetPin(HC595_Handle* handle, uint8_t pin);

/**
 * @brief Set entire register value for one chip
 * @param handle: Pointer to HC595 handle
 * @param chip_index: Chip number (0 to num_chips-1)
 * @param value: 8-bit value to set
 * @return HC595_Status
 */
HC595_Status HC595_SetChip(HC595_Handle* handle, uint8_t chip_index, uint8_t value);

/**
 * @brief Get current register value for one chip
 * @param handle: Pointer to HC595 handle
 * @param chip_index: Chip number
 * @return Current 8-bit value
 */
uint8_t HC595_GetChip(HC595_Handle* handle, uint8_t chip_index);

/**
 * @brief Set all outputs at once
 * @param handle: Pointer to HC595 handle
 * @param data: Array of values (one per chip)
 * @param length: Number of chips to update
 * @return HC595_Status
 */
HC595_Status HC595_SetAll(HC595_Handle* handle, const uint8_t* data, uint8_t length);

/**
 * @brief Update shift register outputs (latch data)
 * @param handle: Pointer to HC595 handle
 * @return HC595_Status
 */
HC595_Status HC595_Update(HC595_Handle* handle);

/**
 * @brief Clear all outputs (set to 0)
 * @param handle: Pointer to HC595 handle
 * @return HC595_Status
 */
HC595_Status HC595_Clear(HC595_Handle* handle);

/**
 * @brief Enable outputs (OE pin LOW)
 * @param handle: Pointer to HC595 handle
 * @return HC595_Status
 */
HC595_Status HC595_Enable(HC595_Handle* handle);

/**
 * @brief Disable outputs (OE pin HIGH)
 * @param handle: Pointer to HC595 handle
 * @return HC595_Status
 */
HC595_Status HC595_Disable(HC595_Handle* handle);

/**
 * @brief Toggle a single pin
 * @param handle: Pointer to HC595 handle
 * @param pin: Pin number
 * @return HC595_Status
 */
HC595_Status HC595_TogglePin(HC595_Handle* handle, uint8_t pin);

/**
 * @brief Write multiple bits starting at pin position
 * @param handle: Pointer to HC595 handle
 * @param start_pin: Starting pin number
 * @param value: Value to write
 * @param num_bits: Number of bits to write (1-32)
 * @return HC595_Status
 */
HC595_Status HC595_WriteBits(HC595_Handle* handle, uint8_t start_pin, uint32_t value, uint8_t num_bits);

/**
 * @brief Read multiple bits starting at pin position
 * @param handle: Pointer to HC595 handle
 * @param start_pin: Starting pin number
 * @param num_bits: Number of bits to read (1-32)
 * @return Value read from buffer
 */
uint32_t HC595_ReadBits(HC595_Handle* handle, uint8_t start_pin, uint8_t num_bits);

#endif // HC595_H
