/*
 * max31865.c
 * MAX31865 RTD to Digital Converter Driver Implementation
 */

#include <max31865.h>
#include <math.h>

// =============================================================================
// REGISTER ADDRESSES
// =============================================================================

#define MAX31865_REG_CONFIG         0x00
#define MAX31865_REG_RTD_MSB        0x01
#define MAX31865_REG_RTD_LSB        0x02
#define MAX31865_REG_HIGH_FAULT_MSB 0x03
#define MAX31865_REG_HIGH_FAULT_LSB 0x04
#define MAX31865_REG_LOW_FAULT_MSB  0x05
#define MAX31865_REG_LOW_FAULT_LSB  0x06
#define MAX31865_REG_FAULT_STATUS   0x07

// Configuration Register Bits
#define MAX31865_CONFIG_BIAS        0x80
#define MAX31865_CONFIG_MODEAUTO    0x40
#define MAX31865_CONFIG_MODEOFF     0x00
#define MAX31865_CONFIG_1SHOT       0x20
#define MAX31865_CONFIG_3WIRE       0x10
#define MAX31865_CONFIG_FAULTSTAT   0x0C
#define MAX31865_CONFIG_FILT50HZ    0x01

// Read/Write bit
#define MAX31865_READ_BIT           0x00
#define MAX31865_WRITE_BIT          0x80

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Short delay for software SPI
 */
static inline void MAX31865_Delay(void) {
    for(volatile int i = 0; i < 5; i++);
}

/**
 * @brief Software SPI transfer byte
 */
