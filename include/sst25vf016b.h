/*
 * sst25vf016b.h
 * SST25VF016B 16M-bit (2MB) SPI Flash Driver
 * Microchip (SST) Serial Flash Memory
 *
 * Note: Uses AAI (Auto Address Increment) programming
 */

#ifndef SST25VF016B_H
#define SST25VF016B_H

#include <stdint.h>
#include <stdbool.h>
#pragma once
#include <stm32f1xx_hal.h>

// =============================================================================
// FEATURE CONFIGURATION
// =============================================================================

#define SST25_ENABLE_SPI_MODE    1

#if SST25_ENABLE_SPI_MODE
//typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
#endif

// =============================================================================
// SST25VF016B SPECIFICATIONS
// =============================================================================

#define SST25_PAGE_SIZE          256         // 256 bytes per page
#define SST25_SECTOR_SIZE        4096        // 4KB per sector
#define SST25_BLOCK_SIZE_32K     32768       // 32KB block
#define SST25_BLOCK_SIZE_64K     65536       // 64KB block
#define SST25_TOTAL_SIZE         (2 * 1024 * 1024)  // 2MB total (16Mbit)

#define SST25_PAGE_COUNT         8192        // 8,192 pages
#define SST25_SECTOR_COUNT       512         // 512 sectors
#define SST25_BLOCK_COUNT_32K    64          // 64 x 32KB blocks
#define SST25_BLOCK_COUNT_64K    32          // 32 x 64KB blocks

// JEDEC ID for SST25VF016B
#define SST25_JEDEC_MFR_ID       0xBF        // SST/Microchip
#define SST25_JEDEC_MEM_TYPE     0x25        // 25 series
#define SST25_JEDEC_CAPACITY     0x8E        // 16Mbit

// =============================================================================
// SST25 COMMAND SET
// =============================================================================

// Read Commands
#define SST25_CMD_READ              0x03    // Read Data (up to 25MHz)
#define SST25_CMD_HIGH_SPEED_READ   0x0B    // High-Speed Read (up to 80MHz)

// Program Commands
#define SST25_CMD_BYTE_PROGRAM      0x02    // Byte Program (single byte)
#define SST25_CMD_AAI_WORD_PROGRAM  0xAD    // AAI Word Program (2 bytes auto-increment)

// Erase Commands
#define SST25_CMD_SECTOR_ERASE      0x20    // Sector Erase (4KB)
#define SST25_CMD_BLOCK_ERASE_32K   0x52    // Block Erase (32KB)
#define SST25_CMD_BLOCK_ERASE_64K   0xD8    // Block Erase (64KB)
#define SST25_CMD_CHIP_ERASE        0x60    // Chip Erase (alt: 0xC7)
#define SST25_CMD_CHIP_ERASE_ALT    0xC7    // Chip Erase alternative

// Write Control Commands
#define SST25_CMD_WRITE_ENABLE      0x06    // Write Enable (WREN)
#define SST25_CMD_WRITE_DISABLE     0x04    // Write Disable (WRDI)
#define SST25_CMD_EWSR              0x50    // Enable Write Status Register

// Status Register Commands
#define SST25_CMD_READ_STATUS       0x05    // Read Status Register (RDSR)
#define SST25_CMD_WRITE_STATUS      0x01    // Write Status Register (WRSR)

// ID Commands
#define SST25_CMD_JEDEC_ID          0x9F    // Read JEDEC ID
#define SST25_CMD_READ_ID           0x90    // Read Manufacturer/Device ID
#define SST25_CMD_READ_ID_ALT       0xAB    // Read ID (alternative)

// =============================================================================
// STATUS REGISTER BITS
// =============================================================================

#define SST25_SR_BUSY               0x01    // Write/Erase in Progress
#define SST25_SR_WEL                0x02    // Write Enable Latch
#define SST25_SR_BP0                0x04    // Block Protect Bit 0
#define SST25_SR_BP1                0x08    // Block Protect Bit 1
#define SST25_SR_BP2                0x10    // Block Protect Bit 2
#define SST25_SR_BP3                0x20    // Block Protect Bit 3
#define SST25_SR_AAI                0x40    // AAI Programming Mode
#define SST25_SR_BPL                0x80    // Block Protect Lock-Down

// =============================================================================
// ENUMERATIONS
// =============================================================================

typedef enum {
    SST25_OK = 0,
    SST25_ERROR_INIT,
    SST25_ERROR_SPI,
    SST25_ERROR_BUSY,
    SST25_ERROR_TIMEOUT,
    SST25_ERROR_JEDEC_ID,
    SST25_ERROR_PARAM,
    SST25_ERROR_WRITE_PROTECT,
    SST25_ERROR_ERASE
} SST25_Status;

// =============================================================================
// DATA STRUCTURES
// =============================================================================

// CS control callback function type
typedef void (*SST25_CS_Callback)(bool state);

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

    // CS callback for external control (e.g., via HC595)
    SST25_CS_Callback cs_callback;

    // Hardware SPI (if used)
#if SST25_ENABLE_SPI_MODE
    SPI_HandleTypeDef* hspi;
