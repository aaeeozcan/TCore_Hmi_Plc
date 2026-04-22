/*
 * sst25vf016b.c
 * SST25VF016B 16M-bit (2MB) SPI Flash Driver
 * Microchip (SST) Serial Flash Memory
 *
 * Key differences from W25Q series:
 * - Uses AAI (Auto Address Increment) programming instead of Page Program
 * - Only 2MB capacity (vs 16MB for W25Q128)
 * - Different JEDEC ID (0xBF, 0x25, 0x41)
 */

#include <sst25vf016b.h>
#include <string.h>

// =============================================================================
// PRIVATE DEFINES
// =============================================================================

#define SST25_TIMEOUT_DEFAULT    1000    // Default timeout 1s
#define SST25_TIMEOUT_BYTE_PROG  10      // Byte program timeout ~10us typical
#define SST25_TIMEOUT_AAI_PROG   10      // AAI word program timeout ~10us
#define SST25_TIMEOUT_SECTOR     50      // Sector erase timeout 25ms typical
#define SST25_TIMEOUT_BLOCK_32K  50      // 32KB block erase timeout 25ms typical
#define SST25_TIMEOUT_BLOCK_64K  50      // 64KB block erase timeout 25ms typical
#define SST25_TIMEOUT_CHIP       100     // Chip erase timeout 50ms typical

// =============================================================================
// PRIVATE FUNCTION PROTOTYPES
// =============================================================================

static void SST25_CS_Low(SST25_Handle* handle);
static void SST25_CS_High(SST25_Handle* handle);
static uint8_t SST25_SPI_Transfer(SST25_Handle* handle, uint8_t data);
static void SST25_SPI_TransferBuffer(SST25_Handle* handle, uint8_t* tx_data, uint8_t* rx_data, uint32_t size);
static void SST25_SendCommand(SST25_Handle* handle, uint8_t cmd);
static void SST25_SendAddress(SST25_Handle* handle, uint32_t address);

// Software SPI helpers
static void SST25_SW_SPI_Delay(void);
static uint8_t SST25_SW_SPI_Transfer(SST25_Handle* handle, uint8_t data);

// =============================================================================
// PRIVATE FUNCTIONS - SPI LOW LEVEL
// =============================================================================

static void SST25_SW_SPI_Delay(void)
{
    volatile uint8_t i;
    for (i = 0; i < 2; i++) {
        __NOP();
    }
}

static void SST25_CS_Low(SST25_Handle* handle)
{
    if (handle->cs_callback) {
        handle->cs_callback(false);
    } else if (handle->cs_port) {
        HAL_GPIO_WritePin(handle->cs_port, handle->cs_pin, GPIO_PIN_RESET);
    }
}

static void SST25_CS_High(SST25_Handle* handle)
{
    if (handle->cs_callback) {
        handle->cs_callback(true);
    } else if (handle->cs_port) {
        HAL_GPIO_WritePin(handle->cs_port, handle->cs_pin, GPIO_PIN_SET);
    }
}

