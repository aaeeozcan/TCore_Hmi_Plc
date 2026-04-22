/*
 * ymodem.h
 * YMODEM Protocol Implementation for STM32
 * File transfer over UART with filename, size and CRC-16
 *
 * YMODEM advantages over XMODEM:
 * - Filename transmitted in packet 0
 * - File size transmitted in packet 0
 * - 1024 byte packets (faster transfer)
 * - Batch file transfer support
 */

#ifndef YMODEM_H
#define YMODEM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"

// =============================================================================
// YMODEM PROTOCOL CONSTANTS
// =============================================================================

#define YMODEM_SOH          0x01    // Start of Header (128 byte packet)
#define YMODEM_STX          0x02    // Start of Header (1024 byte packet)
#define YMODEM_EOT          0x04    // End of Transmission
#define YMODEM_ACK          0x06    // Acknowledge
#define YMODEM_NAK          0x15    // Not Acknowledge
#define YMODEM_CAN          0x18    // Cancel
#define YMODEM_CRC_CHAR     'C'     // CRC mode request character

#define YMODEM_PACKET_SIZE      128     // Small packet data size
#define YMODEM_PACKET_1K_SIZE   1024    // 1K packet data size
#define YMODEM_MAX_RETRIES      10      // Maximum retry count
#define YMODEM_TIMEOUT_MS       1000    // Receive timeout (1 second)
#define YMODEM_NAK_TIMEOUT_MS   1000    // Initial NAK timeout (3 seconds - sends 'C' every 3s)

#define YMODEM_MAX_FILENAME_LEN 64      // Max filename length
#define YMODEM_MAX_FILESIZE_LEN 16      // Max file size string length

// =============================================================================
// YMODEM STATUS CODES
// =============================================================================

typedef enum {
    YMODEM_OK = 0,              // Transfer successful
    YMODEM_ERROR_TIMEOUT,       // Receive timeout
    YMODEM_ERROR_RETRIES,       // Max retries exceeded
    YMODEM_ERROR_CANCEL,        // Transfer cancelled by sender
    YMODEM_ERROR_SEQUENCE,      // Packet sequence error
    YMODEM_ERROR_CRC,           // CRC check failed
    YMODEM_ERROR_FLASH,         // Flash write error
    YMODEM_ERROR_PARAM,         // Invalid parameter
    YMODEM_ERROR_BUSY,          // Transfer already in progress
    YMODEM_ERROR_NO_RESPONSE,   // No response from sender
    YMODEM_ERROR_FILE_TOO_LARGE,// File exceeds max size
    YMODEM_ERROR_INVALID_PACKET // Invalid packet format
} YMODEM_Status;

// =============================================================================
// YMODEM TRANSFER STATE
// =============================================================================

typedef enum {
    YMODEM_STATE_IDLE = 0,      // No transfer in progress
    YMODEM_STATE_WAIT_START,    // Waiting for packet 0 (file info)
    YMODEM_STATE_RECEIVING,     // Receiving data packets
    YMODEM_STATE_WAIT_EOT,      // Waiting for end of transmission
    YMODEM_STATE_COMPLETE,      // Transfer complete
    YMODEM_STATE_ERROR          // Transfer error
} YMODEM_State;

// =============================================================================
// CALLBACK FUNCTION TYPES
// =============================================================================

// Flash write callback: address, data, size -> success
typedef bool (*YMODEM_FlashWriteCallback)(uint32_t address, const uint8_t* data, uint32_t size);

// Flash erase callback: start_address, size -> success
typedef bool (*YMODEM_FlashEraseCallback)(uint32_t address, uint32_t size);

// Progress callback: bytes_received, total_size, filename
typedef void (*YMODEM_ProgressCallback)(uint32_t bytes_received, uint32_t total_size, const char* filename);

// File start callback: filename, filesize -> accept (true) or reject (false)
typedef bool (*YMODEM_FileStartCallback)(const char* filename, uint32_t filesize);

// =============================================================================
// FILE INFO STRUCTURE
// =============================================================================

typedef struct {
    char filename[YMODEM_MAX_FILENAME_LEN];  // Received filename
    uint32_t filesize;                        // Received file size
    bool valid;                               // Info is valid
} YMODEM_FileInfo;

// =============================================================================
// YMODEM HANDLE STRUCTURE
// =============================================================================

