/*
 * ymodem_flash.h
 * YMODEM to W25Q Flash Integration
 * Automatic filename extraction and Flash File System support
 */

#ifndef YMODEM_FLASH_H
#define YMODEM_FLASH_H

#include <stdint.h>
#include <stdbool.h>
#include "ymodem.h"
#include "sst25vf016b.h"
#include "flash_fs.h"

// =============================================================================
// YMODEM FLASH HANDLE
// =============================================================================

typedef struct {
    YMODEM_Handle ymodem;           // YMODEM protocol handle
    W25Q_Handle* flash;             // W25Q Flash handle
    FFS_Handle* ffs;                // Flash File System handle (optional)
    bool use_ffs;                   // Use FFS for file management
} YMODEM_Flash_Handle;

// =============================================================================
// RESULT STRUCTURE
// =============================================================================

typedef struct {
    char filename[YMODEM_MAX_FILENAME_LEN];  // Received filename
    uint32_t filesize;                        // File size
    uint32_t bytes_received;                  // Actual bytes written
    bool success;                             // Transfer successful
    YMODEM_Status status;                     // Status code
} YMODEM_Flash_Result;

// =============================================================================
// PUBLIC FUNCTIONS - INITIALIZATION
// =============================================================================

/**
 * @brief Initialize YMODEM Flash transfer system (without FFS)
 * @param handle: Pointer to YMODEM_Flash_Handle
 * @param huart: UART handle for YMODEM communication
 * @param flash: W25Q Flash handle (must be initialized)
 * @return YMODEM_Status
 */
YMODEM_Status YMODEM_Flash_Init(YMODEM_Flash_Handle* handle,
                                 UART_HandleTypeDef* huart,
                                 W25Q_Handle* flash);

/**
 * @brief Initialize YMODEM Flash transfer system with FFS
 * @param handle: Pointer to YMODEM_Flash_Handle
 * @param huart: UART handle for YMODEM communication
 * @param flash: W25Q Flash handle (must be initialized)
 * @param ffs: Flash File System handle (must be initialized)
 * @return YMODEM_Status
 */
YMODEM_Status YMODEM_Flash_InitWithFFS(YMODEM_Flash_Handle* handle,
                                        UART_HandleTypeDef* huart,
                                        W25Q_Handle* flash,
                                        FFS_Handle* ffs);

// =============================================================================
// PUBLIC FUNCTIONS - FILE TRANSFER
// =============================================================================

/**
 * @brief Receive file via YMODEM and store to flash (simple mode)
 * @param handle: Pointer to YMODEM_Flash_Handle
 * @param flash_address: Start address in flash
 * @param max_size: Maximum file size allowed
 * @param result: Pointer to store transfer result (can be NULL)
 * @return YMODEM_Status
 *
 * Note: Filename is extracted from YMODEM packet 0
 */
YMODEM_Status YMODEM_Flash_Receive(YMODEM_Flash_Handle* handle,
                                    uint32_t flash_address,
                                    uint32_t max_size,
                                    YMODEM_Flash_Result* result);

/**
 * @brief Receive file via YMODEM and store using FFS
 * @param handle: Pointer to YMODEM_Flash_Handle (must be initialized with FFS)
 * @param max_size: Maximum file size allowed
 * @param file_type: File type for FFS
 * @param result: Pointer to store transfer result (can be NULL)
 * @return YMODEM_Status
 *
 * Note: Filename is automatically extracted from YMODEM transfer
 *       File is created in FFS with the received filename
 */
YMODEM_Status YMODEM_Flash_ReceiveToFFS(YMODEM_Flash_Handle* handle,
                                         uint32_t max_size,
                                         FFS_FileType file_type,
                                         YMODEM_Flash_Result* result);

// =============================================================================
// PUBLIC FUNCTIONS - FILE OPERATIONS
// =============================================================================

/**
 * @brief Get last received file info
 * @param handle: Pointer to YMODEM_Flash_Handle
 * @return Pointer to file info or NULL
 */
const YMODEM_FileInfo* YMODEM_Flash_GetFileInfo(YMODEM_Flash_Handle* handle);

/**
 * @brief Cancel ongoing transfer
 * @param handle: Pointer to YMODEM_Flash_Handle
 */
void YMODEM_Flash_Cancel(YMODEM_Flash_Handle* handle);

// =============================================================================
// PUBLIC FUNCTIONS - UTILITY
// =============================================================================

/**
 * @brief Calculate CRC32 of data
 * @param data: Data buffer
 * @param size: Data size
 * @return CRC32 value
 */
uint32_t YMODEM_Flash_CalculateCRC32(const uint8_t* data, uint32_t size);

// =============================================================================
// NON-BLOCKING (POLLING) FUNCTIONS - For main loop integration
// =============================================================================

/**
 * @brief Poll for incoming YMODEM transfer (non-blocking)
 * @param handle: Pointer to YMODEM_Flash_Handle
 * @return YMODEM_Status:
 *         - YMODEM_OK: Transfer complete (check result)
 *         - YMODEM_ERROR_BUSY: Transfer in progress or waiting
 *         - Other: Error occurred
 *
 * Usage in main loop:
 *   YMODEM_Flash_Result result;
 *   YMODEM_Status status = YMODEM_Flash_Poll(&ymodem_flash, &result);
 *   if (status == YMODEM_OK && result.success) {
 *       printf("Received: %s (%lu bytes)\n", result.filename, result.bytes_received);
 *   }
 */
YMODEM_Status YMODEM_Flash_Poll(YMODEM_Flash_Handle* handle, YMODEM_Flash_Result* result);

/**
 * @brief Start listening for YMODEM transfer
 * @param handle: Pointer to YMODEM_Flash_Handle
 * @param max_size: Maximum file size allowed
 * @param file_type: File type for FFS (if using FFS)
 *
 * Call this once at startup or after a transfer completes
 */
void YMODEM_Flash_StartListen(YMODEM_Flash_Handle* handle, uint32_t max_size, FFS_FileType file_type);

/**
 * @brief Check if a transfer is currently active
 * @param handle: Pointer to YMODEM_Flash_Handle
 * @return true if transfer in progress
 */
bool YMODEM_Flash_IsTransferActive(YMODEM_Flash_Handle* handle);

/**
 * @brief Send 'C' character to request CRC mode (call periodically when idle)
 * @param handle: Pointer to YMODEM_Flash_Handle
 * @param interval_ms: Minimum interval between 'C' sends (e.g., 3000 for 3 seconds)
 *
 * This tells the sender that receiver is ready for YMODEM-CRC transfer
 */
void YMODEM_Flash_SendReadySignal(YMODEM_Flash_Handle* handle, uint32_t interval_ms);

#endif // YMODEM_FLASH_H