static uint8_t SST25_SW_SPI_Transfer(SST25_Handle* handle, uint8_t data)
{
    uint8_t received = 0;

    for (int i = 7; i >= 0; i--) {
        // Set MOSI
        if (data & (1 << i)) {
            HAL_GPIO_WritePin(handle->mosi_port, handle->mosi_pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(handle->mosi_port, handle->mosi_pin, GPIO_PIN_RESET);
        }

        SST25_SW_SPI_Delay();

        // Clock high
        HAL_GPIO_WritePin(handle->sclk_port, handle->sclk_pin, GPIO_PIN_SET);

        SST25_SW_SPI_Delay();

        // Read MISO
        if (HAL_GPIO_ReadPin(handle->miso_port, handle->miso_pin) == GPIO_PIN_SET) {
            received |= (1 << i);
        }

        // Clock low
        HAL_GPIO_WritePin(handle->sclk_port, handle->sclk_pin, GPIO_PIN_RESET);
    }

    return received;
}

static uint8_t SST25_SPI_Transfer(SST25_Handle* handle, uint8_t data)
{
#if SST25_ENABLE_SPI_MODE
    if (handle->use_hardware_spi && handle->hspi) {
        uint8_t received;
        HAL_SPI_TransmitReceive(handle->hspi, &data, &received, 1, SST25_TIMEOUT_DEFAULT);
        return received;
    }
#endif
    return SST25_SW_SPI_Transfer(handle, data);
}

static void SST25_SPI_TransferBuffer(SST25_Handle* handle, uint8_t* tx_data, uint8_t* rx_data, uint32_t size)
{
#if SST25_ENABLE_SPI_MODE
    if (handle->use_hardware_spi && handle->hspi) {
        if (tx_data && rx_data) {
            HAL_SPI_TransmitReceive(handle->hspi, tx_data, rx_data, size, SST25_TIMEOUT_DEFAULT);
        } else if (tx_data) {
            HAL_SPI_Transmit(handle->hspi, tx_data, size, SST25_TIMEOUT_DEFAULT);
        } else if (rx_data) {
            HAL_SPI_Receive(handle->hspi, rx_data, size, SST25_TIMEOUT_DEFAULT);
        }
        return;
    }
#endif
    for (uint32_t i = 0; i < size; i++) {
        uint8_t tx = tx_data ? tx_data[i] : 0xFF;
        uint8_t rx = SST25_SW_SPI_Transfer(handle, tx);
        if (rx_data) {
            rx_data[i] = rx;
        }
    }
}

static void SST25_SendCommand(SST25_Handle* handle, uint8_t cmd)
{
    SST25_SPI_Transfer(handle, cmd);
}

static void SST25_SendAddress(SST25_Handle* handle, uint32_t address)
{
    SST25_SPI_Transfer(handle, (address >> 16) & 0xFF);
    SST25_SPI_Transfer(handle, (address >> 8) & 0xFF);
    SST25_SPI_Transfer(handle, address & 0xFF);
}

// =============================================================================
// INITIALIZATION FUNCTIONS
// =============================================================================

SST25_Status SST25_Init(SST25_Handle* handle)
{
    if (!handle) {
        return SST25_ERROR_PARAM;
    }

    // Ensure CS is high initially
    SST25_CS_High(handle);
    HAL_Delay(1);

    // Read and verify JEDEC ID
    uint8_t mfr, type, cap;
    SST25_Status status = SST25_ReadJEDECID(handle, &mfr, &type, &cap);
    if (status != SST25_OK) {
        return status;
    }

    // Verify it's an SST25VF016B
    if (mfr != SST25_JEDEC_MFR_ID) {
        return SST25_ERROR_JEDEC_ID;
    }

    if (type != SST25_JEDEC_MEM_TYPE) {
        return SST25_ERROR_JEDEC_ID;
    }

    if (cap != SST25_JEDEC_CAPACITY) {
        return SST25_ERROR_JEDEC_ID;
    }

    // Store device info
    handle->manufacturer_id = mfr;
    handle->memory_type = type;
    handle->capacity_id = cap;

    // Disable block protection for full write access
    status = SST25_UnprotectAll(handle);
    if (status != SST25_OK) {
        return status;
    }

    handle->initialized = true;
    return SST25_OK;
}

SST25_Status SST25_ReadJEDECID(SST25_Handle* handle, uint8_t* manufacturer_id,
                               uint8_t* memory_type, uint8_t* capacity)
{
    if (!handle) {
        return SST25_ERROR_PARAM;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_JEDEC_ID);

    if (manufacturer_id) {
        *manufacturer_id = SST25_SPI_Transfer(handle, 0xFF);
    } else {
        SST25_SPI_Transfer(handle, 0xFF);
    }

    if (memory_type) {
        *memory_type = SST25_SPI_Transfer(handle, 0xFF);
    } else {
        SST25_SPI_Transfer(handle, 0xFF);
    }

    if (capacity) {
        *capacity = SST25_SPI_Transfer(handle, 0xFF);
    } else {
        SST25_SPI_Transfer(handle, 0xFF);
    }

    SST25_CS_High(handle);

    return SST25_OK;
}

// =============================================================================
// READ FUNCTIONS
// =============================================================================

SST25_Status SST25_Read(SST25_Handle* handle, uint32_t address, uint8_t* data, uint32_t size)
{
    if (!handle || !data || address + size > SST25_TOTAL_SIZE) {
        return SST25_ERROR_PARAM;
    }

    SST25_Status status = SST25_WaitReady(handle, SST25_TIMEOUT_DEFAULT);
    if (status != SST25_OK) {
        return status;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_READ);
    SST25_SendAddress(handle, address);

    SST25_SPI_TransferBuffer(handle, NULL, data, size);

    SST25_CS_High(handle);

    return SST25_OK;
}

SST25_Status SST25_HighSpeedRead(SST25_Handle* handle, uint32_t address, uint8_t* data, uint32_t size)
{
    if (!handle || !data || address + size > SST25_TOTAL_SIZE) {
        return SST25_ERROR_PARAM;
    }

    SST25_Status status = SST25_WaitReady(handle, SST25_TIMEOUT_DEFAULT);
    if (status != SST25_OK) {
        return status;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_HIGH_SPEED_READ);
    SST25_SendAddress(handle, address);
    SST25_SPI_Transfer(handle, 0xFF);  // Dummy byte

    SST25_SPI_TransferBuffer(handle, NULL, data, size);

    SST25_CS_High(handle);

    return SST25_OK;
}

// =============================================================================
// WRITE FUNCTIONS
// =============================================================================

SST25_Status SST25_ByteProgram(SST25_Handle* handle, uint32_t address, uint8_t data)
{
    if (!handle || address >= SST25_TOTAL_SIZE) {
        return SST25_ERROR_PARAM;
    }

    SST25_Status status = SST25_WaitReady(handle, SST25_TIMEOUT_DEFAULT);
    if (status != SST25_OK) {
        return status;
    }

    status = SST25_WriteEnable(handle);
    if (status != SST25_OK) {
        return status;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_BYTE_PROGRAM);
    SST25_SendAddress(handle, address);
    SST25_SPI_Transfer(handle, data);

    SST25_CS_High(handle);

    return SST25_WaitReady(handle, SST25_TIMEOUT_BYTE_PROG);
}

SST25_Status SST25_AAIProgram(SST25_Handle* handle, uint32_t address, const uint8_t* data, uint32_t size)
{
    if (!handle || !data || size == 0 || address + size > SST25_TOTAL_SIZE) {
        return SST25_ERROR_PARAM;
    }

    SST25_Status status;

    // Wait for device ready
    status = SST25_WaitReady(handle, SST25_TIMEOUT_DEFAULT);
    if (status != SST25_OK) {
        return status;
    }

    // Enable write
    status = SST25_WriteEnable(handle);
    if (status != SST25_OK) {
        return status;
    }

    uint32_t offset = 0;

    // Handle odd starting address with single byte program
    if (address & 0x01) {
        SST25_CS_Low(handle);
        SST25_SendCommand(handle, SST25_CMD_BYTE_PROGRAM);
        SST25_SendAddress(handle, address);
        SST25_SPI_Transfer(handle, data[0]);
        SST25_CS_High(handle);

        status = SST25_WaitReady(handle, SST25_TIMEOUT_BYTE_PROG);
        if (status != SST25_OK) {
            return status;
        }

        address++;
        offset++;

        // Re-enable write for AAI
        status = SST25_WriteEnable(handle);
        if (status != SST25_OK) {
            return status;
        }
    }

    // AAI Word Program for remaining even bytes (2 bytes at a time)
    if (offset < size && (size - offset) >= 2) {
        // First AAI command with address
        SST25_CS_Low(handle);
        SST25_SendCommand(handle, SST25_CMD_AAI_WORD_PROGRAM);
        SST25_SendAddress(handle, address);
        SST25_SPI_Transfer(handle, data[offset]);
        SST25_SPI_Transfer(handle, data[offset + 1]);
        SST25_CS_High(handle);

        status = SST25_WaitReady(handle, SST25_TIMEOUT_AAI_PROG);
        if (status != SST25_OK) {
            SST25_WriteDisable(handle);
            return status;
        }

        offset += 2;

        // Continue AAI without address (auto-increment)
        while (offset + 1 < size) {
            SST25_CS_Low(handle);
            SST25_SendCommand(handle, SST25_CMD_AAI_WORD_PROGRAM);
            SST25_SPI_Transfer(handle, data[offset]);
            SST25_SPI_Transfer(handle, data[offset + 1]);
            SST25_CS_High(handle);

            status = SST25_WaitReady(handle, SST25_TIMEOUT_AAI_PROG);
            if (status != SST25_OK) {
                SST25_WriteDisable(handle);
                return status;
            }

            offset += 2;
        }

        // Exit AAI mode with WRDI
        SST25_WriteDisable(handle);

        status = SST25_WaitReady(handle, SST25_TIMEOUT_DEFAULT);
        if (status != SST25_OK) {
            return status;
        }
    }

    // Handle remaining odd byte with single byte program
    if (offset < size) {
        status = SST25_WriteEnable(handle);
        if (status != SST25_OK) {
            return status;
        }

        SST25_CS_Low(handle);
        SST25_SendCommand(handle, SST25_CMD_BYTE_PROGRAM);
        SST25_SendAddress(handle, address + (offset - (address & 0x01 ? 1 : 0)));
        SST25_SPI_Transfer(handle, data[offset]);
        SST25_CS_High(handle);

        status = SST25_WaitReady(handle, SST25_TIMEOUT_BYTE_PROG);
        if (status != SST25_OK) {
            return status;
        }
    }

    return SST25_OK;
}

SST25_Status SST25_Write(SST25_Handle* handle, uint32_t address, const uint8_t* data, uint32_t size)
{
    // SST25 uses AAI programming instead of page program
    return SST25_AAIProgram(handle, address, data, size);
}

// =============================================================================
// ERASE FUNCTIONS
// =============================================================================

SST25_Status SST25_EraseSector(SST25_Handle* handle, uint32_t sector_num)
{
    if (!handle || sector_num >= SST25_SECTOR_COUNT) {
        return SST25_ERROR_PARAM;
    }

    uint32_t address = sector_num * SST25_SECTOR_SIZE;

    SST25_Status status = SST25_WaitReady(handle, SST25_TIMEOUT_DEFAULT);
    if (status != SST25_OK) {
        return status;
    }

    status = SST25_WriteEnable(handle);
    if (status != SST25_OK) {
        return status;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_SECTOR_ERASE);
    SST25_SendAddress(handle, address);

    SST25_CS_High(handle);

    return SST25_WaitReady(handle, SST25_TIMEOUT_SECTOR);
}

SST25_Status SST25_EraseBlock32K(SST25_Handle* handle, uint32_t block_num)
{
    if (!handle || block_num >= SST25_BLOCK_COUNT_32K) {
        return SST25_ERROR_PARAM;
    }

    uint32_t address = block_num * SST25_BLOCK_SIZE_32K;

    SST25_Status status = SST25_WaitReady(handle, SST25_TIMEOUT_DEFAULT);
    if (status != SST25_OK) {
        return status;
    }

    status = SST25_WriteEnable(handle);
    if (status != SST25_OK) {
        return status;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_BLOCK_ERASE_32K);
    SST25_SendAddress(handle, address);

    SST25_CS_High(handle);

    return SST25_WaitReady(handle, SST25_TIMEOUT_BLOCK_32K);
}

SST25_Status SST25_EraseBlock64K(SST25_Handle* handle, uint32_t block_num)
{
    if (!handle || block_num >= SST25_BLOCK_COUNT_64K) {
        return SST25_ERROR_PARAM;
    }

    uint32_t address = block_num * SST25_BLOCK_SIZE_64K;

    SST25_Status status = SST25_WaitReady(handle, SST25_TIMEOUT_DEFAULT);
    if (status != SST25_OK) {
        return status;
    }

    status = SST25_WriteEnable(handle);
    if (status != SST25_OK) {
        return status;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_BLOCK_ERASE_64K);
    SST25_SendAddress(handle, address);

    SST25_CS_High(handle);

    return SST25_WaitReady(handle, SST25_TIMEOUT_BLOCK_64K);
}

SST25_Status SST25_EraseChip(SST25_Handle* handle)
{
    if (!handle) {
        return SST25_ERROR_PARAM;
    }

    SST25_Status status = SST25_WaitReady(handle, SST25_TIMEOUT_DEFAULT);
    if (status != SST25_OK) {
        return status;
    }

    status = SST25_WriteEnable(handle);
    if (status != SST25_OK) {
        return status;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_CHIP_ERASE);

    SST25_CS_High(handle);

    return SST25_WaitReady(handle, SST25_TIMEOUT_CHIP);
}

// =============================================================================
// STATUS FUNCTIONS
// =============================================================================

SST25_Status SST25_ReadStatusReg(SST25_Handle* handle, uint8_t* status)
{
    if (!handle || !status) {
        return SST25_ERROR_PARAM;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_READ_STATUS);
    *status = SST25_SPI_Transfer(handle, 0xFF);

    SST25_CS_High(handle);

    return SST25_OK;
}

SST25_Status SST25_WriteStatusReg(SST25_Handle* handle, uint8_t status)
{
    if (!handle) {
        return SST25_ERROR_PARAM;
    }

    // First enable write to status register
    SST25_Status st = SST25_EnableWriteStatusReg(handle);
    if (st != SST25_OK) {
        return st;
    }

    SST25_CS_Low(handle);

    SST25_SendCommand(handle, SST25_CMD_WRITE_STATUS);
    SST25_SPI_Transfer(handle, status);

    SST25_CS_High(handle);

    return SST25_OK;
}

bool SST25_IsBusy(SST25_Handle* handle)
{
    uint8_t status;
    if (SST25_ReadStatusReg(handle, &status) != SST25_OK) {
        return true;
    }
    return (status & SST25_SR_BUSY) != 0;
}

SST25_Status SST25_WaitReady(SST25_Handle* handle, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();

    while (SST25_IsBusy(handle)) {
        if ((HAL_GetTick() - start) > timeout_ms) {
            return SST25_ERROR_TIMEOUT;
        }
    }

    return SST25_OK;
}

// =============================================================================
// WRITE PROTECTION
// =============================================================================

SST25_Status SST25_WriteEnable(SST25_Handle* handle)
{
    if (!handle) {
        return SST25_ERROR_PARAM;
    }

    SST25_CS_Low(handle);
    SST25_SendCommand(handle, SST25_CMD_WRITE_ENABLE);
    SST25_CS_High(handle);

    // Verify WEL bit is set
    uint8_t status;
    SST25_Status st = SST25_ReadStatusReg(handle, &status);
    if (st != SST25_OK) {
        return st;
    }

    if (!(status & SST25_SR_WEL)) {
        return SST25_ERROR_WRITE_PROTECT;
    }

    return SST25_OK;
}

SST25_Status SST25_WriteDisable(SST25_Handle* handle)
{
    if (!handle) {
        return SST25_ERROR_PARAM;
    }

    SST25_CS_Low(handle);
    SST25_SendCommand(handle, SST25_CMD_WRITE_DISABLE);
    SST25_CS_High(handle);

    return SST25_OK;
}

SST25_Status SST25_EnableWriteStatusReg(SST25_Handle* handle)
{
    if (!handle) {
        return SST25_ERROR_PARAM;
    }

    SST25_CS_Low(handle);
    SST25_SendCommand(handle, SST25_CMD_EWSR);
    SST25_CS_High(handle);

    return SST25_OK;
}

SST25_Status SST25_UnprotectAll(SST25_Handle* handle)
{
    if (!handle) {
        return SST25_ERROR_PARAM;
    }

    // Clear all block protect bits (BP0-BP3 = 0, BPL = 0)
    return SST25_WriteStatusReg(handle, 0x00);
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

uint32_t SST25_GetSectorNum(uint32_t address)
{
    return address / SST25_SECTOR_SIZE;
}

uint32_t SST25_GetBlock32KNum(uint32_t address)
{
    return address / SST25_BLOCK_SIZE_32K;
}

uint32_t SST25_GetBlock64KNum(uint32_t address)
{
    return address / SST25_BLOCK_SIZE_64K;
}