typedef struct {
    // UART handle
    UART_HandleTypeDef* huart;

    // Flash callbacks
    YMODEM_FlashWriteCallback flash_write;
    YMODEM_FlashEraseCallback flash_erase;
    YMODEM_ProgressCallback progress;
    YMODEM_FileStartCallback file_start;

    // Flash configuration
    uint32_t flash_start_address;   // Start address in flash
    uint32_t flash_max_size;        // Maximum file size

    // Transfer state
    YMODEM_State state;
    uint8_t expected_packet_num;    // Expected packet sequence number
    uint32_t bytes_received;        // Total bytes received
    uint32_t packet_count;          // Total packets received
    uint32_t current_address;       // Current flash write address

    // File info from packet 0
    YMODEM_FileInfo file_info;

    // Packet buffer (1024 + header + CRC)
    uint8_t packet_buffer[YMODEM_PACKET_1K_SIZE + 5];
    uint16_t current_packet_size;   // 128 or 1024

    // Statistics
    uint32_t retry_count;           // Total retries
    uint32_t crc_errors;            // CRC error count

} YMODEM_Handle;

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize YMODEM handle
 * @param handle: Pointer to YMODEM handle
 * @param huart: UART handle for communication
 * @return YMODEM_Status
 */
YMODEM_Status YMODEM_Init(YMODEM_Handle* handle, UART_HandleTypeDef* huart);

/**
 * @brief Configure flash parameters
 * @param handle: Pointer to YMODEM handle
 * @param start_address: Flash start address for file storage
 * @param max_size: Maximum file size allowed
 * @param write_cb: Flash write callback function
 * @param erase_cb: Flash erase callback function (can be NULL)
 * @return YMODEM_Status
 */
YMODEM_Status YMODEM_ConfigureFlash(YMODEM_Handle* handle,
                                     uint32_t start_address,
                                     uint32_t max_size,
                                     YMODEM_FlashWriteCallback write_cb,
                                     YMODEM_FlashEraseCallback erase_cb);

/**
 * @brief Set progress callback
 * @param handle: Pointer to YMODEM handle
 * @param progress_cb: Progress callback function
 */
void YMODEM_SetProgressCallback(YMODEM_Handle* handle, YMODEM_ProgressCallback progress_cb);

/**
 * @brief Set file start callback (called when filename/size received)
 * @param handle: Pointer to YMODEM handle
 * @param file_start_cb: File start callback function
 */
void YMODEM_SetFileStartCallback(YMODEM_Handle* handle, YMODEM_FileStartCallback file_start_cb);

/**
 * @brief Start YMODEM receive (blocking)
 * @param handle: Pointer to YMODEM handle
 * @param file_info: Pointer to store received file info (can be NULL)
 * @return YMODEM_Status
 * @note This function blocks until transfer is complete or error occurs
 */
YMODEM_Status YMODEM_Receive(YMODEM_Handle* handle, YMODEM_FileInfo* file_info);

/**
 * @brief Cancel ongoing transfer
 * @param handle: Pointer to YMODEM handle
 */
void YMODEM_Cancel(YMODEM_Handle* handle);

/**
 * @brief Get current transfer state
 * @param handle: Pointer to YMODEM handle
 * @return Current YMODEM_State
 */
YMODEM_State YMODEM_GetState(YMODEM_Handle* handle);

/**
 * @brief Get bytes received so far
 * @param handle: Pointer to YMODEM handle
 * @return Bytes received
 */
uint32_t YMODEM_GetBytesReceived(YMODEM_Handle* handle);

/**
 * @brief Get received file info
 * @param handle: Pointer to YMODEM handle
 * @return Pointer to file info
 */
const YMODEM_FileInfo* YMODEM_GetFileInfo(YMODEM_Handle* handle);

/**
 * @brief Calculate CRC-16 (XMODEM/YMODEM polynomial)
 * @param data: Data buffer
 * @param size: Data size
 * @return CRC-16 value
 */
uint16_t YMODEM_CalculateCRC16(const uint8_t* data, uint32_t size);

/**
 * @brief Get status string for debugging
 * @param status: YMODEM_Status code
 * @return Status string
 */
const char* YMODEM_GetStatusString(YMODEM_Status status);

// =============================================================================
// NON-BLOCKING FUNCTIONS (for main loop polling)
// =============================================================================

/**
 * @brief Check if YMODEM start sequence detected (non-blocking)
 * @param handle: Pointer to YMODEM handle
 * @return true if SOH/STX received (transfer starting), false otherwise
 * @note Call this in main loop to detect incoming transfer
 */
bool YMODEM_CheckStart(YMODEM_Handle* handle);

/**
 * @brief Poll YMODEM transfer (non-blocking state machine)
 * @param handle: Pointer to YMODEM handle
 * @return YMODEM_Status (YMODEM_OK when complete, YMODEM_ERROR_BUSY while in progress)
 * @note Call this repeatedly in main loop after YMODEM_CheckStart returns true
 */
YMODEM_Status YMODEM_Poll(YMODEM_Handle* handle);

/**
 * @brief Start listening for YMODEM transfer
 * @param handle: Pointer to YMODEM handle
 * @note Sends 'C' character to request CRC mode and waits for sender
 */
void YMODEM_StartListen(YMODEM_Handle* handle);

/**
 * @brief Check if transfer is in progress
 * @param handle: Pointer to YMODEM handle
 * @return true if transfer in progress
 */
bool YMODEM_IsTransferActive(YMODEM_Handle* handle);

#endif // YMODEM_H