static uint8_t MAX31865_SPI_TransferByte_SW(MAX31865_Handle* handle, uint8_t data) {
    uint8_t result = 0;

    for(int i = 7; i >= 0; i--) {
        // Set MOSI
        HAL_GPIO_WritePin(handle->mosi_port, handle->mosi_pin,
                         (data & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        MAX31865_Delay();

        // Clock high
        HAL_GPIO_WritePin(handle->sclk_port, handle->sclk_pin, GPIO_PIN_SET);
        MAX31865_Delay();

        // Read MISO
        if(HAL_GPIO_ReadPin(handle->miso_port, handle->miso_pin) == GPIO_PIN_SET) {
            result |= (1 << i);
        }

        // Clock low
        HAL_GPIO_WritePin(handle->sclk_port, handle->sclk_pin, GPIO_PIN_RESET);
        MAX31865_Delay();
    }

    return result;
}

#if MAX31865_ENABLE_SPI_MODE
/**
 * @brief Hardware SPI transfer byte
 */
static uint8_t MAX31865_SPI_TransferByte_HW(MAX31865_Handle* handle, uint8_t data) {
    uint8_t result = 0;

    if(handle->hspi != NULL) {
        HAL_SPI_TransmitReceive(handle->hspi, &data, &result, 1, 100);
    }

    return result;
}
#endif

/**
 * @brief SPI transfer byte (auto select SW or HW)
 */
static uint8_t MAX31865_SPI_Transfer(MAX31865_Handle* handle, uint8_t data) {
#if MAX31865_ENABLE_SPI_MODE
    if(handle->use_hardware_spi) {
        return MAX31865_SPI_TransferByte_HW(handle, data);
    } else {
        return MAX31865_SPI_TransferByte_SW(handle, data);
    }
#else
    return MAX31865_SPI_TransferByte_SW(handle, data);
#endif
}

/**
 * @brief Chip Select control
 */
static void MAX31865_CS_Low(MAX31865_Handle* handle) {
    if (handle->cs_callback != NULL) {
        // Use callback for external CS control (e.g., HC595)
        handle->cs_callback(false);  // false = CS low (active)
    } else {
        // Use direct GPIO
        HAL_GPIO_WritePin(handle->cs_port, handle->cs_pin, GPIO_PIN_RESET);
    }
    MAX31865_Delay();
}

static void MAX31865_CS_High(MAX31865_Handle* handle) {
    MAX31865_Delay();
    if (handle->cs_callback != NULL) {
        // Use callback for external CS control (e.g., HC595)
        handle->cs_callback(true);  // true = CS high (inactive)
    } else {
        // Use direct GPIO
        HAL_GPIO_WritePin(handle->cs_port, handle->cs_pin, GPIO_PIN_SET);
    }
    MAX31865_Delay();
}

/**
 * @brief Read register
 */
static uint8_t MAX31865_ReadRegister(MAX31865_Handle* handle, uint8_t addr) {
    uint8_t result;

    MAX31865_CS_Low(handle);
    MAX31865_SPI_Transfer(handle, addr & 0x7F);  // Read bit = 0
    result = MAX31865_SPI_Transfer(handle, 0xFF);
    MAX31865_CS_High(handle);

    return result;
}

/**
 * @brief Write register
 */
static void MAX31865_WriteRegister(MAX31865_Handle* handle, uint8_t addr, uint8_t data) {
    MAX31865_CS_Low(handle);
    MAX31865_SPI_Transfer(handle, addr | 0x80);  // Write bit = 1
    MAX31865_SPI_Transfer(handle, data);
    MAX31865_CS_High(handle);
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

MAX31865_Status MAX31865_Init(MAX31865_Handle* handle) {
    if(handle == NULL) {
        return MAX31865_ERROR_INIT;
    }

    // Initialize GPIO pins for software SPI
    if(!handle->use_hardware_spi) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        // SCLK - Output
        GPIO_InitStruct.Pin = handle->sclk_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(handle->sclk_port, &GPIO_InitStruct);

        // MOSI - Output
        GPIO_InitStruct.Pin = handle->mosi_pin;
        HAL_GPIO_Init(handle->mosi_port, &GPIO_InitStruct);

        // MISO - Input
        GPIO_InitStruct.Pin = handle->miso_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(handle->miso_port, &GPIO_InitStruct);

        // Initial states
        HAL_GPIO_WritePin(handle->sclk_port, handle->sclk_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(handle->mosi_port, handle->mosi_pin, GPIO_PIN_RESET);
    }

    // CS - Output (always software controlled)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = handle->cs_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(handle->cs_port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(handle->cs_port, handle->cs_pin, GPIO_PIN_SET);  // CS high (inactive)

    // DRDY - Input (optional)
    if(handle->drdy_enabled) {
        GPIO_InitStruct.Pin = handle->drdy_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(handle->drdy_port, &GPIO_InitStruct);
    }

    // Configure MAX31865
    uint8_t config = 0;

    // Wire mode
    if(handle->wire_mode == MAX31865_3WIRE) {
        config |= MAX31865_CONFIG_3WIRE;
    }

    // Filter mode
    if(handle->filter_mode == MAX31865_FILTER_50HZ) {
        config |= MAX31865_CONFIG_FILT50HZ;
    }

    // Write configuration
    MAX31865_WriteRegister(handle, MAX31865_REG_CONFIG, config);

    // Clear any faults
    MAX31865_ClearFault(handle);

    handle->initialized = true;

    return MAX31865_OK;
}

MAX31865_Status MAX31865_SetBias(MAX31865_Handle* handle, bool enable) {
    if(!handle->initialized) {
        return MAX31865_ERROR_INIT;
    }

    uint8_t config = MAX31865_ReadRegister(handle, MAX31865_REG_CONFIG);

    if(enable) {
        config |= MAX31865_CONFIG_BIAS;
    } else {
        config &= ~MAX31865_CONFIG_BIAS;
    }

    MAX31865_WriteRegister(handle, MAX31865_REG_CONFIG, config);

    // Wait for bias voltage to stabilize
    if(enable) {
        HAL_Delay(10);
    }

    return MAX31865_OK;
}

MAX31865_Status MAX31865_TriggerConversion(MAX31865_Handle* handle) {
    if(!handle->initialized) {
        return MAX31865_ERROR_INIT;
    }

    uint8_t config = MAX31865_ReadRegister(handle, MAX31865_REG_CONFIG);
    config |= MAX31865_CONFIG_1SHOT;
    MAX31865_WriteRegister(handle, MAX31865_REG_CONFIG, config);

    // Wait for conversion (typical 65ms for 50Hz filter)
    HAL_Delay(70);

    return MAX31865_OK;
}

MAX31865_Status MAX31865_ReadRawRTD(MAX31865_Handle* handle, uint16_t* rtd_value) {
    if(!handle->initialized || rtd_value == NULL) {
        return MAX31865_ERROR_INIT;
    }

    // Read RTD MSB and LSB
    uint8_t msb = MAX31865_ReadRegister(handle, MAX31865_REG_RTD_MSB);
    uint8_t lsb = MAX31865_ReadRegister(handle, MAX31865_REG_RTD_LSB);

    // Combine to 15-bit value (bit 0 is fault bit)
    *rtd_value = ((uint16_t)msb << 7) | (lsb >> 1);

    // Check fault bit
    if(lsb & 0x01) {
        handle->fault_status = MAX31865_ReadRegister(handle, MAX31865_REG_FAULT_STATUS);
        return MAX31865_ERROR_FAULT;
    }

    return MAX31865_OK;
}

MAX31865_Status MAX31865_ReadResistance(MAX31865_Handle* handle, float* resistance) {
    if(!handle->initialized || resistance == NULL) {
        return MAX31865_ERROR_INIT;
    }

    uint16_t rtd_raw;
    MAX31865_Status status = MAX31865_ReadRawRTD(handle, &rtd_raw);

    if(status != MAX31865_OK) {
        return status;
    }

    // Calculate resistance: R_RTD = (RTD_DATA / 32768) * R_REF
    *resistance = ((float)rtd_raw / 32768.0f) * handle->ref_resistor;

    return MAX31865_OK;
}

MAX31865_Status MAX31865_ReadTemperature(MAX31865_Handle* handle, float* temperature) {
    if(!handle->initialized || temperature == NULL) {
        return MAX31865_ERROR_INIT;
    }

    float resistance;
    MAX31865_Status status = MAX31865_ReadResistance(handle, &resistance);

    if(status != MAX31865_OK) {
        return status;
    }

    *temperature = MAX31865_RTD_to_Temperature(resistance, handle->rtd_type);

    return MAX31865_OK;
}

uint8_t MAX31865_GetFault(MAX31865_Handle* handle) {
    if(!handle->initialized) {
        return 0xFF;
    }

    return MAX31865_ReadRegister(handle, MAX31865_REG_FAULT_STATUS);
}

MAX31865_Status MAX31865_ClearFault(MAX31865_Handle* handle) {
    if(!handle->initialized) {
        return MAX31865_ERROR_INIT;
    }

    uint8_t config = MAX31865_ReadRegister(handle, MAX31865_REG_CONFIG);
    config &= ~0x2C;  // Clear fault status bits D3, D2
    config |= 0x02;   // Set D1 to clear fault
    MAX31865_WriteRegister(handle, MAX31865_REG_CONFIG, config);

    handle->fault_status = 0;

    return MAX31865_OK;
}

// =============================================================================
// COMMUNICATION VERIFICATION
// =============================================================================

MAX31865_Status MAX31865_VerifyCommunication(MAX31865_Handle* handle, uint8_t* config_value) {
    if(handle == NULL) {
        return MAX31865_ERROR_INIT;
    }

    // Read config register
    uint8_t read1 = MAX31865_ReadRegister(handle, MAX31865_REG_CONFIG);

    // Write a known pattern (0x01 = 50Hz filter only)
    MAX31865_WriteRegister(handle, MAX31865_REG_CONFIG, 0x01);
    HAL_Delay(1);

    // Read back
    uint8_t read2 = MAX31865_ReadRegister(handle, MAX31865_REG_CONFIG);

    // Restore original config
    MAX31865_WriteRegister(handle, MAX31865_REG_CONFIG, read1);

    if (config_value != NULL) {
        *config_value = read2;
    }

    // If all 0xFF or all 0x00, no chip connected
    if (read2 == 0xFF || read2 == 0x00) {
        return MAX31865_ERROR_SPI;
    }

    // If read back contains what we wrote (filter bit), SPI works
    if ((read2 & 0x01) == 0x01) {
        return MAX31865_OK;
    }

    return MAX31865_ERROR_INIT;
}

// =============================================================================
// TEMPERATURE CONVERSION
// =============================================================================

float MAX31865_RTD_to_Temperature(float resistance, MAX31865_RTDType rtd_type) {
    // Constants for PT100 (IEC 60751)
    const float R0 = (rtd_type == MAX31865_RTD_PT100) ? 100.0f : 1000.0f;
    const float A = 3.9083e-3f;
    const float B = -5.775e-7f;

    // Simplified Callendar-Van Dusen equation for 0°C to 850°C
    // T = (-R0*A + sqrt(R0²*A² - 4*R0*B*(R0 - R))) / (2*R0*B)

    float temp_C;

    // For temperature > 0°C, use simplified quadratic equation
    if(resistance >= R0) {
        temp_C = (-R0 * A + sqrtf(R0 * R0 * A * A - 4 * R0 * B * (R0 - resistance))) / (2 * R0 * B);
    }
    // For temperature < 0°C, use linear approximation
    else {
        temp_C = (resistance - R0) / (R0 * A);
    }

    return temp_C;
}