#else
    void* hspi;
#endif
    bool use_hardware_spi;

    // Device info (populated after init)
    uint8_t manufacturer_id;
    uint8_t memory_type;
    uint8_t capacity_id;

    // State
    bool initialized;

} SST25_Handle;

// =============================================================================
// COMPATIBILITY ALIASES (to use same API as W25Q)
// =============================================================================

// These aliases allow using SST25 with code written for W25Q
typedef SST25_Status W25Q_Status;
typedef SST25_Handle W25Q_Handle;
typedef SST25_CS_Callback W25Q_CS_Callback;

#define W25Q_OK                 SST25_OK
#define W25Q_ERROR_INIT         SST25_ERROR_INIT
#define W25Q_ERROR_SPI          SST25_ERROR_SPI
#define W25Q_ERROR_BUSY         SST25_ERROR_BUSY
#define W25Q_ERROR_TIMEOUT      SST25_ERROR_TIMEOUT
#define W25Q_ERROR_JEDEC_ID     SST25_ERROR_JEDEC_ID
#define W25Q_ERROR_PARAM        SST25_ERROR_PARAM
#define W25Q_ERROR_WRITE_PROTECT SST25_ERROR_WRITE_PROTECT
#define W25Q_ERROR_ERASE        SST25_ERROR_ERASE

#define W25Q_PAGE_SIZE          SST25_PAGE_SIZE
#define W25Q_SECTOR_SIZE        SST25_SECTOR_SIZE
#define W25Q_BLOCK_SIZE_32K     SST25_BLOCK_SIZE_32K
#define W25Q_BLOCK_SIZE_64K     SST25_BLOCK_SIZE_64K
#define W25Q_TOTAL_SIZE         SST25_TOTAL_SIZE
#define W25Q_SECTOR_COUNT       SST25_SECTOR_COUNT

// =============================================================================
// INITIALIZATION FUNCTIONS
// =============================================================================

/**
 * @brief Initialize SST25VF016B
 * @param handle: Pointer to SST25 handle (must be configured before calling)
 * @return SST25_Status
 */
SST25_Status SST25_Init(SST25_Handle* handle);

/**
 * @brief Read JEDEC ID
 * @param handle: Pointer to SST25 handle
 * @param manufacturer_id: Pointer to store manufacturer ID (0xBF)
 * @param memory_type: Pointer to store memory type (0x25)
 * @param capacity: Pointer to store capacity ID (0x41)
 * @return SST25_Status
 */
SST25_Status SST25_ReadJEDECID(SST25_Handle* handle, uint8_t* manufacturer_id,
                               uint8_t* memory_type, uint8_t* capacity);

// =============================================================================
// READ FUNCTIONS
// =============================================================================

/**
 * @brief Read data from flash
 * @param handle: Pointer to SST25 handle
 * @param address: Start address (24-bit)
 * @param data: Buffer to store read data
 * @param size: Number of bytes to read
 * @return SST25_Status
 */
SST25_Status SST25_Read(SST25_Handle* handle, uint32_t address, uint8_t* data, uint32_t size);

/**
 * @brief High-speed read data from flash (with dummy byte)
 * @param handle: Pointer to SST25 handle
 * @param address: Start address (24-bit)
 * @param data: Buffer to store read data
 * @param size: Number of bytes to read
 * @return SST25_Status
 */
SST25_Status SST25_HighSpeedRead(SST25_Handle* handle, uint32_t address, uint8_t* data, uint32_t size);

// =============================================================================
// WRITE FUNCTIONS
// =============================================================================

/**
 * @brief Write data to flash using AAI programming
 * @param handle: Pointer to SST25 handle
 * @param address: Start address (24-bit, should be even for optimal AAI)
 * @param data: Data to write
 * @param size: Number of bytes to write
 * @return SST25_Status
 * @note Uses AAI (Auto Address Increment) for fast programming
 */
SST25_Status SST25_Write(SST25_Handle* handle, uint32_t address, const uint8_t* data, uint32_t size);

/**
 * @brief Write a single byte
 * @param handle: Pointer to SST25 handle
 * @param address: Address (24-bit)
 * @param data: Byte to write
 * @return SST25_Status
 */
SST25_Status SST25_ByteProgram(SST25_Handle* handle, uint32_t address, uint8_t data);

/**
 * @brief Write using AAI Word Program (2 bytes at a time)
 * @param handle: Pointer to SST25 handle
 * @param address: Start address (24-bit)
 * @param data: Data to write
 * @param size: Number of bytes to write
 * @return SST25_Status
 * @note Address must be even, size will be rounded up to even
 */
SST25_Status SST25_AAIProgram(SST25_Handle* handle, uint32_t address, const uint8_t* data, uint32_t size);

// =============================================================================
// ERASE FUNCTIONS
// =============================================================================

/**
 * @brief Erase a 4KB sector
 * @param handle: Pointer to SST25 handle
 * @param sector_num: Sector number (0-511)
 * @return SST25_Status
 */
SST25_Status SST25_EraseSector(SST25_Handle* handle, uint32_t sector_num);

