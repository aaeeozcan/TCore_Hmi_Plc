/*
 * hc595.c
 * 74HC595 Shift Register Driver Implementation
 */

#include <hc595.h>
#include <string.h>

// Validate configuration
#if HC595_ENABLE_SPI_MODE && !defined(HAL_SPI_MODULE_ENABLED)
#warning "HC595_ENABLE_SPI_MODE is enabled but SPI peripheral is not enabled in CubeMX"
#endif

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Short delay for bit-bang timing
 */
static inline void HC595_Delay(void) {
    // ~1us delay at 72MHz
    for(volatile int i = 0; i < 10; i++);
}

/**
 * @brief Shift out one byte (software mode)
 * @param handle: HC595 handle
 * @param data: Byte to shift out
 */
static void HC595_ShiftOutSW(HC595_Handle* handle, uint8_t data) {
    for(int i = 7; i >= 0; i--) {
        // Set data bit
        HAL_GPIO_WritePin(handle->data_port, handle->data_pin,
                         (data & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HC595_Delay();

        // Pulse clock
        HAL_GPIO_WritePin(handle->clock_port, handle->clock_pin, GPIO_PIN_SET);
        HC595_Delay();
        HAL_GPIO_WritePin(handle->clock_port, handle->clock_pin, GPIO_PIN_RESET);
        HC595_Delay();
    }
}

#if HC595_ENABLE_SPI_MODE
/**
 * @brief Shift out one byte (hardware SPI mode)
 * @param handle: HC595 handle
 * @param data: Byte to shift out
 * @return HC595_Status
 */
static HC595_Status HC595_ShiftOutHW(HC595_Handle* handle, uint8_t data) {
    if(handle->hspi == NULL) {
        return HC595_ERROR_PARAM;
    }

    HAL_StatusTypeDef status = HAL_SPI_Transmit(handle->hspi, &data, 1, 100);

    if(status != HAL_OK) {
        return HC595_ERROR_TIMEOUT;
    }

    return HC595_OK;
}
#endif

/**
 * @brief Latch the data to outputs
 * @param handle: HC595 handle
 */
static void HC595_Latch(HC595_Handle* handle) {
    // Pulse latch pin (rising edge transfers shift register to output)
    HAL_GPIO_WritePin(handle->latch_port, handle->latch_pin, GPIO_PIN_RESET);
    HC595_Delay();
    HAL_GPIO_WritePin(handle->latch_port, handle->latch_pin, GPIO_PIN_SET);
    HC595_Delay();
    HAL_GPIO_WritePin(handle->latch_port, handle->latch_pin, GPIO_PIN_RESET);
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

HC595_Status HC595_Init(HC595_Handle* handle) {
    if(handle == NULL) {
        return HC595_ERROR_PARAM;
    }

    if(handle->num_chips == 0 || handle->num_chips > HC595_MAX_CHIPS) {
        return HC595_ERROR_PARAM;
    }

    // Initialize control pins
    if(handle->mode == HC595_MODE_SOFTWARE) {
        // Configure GPIO pins for bit-bang mode
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        // Data pin
        GPIO_InitStruct.Pin = handle->data_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(handle->data_port, &GPIO_InitStruct);

        // Clock pin
        GPIO_InitStruct.Pin = handle->clock_pin;
        HAL_GPIO_Init(handle->clock_port, &GPIO_InitStruct);

        // Set initial states
        HAL_GPIO_WritePin(handle->data_port, handle->data_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(handle->clock_port, handle->clock_pin, GPIO_PIN_RESET);
    }

    // Latch pin (always software controlled)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = handle->latch_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(handle->latch_port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(handle->latch_port, handle->latch_pin, GPIO_PIN_RESET);

    // Output Enable pin (optional)
    if(handle->oe_active) {
        GPIO_InitStruct.Pin = handle->oe_pin;
        HAL_GPIO_Init(handle->oe_port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(handle->oe_port, handle->oe_pin, GPIO_PIN_RESET); // Enable outputs
    }

    // Clear output buffer
    memset(handle->output_buffer, 0, HC595_MAX_CHIPS);

    // Send initial state (all outputs LOW)
    HC595_Update(handle);

    handle->initialized = true;

    return HC595_OK;
}

HC595_Status HC595_SetPin(HC595_Handle* handle, uint8_t pin, bool state) {
    if(handle == NULL || !handle->initialized) {
        return HC595_ERROR_INIT;
    }

    if(pin >= (handle->num_chips * 8)) {
        return HC595_ERROR_PARAM;
    }

    uint8_t chip_index = pin / 8;
    uint8_t bit_index = pin % 8;

    if(state) {
        handle->output_buffer[chip_index] |= (1 << bit_index);
    } else {
        handle->output_buffer[chip_index] &= ~(1 << bit_index);
    }

    return HC595_OK;
}

bool HC595_GetPin(HC595_Handle* handle, uint8_t pin) {
    if(handle == NULL || !handle->initialized) {
        return false;
    }

    if(pin >= (handle->num_chips * 8)) {
        return false;
    }

    uint8_t chip_index = pin / 8;
    uint8_t bit_index = pin % 8;

    return (handle->output_buffer[chip_index] & (1 << bit_index)) != 0;
}

HC595_Status HC595_SetChip(HC595_Handle* handle, uint8_t chip_index, uint8_t value) {
    if(handle == NULL || !handle->initialized) {
        return HC595_ERROR_INIT;
    }

    if(chip_index >= handle->num_chips) {
        return HC595_ERROR_PARAM;
    }

    handle->output_buffer[chip_index] = value;

    return HC595_OK;
}

uint8_t HC595_GetChip(HC595_Handle* handle, uint8_t chip_index) {
    if(handle == NULL || !handle->initialized) {
        return 0;
    }

    if(chip_index >= handle->num_chips) {
        return 0;
    }

    return handle->output_buffer[chip_index];
}

HC595_Status HC595_SetAll(HC595_Handle* handle, const uint8_t* data, uint8_t length) {
    if(handle == NULL || !handle->initialized || data == NULL) {
        return HC595_ERROR_INIT;
    }

    if(length > handle->num_chips) {
        length = handle->num_chips;
    }

    memcpy(handle->output_buffer, data, length);

    return HC595_OK;
}

HC595_Status HC595_Update(HC595_Handle* handle) {
    if(handle == NULL || !handle->initialized) {
        return HC595_ERROR_INIT;
    }

#if HC595_ENABLE_SPI_MODE
    HC595_Status status = HC595_OK;
#endif

    // Shift out data for all chips (MSB chip first for daisy-chain)
    for(int i = handle->num_chips - 1; i >= 0; i--) {
        if(handle->mode == HC595_MODE_SOFTWARE) {
            HC595_ShiftOutSW(handle, handle->output_buffer[i]);
        }
#if HC595_ENABLE_SPI_MODE
        else {
            status = HC595_ShiftOutHW(handle, handle->output_buffer[i]);
            if(status != HC595_OK) {
                return status;
            }
        }
#endif
    }

    // Latch data to outputs
    HC595_Latch(handle);

    return HC595_OK;
}

HC595_Status HC595_Clear(HC595_Handle* handle) {
    if(handle == NULL || !handle->initialized) {
        return HC595_ERROR_INIT;
    }

    memset(handle->output_buffer, 0, handle->num_chips);

    return HC595_Update(handle);
}

HC595_Status HC595_Enable(HC595_Handle* handle) {
    if(handle == NULL || !handle->initialized) {
        return HC595_ERROR_INIT;
    }

    if(handle->oe_active) {
        HAL_GPIO_WritePin(handle->oe_port, handle->oe_pin, GPIO_PIN_RESET); // Active LOW
    }

    return HC595_OK;
}

HC595_Status HC595_Disable(HC595_Handle* handle) {
    if(handle == NULL || !handle->initialized) {
        return HC595_ERROR_INIT;
    }

    if(handle->oe_active) {
        HAL_GPIO_WritePin(handle->oe_port, handle->oe_pin, GPIO_PIN_SET); // Active LOW
    }

    return HC595_OK;
}

HC595_Status HC595_TogglePin(HC595_Handle* handle, uint8_t pin) {
    if(handle == NULL || !handle->initialized) {
        return HC595_ERROR_INIT;
    }

    bool current_state = HC595_GetPin(handle, pin);
    return HC595_SetPin(handle, pin, !current_state);
}

HC595_Status HC595_WriteBits(HC595_Handle* handle, uint8_t start_pin, uint32_t value, uint8_t num_bits) {
    if(handle == NULL || !handle->initialized) {
        return HC595_ERROR_INIT;
    }

    if(start_pin + num_bits > (handle->num_chips * 8)) {
        return HC595_ERROR_PARAM;
    }

    for(uint8_t i = 0; i < num_bits; i++) {
        bool bit_value = (value & (1 << i)) != 0;
        HC595_Status status = HC595_SetPin(handle, start_pin + i, bit_value);
        if(status != HC595_OK) {
            return status;
        }
    }

    return HC595_OK;
}

uint32_t HC595_ReadBits(HC595_Handle* handle, uint8_t start_pin, uint8_t num_bits) {
    if(handle == NULL || !handle->initialized) {
        return 0;
    }

    if(start_pin + num_bits > (handle->num_chips * 8) || num_bits > 32) {
        return 0;
    }

    uint32_t result = 0;

    for(uint8_t i = 0; i < num_bits; i++) {
        if(HC595_GetPin(handle, start_pin + i)) {
            result |= (1 << i);
        }
    }

    return result;
}