/**
 * @brief Erase a 32KB block
 * @param handle: Pointer to SST25 handle
 * @param block_num: Block number (0-63)
 * @return SST25_Status
 */
SST25_Status SST25_EraseBlock32K(SST25_Handle* handle, uint32_t block_num);

/**
 * @brief Erase a 64KB block
 * @param handle: Pointer to SST25 handle
 * @param block_num: Block number (0-31)
 * @return SST25_Status
 */
SST25_Status SST25_EraseBlock64K(SST25_Handle* handle, uint32_t block_num);

/**
 * @brief Erase entire chip
 * @param handle: Pointer to SST25 handle
 * @return SST25_Status
 * @note This operation takes about 50ms
 */
SST25_Status SST25_EraseChip(SST25_Handle* handle);

// =============================================================================
// STATUS FUNCTIONS
// =============================================================================

/**
 * @brief Read Status Register
 * @param handle: Pointer to SST25 handle
 * @param status: Pointer to store status value
 * @return SST25_Status
 */
SST25_Status SST25_ReadStatusReg(SST25_Handle* handle, uint8_t* status);

/**
 * @brief Write Status Register
 * @param handle: Pointer to SST25 handle
 * @param status: Status value to write
 * @return SST25_Status
 */
SST25_Status SST25_WriteStatusReg(SST25_Handle* handle, uint8_t status);

/**
 * @brief Check if device is busy
 * @param handle: Pointer to SST25 handle
 * @return true if busy, false if ready
 */
bool SST25_IsBusy(SST25_Handle* handle);

/**
 * @brief Wait until device is ready
 * @param handle: Pointer to SST25 handle
 * @param timeout_ms: Timeout in milliseconds
 * @return SST25_Status
 */
SST25_Status SST25_WaitReady(SST25_Handle* handle, uint32_t timeout_ms);

// =============================================================================
// WRITE PROTECTION
// =============================================================================

/**
 * @brief Enable write operations
 * @param handle: Pointer to SST25 handle
 * @return SST25_Status
 */
SST25_Status SST25_WriteEnable(SST25_Handle* handle);

/**
 * @brief Disable write operations
 * @param handle: Pointer to SST25 handle
 * @return SST25_Status
 */
SST25_Status SST25_WriteDisable(SST25_Handle* handle);

/**
 * @brief Enable Write Status Register
 * @param handle: Pointer to SST25 handle
 * @return SST25_Status
 * @note Must be called before WriteStatusReg
 */
SST25_Status SST25_EnableWriteStatusReg(SST25_Handle* handle);

/**
 * @brief Disable block protection (BP0-BP3 = 0)
 * @param handle: Pointer to SST25 handle
 * @return SST25_Status
 */
SST25_Status SST25_UnprotectAll(SST25_Handle* handle);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Get sector number from address
 * @param address: Flash address
 * @return Sector number (0-511)
 */
uint32_t SST25_GetSectorNum(uint32_t address);

/**
 * @brief Get 32KB block number from address
 * @param address: Flash address
 * @return Block number (0-63)
 */
uint32_t SST25_GetBlock32KNum(uint32_t address);

/**
 * @brief Get 64KB block number from address
 * @param address: Flash address
 * @return Block number (0-31)
 */
uint32_t SST25_GetBlock64KNum(uint32_t address);

// =============================================================================
// W25Q COMPATIBILITY WRAPPERS
// =============================================================================

// These functions provide W25Q-compatible API
#define W25Q_Init(h)                        SST25_Init(h)
#define W25Q_ReadJEDECID(h, m, t, c)        SST25_ReadJEDECID(h, m, t, c)
#define W25Q_Read(h, a, d, s)               SST25_Read(h, a, d, s)
#define W25Q_FastRead(h, a, d, s)           SST25_HighSpeedRead(h, a, d, s)
#define W25Q_Write(h, a, d, s)              SST25_Write(h, a, d, s)
#define W25Q_EraseSector(h, n)              SST25_EraseSector(h, n)
#define W25Q_EraseBlock32K(h, n)            SST25_EraseBlock32K(h, n)
#define W25Q_EraseBlock64K(h, n)            SST25_EraseBlock64K(h, n)
#define W25Q_EraseChip(h)                   SST25_EraseChip(h)
#define W25Q_ReadStatusReg1(h, s)           SST25_ReadStatusReg(h, s)
#define W25Q_ReadStatusReg2(h, s)           SST25_ReadStatusReg(h, s)
#define W25Q_ReadStatusReg3(h, s)           SST25_ReadStatusReg(h, s)
#define W25Q_WriteStatusReg1(h, s)          SST25_WriteStatusReg(h, s)
#define W25Q_IsBusy(h)                      SST25_IsBusy(h)
#define W25Q_WaitReady(h, t)                SST25_WaitReady(h, t)
#define W25Q_WriteEnable(h)                 SST25_WriteEnable(h)
#define W25Q_WriteDisable(h)                SST25_WriteDisable(h)
#define W25Q_GetSectorNum(a)                SST25_GetSectorNum(a)
#define W25Q_GetBlockNum(a)                 SST25_GetBlock64KNum(a)

#endif // SST25VF016B_H
