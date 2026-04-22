/*
 * ymodem.c
 * YMODEM Protocol Implementation for STM32
 * File transfer over UART with filename, size and CRC-16
 *
 * YMODEM Packet Format:
 *
 * Packet 0 (File Info):
 * +-----+--------+----------+----------------------------------+----------+
 * | SOH | 0x00   | 0xFF     | filename + NUL + filesize + NUL  | CRC16    |
 * | 1B  | 1B     | 1B       | 128B                             | 2B (H,L) |
 * +-----+--------+----------+----------------------------------+----------+
 *
 * Data Packets (1-N):
 * +-----+--------+----------+------------------+----------+
 * | STX | PktNum | ~PktNum  | Data (1024 bytes)| CRC16    |
 * | 1B  | 1B     | 1B       | 1024B            | 2B (H,L) |
 * +-----+--------+----------+------------------+----------+
 *
 * End Packet (empty filename):
 * +-----+--------+----------+------------------+----------+
 * | SOH | 0x00   | 0xFF     | NUL (128 bytes)  | CRC16    |
 * +-----+--------+----------+------------------+----------+
 */

#include <ymodem.h>
#include <string.h>
#include <stdlib.h>

// =============================================================================
// CRC-16 CALCULATION (XMODEM/YMODEM polynomial: 0x1021)
// =============================================================================

uint16_t YMODEM_CalculateCRC16(const uint8_t* data, uint32_t size)
{
    uint16_t crc = 0x0000;

    for (uint32_t i = 0; i < size; i++) {
        crc ^= ((uint16_t)data[i] << 8);

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

static YMODEM_Status YMODEM_ReceiveByte(YMODEM_Handle* handle, uint8_t* byte, uint32_t timeout_ms)
{
    HAL_StatusTypeDef status = HAL_UART_Receive(handle->huart, byte, 1, timeout_ms);

    if (status == HAL_OK) {
        return YMODEM_OK;
    } else if (status == HAL_TIMEOUT) {
        return YMODEM_ERROR_TIMEOUT;
    }

    return YMODEM_ERROR_PARAM;
}

static void YMODEM_SendByte(YMODEM_Handle* handle, uint8_t byte)
{
    HAL_UART_Transmit(handle->huart, &byte, 1, 100);
}

static void YMODEM_SendCAN(YMODEM_Handle* handle)
{
    // Send 3 CAN characters to ensure cancellation
    YMODEM_SendByte(handle, YMODEM_CAN);
    YMODEM_SendByte(handle, YMODEM_CAN);
    YMODEM_SendByte(handle, YMODEM_CAN);
}

static bool YMODEM_VerifyPacketCRC(YMODEM_Handle* handle, uint16_t data_size)
{
    // Calculate CRC of data portion
    uint16_t calculated_crc = YMODEM_CalculateCRC16(&handle->packet_buffer[3], data_size);

    // Get received CRC (high byte first)
    uint16_t received_crc = ((uint16_t)handle->packet_buffer[3 + data_size] << 8) |
                            handle->packet_buffer[3 + data_size + 1];

    return (calculated_crc == received_crc);
}

static bool YMODEM_ParsePacket0(YMODEM_Handle* handle)
{
    // Packet 0 contains: filename + NUL + filesize_string + NUL + ...
    // Data starts at packet_buffer[3]

    const char* data = (const char*)&handle->packet_buffer[3];

    // Check for end of batch (empty filename)
    if (data[0] == '\0') {
        handle->file_info.valid = false;
        return true;  // Valid empty packet (end of batch)
    }

    // Extract filename
    size_t filename_len = strlen(data);
    if (filename_len >= YMODEM_MAX_FILENAME_LEN) {
        filename_len = YMODEM_MAX_FILENAME_LEN - 1;
    }
    strncpy(handle->file_info.filename, data, filename_len);
    handle->file_info.filename[filename_len] = '\0';

    // Extract file size (after filename + NUL)
    const char* size_str = data + filename_len + 1;
    handle->file_info.filesize = 0;

    // Parse file size (decimal or octal)
    if (size_str[0] != '\0') {
        handle->file_info.filesize = strtoul(size_str, NULL, 10);
    }

    handle->file_info.valid = true;
    return true;
}

static YMODEM_Status YMODEM_ReceivePacket(YMODEM_Handle* handle, uint8_t first_byte)
{
    YMODEM_Status status;
    uint16_t data_size;
    uint16_t total_size;

    // Determine packet size based on header
    if (first_byte == YMODEM_SOH) {
        data_size = YMODEM_PACKET_SIZE;  // 128 bytes
    } else if (first_byte == YMODEM_STX) {
        data_size = YMODEM_PACKET_1K_SIZE;  // 1024 bytes
    } else {
        return YMODEM_ERROR_INVALID_PACKET;
    }

    handle->packet_buffer[0] = first_byte;
    handle->current_packet_size = data_size;
    total_size = 3 + data_size + 2;  // header(3) + data + CRC(2)

    // Receive rest of packet
    for (uint16_t i = 1; i < total_size; i++) {
        status = YMODEM_ReceiveByte(handle, &handle->packet_buffer[i], YMODEM_TIMEOUT_MS);
        if (status != YMODEM_OK) {
            return status;
        }
    }

    // Verify packet number complement
    uint8_t packet_num = handle->packet_buffer[1];
    uint8_t packet_num_comp = handle->packet_buffer[2];

    if ((packet_num + packet_num_comp) != 0xFF) {
        return YMODEM_ERROR_SEQUENCE;
    }

    // Verify CRC
    if (!YMODEM_VerifyPacketCRC(handle, data_size)) {
        handle->crc_errors++;
        return YMODEM_ERROR_CRC;
    }

    return YMODEM_OK;
}

static YMODEM_Status YMODEM_WaitForPacket(YMODEM_Handle* handle, uint8_t* first_byte, uint32_t timeout_ms)
{
    return YMODEM_ReceiveByte(handle, first_byte, timeout_ms);
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

YMODEM_Status YMODEM_Init(YMODEM_Handle* handle, UART_HandleTypeDef* huart)
{
    if (!handle || !huart) {
        return YMODEM_ERROR_PARAM;
    }

    memset(handle, 0, sizeof(YMODEM_Handle));
    handle->huart = huart;
    handle->state = YMODEM_STATE_IDLE;
    handle->expected_packet_num = 0;

    return YMODEM_OK;
}

YMODEM_Status YMODEM_ConfigureFlash(YMODEM_Handle* handle,
                                     uint32_t start_address,
                                     uint32_t max_size,
                                     YMODEM_FlashWriteCallback write_cb,
                                     YMODEM_FlashEraseCallback erase_cb)
{
    if (!handle || !write_cb) {
        return YMODEM_ERROR_PARAM;
    }

    handle->flash_start_address = start_address;
    handle->flash_max_size = max_size;
    handle->flash_write = write_cb;
    handle->flash_erase = erase_cb;
    handle->current_address = start_address;

    return YMODEM_OK;
}

void YMODEM_SetProgressCallback(YMODEM_Handle* handle, YMODEM_ProgressCallback progress_cb)
{
    if (handle) {
        handle->progress = progress_cb;
    }
}

void YMODEM_SetFileStartCallback(YMODEM_Handle* handle, YMODEM_FileStartCallback file_start_cb)
{
    if (handle) {
        handle->file_start = file_start_cb;
    }
}

YMODEM_Status YMODEM_Receive(YMODEM_Handle* handle, YMODEM_FileInfo* file_info)
{
    if (!handle || !handle->flash_write) {
        return YMODEM_ERROR_PARAM;
    }

    if (handle->state != YMODEM_STATE_IDLE) {
        return YMODEM_ERROR_BUSY;
    }

    YMODEM_Status status;
    uint8_t byte;
    uint32_t retries = 0;
    bool packet0_received = false;

    // Reset state
    handle->state = YMODEM_STATE_WAIT_START;
    handle->expected_packet_num = 0;
    handle->bytes_received = 0;
    handle->packet_count = 0;
    handle->current_address = handle->flash_start_address;
    handle->retry_count = 0;
    handle->crc_errors = 0;
    memset(&handle->file_info, 0, sizeof(YMODEM_FileInfo));

    // Flush UART RX buffer before starting
    __HAL_UART_FLUSH_DRREGISTER(handle->huart);
    while (__HAL_UART_GET_FLAG(handle->huart, UART_FLAG_RXNE)) {
        (void)handle->huart->Instance->DR;
    }

    // Send 'C' to request CRC mode and wait for packet 0
    while (!packet0_received && retries < YMODEM_MAX_RETRIES) {
        YMODEM_SendByte(handle, YMODEM_CRC_CHAR);

        status = YMODEM_WaitForPacket(handle, &byte, YMODEM_NAK_TIMEOUT_MS);

        if (status == YMODEM_OK) {
            if (byte == YMODEM_SOH || byte == YMODEM_STX) {
                // Receive packet 0
                status = YMODEM_ReceivePacket(handle, byte);

                if (status == YMODEM_OK) {
                    // Check packet number is 0
                    if (handle->packet_buffer[1] != 0) {
                        YMODEM_SendByte(handle, YMODEM_NAK);
                        retries++;
                        continue;
                    }

                    // Parse file info
                    if (!YMODEM_ParsePacket0(handle)) {
                        YMODEM_SendCAN(handle);
                        handle->state = YMODEM_STATE_ERROR;
                        return YMODEM_ERROR_INVALID_PACKET;
                    }

                    // Check for end of batch (empty filename)
                    if (!handle->file_info.valid) {
                        YMODEM_SendByte(handle, YMODEM_ACK);
                        handle->state = YMODEM_STATE_COMPLETE;
                        if (file_info) {
                            memset(file_info, 0, sizeof(YMODEM_FileInfo));
                        }
                        return YMODEM_OK;
                    }

                    // Check file size
                    if (handle->file_info.filesize > handle->flash_max_size) {
                        YMODEM_SendCAN(handle);
                        handle->state = YMODEM_STATE_ERROR;
                        return YMODEM_ERROR_FILE_TOO_LARGE;
                    }

                    // Call file start callback if set
                    if (handle->file_start) {
                        if (!handle->file_start(handle->file_info.filename, handle->file_info.filesize)) {
                            YMODEM_SendCAN(handle);
                            handle->state = YMODEM_STATE_ERROR;
                            return YMODEM_ERROR_CANCEL;
                        }
                    }

                    // Erase flash if callback provided
                    if (handle->flash_erase) {
                        if (!handle->flash_erase(handle->flash_start_address, handle->file_info.filesize)) {
                            YMODEM_SendCAN(handle);
                            handle->state = YMODEM_STATE_ERROR;
                            return YMODEM_ERROR_FLASH;
                        }
                    }

                    // ACK packet 0 and send 'C' to start data transfer
                    YMODEM_SendByte(handle, YMODEM_ACK);
                    YMODEM_SendByte(handle, YMODEM_CRC_CHAR);

                    packet0_received = true;
                    handle->expected_packet_num = 1;
                } else {
                    YMODEM_SendByte(handle, YMODEM_NAK);
                    retries++;
                }
            } else if (byte == YMODEM_CAN) {
                // Check for double CAN
                status = YMODEM_ReceiveByte(handle, &byte, 100);
                if (status == YMODEM_OK && byte == YMODEM_CAN) {
                    handle->state = YMODEM_STATE_ERROR;
                    return YMODEM_ERROR_CANCEL;
                }
            }
        } else {
            retries++;
        }
    }

    if (!packet0_received) {
        handle->state = YMODEM_STATE_ERROR;
        return YMODEM_ERROR_NO_RESPONSE;
    }

    // Main receive loop for data packets
    handle->state = YMODEM_STATE_RECEIVING;
    retries = 0;

    while (handle->state == YMODEM_STATE_RECEIVING) {
        status = YMODEM_WaitForPacket(handle, &byte, YMODEM_TIMEOUT_MS);

        if (status == YMODEM_OK) {
            if (byte == YMODEM_SOH || byte == YMODEM_STX) {
                status = YMODEM_ReceivePacket(handle, byte);

                if (status == YMODEM_OK) {
                    uint8_t packet_num = handle->packet_buffer[1];

                    // Handle duplicate packet
                    if (packet_num == (uint8_t)(handle->expected_packet_num - 1)) {
                        YMODEM_SendByte(handle, YMODEM_ACK);
                        continue;
                    }

                    // Check sequence
                    if (packet_num != handle->expected_packet_num) {
                        YMODEM_SendByte(handle, YMODEM_NAK);
                        handle->retry_count++;
                        retries++;
                        if (retries >= YMODEM_MAX_RETRIES) {
                            YMODEM_SendCAN(handle);
                            handle->state = YMODEM_STATE_ERROR;
                            return YMODEM_ERROR_RETRIES;
                        }
                        continue;
                    }

                    // Calculate bytes to write (handle last packet padding)
                    uint32_t bytes_to_write = handle->current_packet_size;
                    uint32_t remaining = handle->file_info.filesize - handle->bytes_received;

                    if (bytes_to_write > remaining) {
                        bytes_to_write = remaining;
                    }

                    // Write to flash
                    if (bytes_to_write > 0) {
                        if (!handle->flash_write(handle->current_address,
                                                 &handle->packet_buffer[3],
                                                 bytes_to_write)) {
                            YMODEM_SendCAN(handle);
                            handle->state = YMODEM_STATE_ERROR;
                            return YMODEM_ERROR_FLASH;
                        }

                        handle->current_address += bytes_to_write;
                        handle->bytes_received += bytes_to_write;
                    }

                    handle->packet_count++;
                    handle->expected_packet_num++;

                    // Progress callback
                    if (handle->progress) {
                        handle->progress(handle->bytes_received,
                                        handle->file_info.filesize,
                                        handle->file_info.filename);
                    }

                    YMODEM_SendByte(handle, YMODEM_ACK);
                    retries = 0;

                } else if (status == YMODEM_ERROR_CRC || status == YMODEM_ERROR_SEQUENCE) {
                    YMODEM_SendByte(handle, YMODEM_NAK);
                    handle->retry_count++;
                    retries++;
                    if (retries >= YMODEM_MAX_RETRIES) {
                        YMODEM_SendCAN(handle);
                        handle->state = YMODEM_STATE_ERROR;
                        return YMODEM_ERROR_RETRIES;
                    }
                }

            } else if (byte == YMODEM_EOT) {
                // First EOT - NAK it per YMODEM spec
                YMODEM_SendByte(handle, YMODEM_NAK);

                // Wait for second EOT
                status = YMODEM_WaitForPacket(handle, &byte, YMODEM_TIMEOUT_MS);
                if (status == YMODEM_OK && byte == YMODEM_EOT) {
                    YMODEM_SendByte(handle, YMODEM_ACK);

                    // Send 'C' for next file (batch mode)
                    YMODEM_SendByte(handle, YMODEM_CRC_CHAR);

                    // Wait for final packet 0 (should be empty for end of batch)
                    status = YMODEM_WaitForPacket(handle, &byte, YMODEM_TIMEOUT_MS);
                    if (status == YMODEM_OK && (byte == YMODEM_SOH || byte == YMODEM_STX)) {
                        status = YMODEM_ReceivePacket(handle, byte);
                        if (status == YMODEM_OK) {
                            YMODEM_SendByte(handle, YMODEM_ACK);
                        }
                    }

                    handle->state = YMODEM_STATE_COMPLETE;
                } else {
                    // Single EOT accepted
                    YMODEM_SendByte(handle, YMODEM_ACK);
                    handle->state = YMODEM_STATE_COMPLETE;
                }

            } else if (byte == YMODEM_CAN) {
                status = YMODEM_ReceiveByte(handle, &byte, 100);
                if (status == YMODEM_OK && byte == YMODEM_CAN) {
                    handle->state = YMODEM_STATE_ERROR;
                    return YMODEM_ERROR_CANCEL;
                }
            }

        } else if (status == YMODEM_ERROR_TIMEOUT) {
            retries++;
            if (retries >= YMODEM_MAX_RETRIES) {
                YMODEM_SendCAN(handle);
                handle->state = YMODEM_STATE_ERROR;
                return YMODEM_ERROR_TIMEOUT;
            }
            YMODEM_SendByte(handle, YMODEM_NAK);
        }
    }

    // Return file info
    if (file_info) {
        memcpy(file_info, &handle->file_info, sizeof(YMODEM_FileInfo));
    }

    handle->state = YMODEM_STATE_IDLE;
    return YMODEM_OK;
}

void YMODEM_Cancel(YMODEM_Handle* handle)
{
    if (!handle) return;

    YMODEM_SendCAN(handle);
    handle->state = YMODEM_STATE_IDLE;
}

YMODEM_State YMODEM_GetState(YMODEM_Handle* handle)
{
    if (!handle) return YMODEM_STATE_IDLE;
    return handle->state;
}

uint32_t YMODEM_GetBytesReceived(YMODEM_Handle* handle)
{
    if (!handle) return 0;
    return handle->bytes_received;
}

const YMODEM_FileInfo* YMODEM_GetFileInfo(YMODEM_Handle* handle)
{
    if (!handle) return NULL;
    return &handle->file_info;
}

const char* YMODEM_GetStatusString(YMODEM_Status status)
{
    switch (status) {
        case YMODEM_OK:                 return "OK";
        case YMODEM_ERROR_TIMEOUT:      return "Timeout";
        case YMODEM_ERROR_RETRIES:      return "Max retries";
        case YMODEM_ERROR_CANCEL:       return "Cancelled";
        case YMODEM_ERROR_SEQUENCE:     return "Sequence error";
        case YMODEM_ERROR_CRC:          return "CRC error";
        case YMODEM_ERROR_FLASH:        return "Flash error";
        case YMODEM_ERROR_PARAM:        return "Invalid param";
        case YMODEM_ERROR_BUSY:         return "Busy";
        case YMODEM_ERROR_NO_RESPONSE:  return "No response";
        case YMODEM_ERROR_FILE_TOO_LARGE: return "File too large";
        case YMODEM_ERROR_INVALID_PACKET: return "Invalid packet";
        default:                        return "Unknown";
    }
}

// =============================================================================
// NON-BLOCKING FUNCTIONS
// =============================================================================

// Non-blocking byte receive (0ms timeout)
static YMODEM_Status YMODEM_ReceiveByteNonBlocking(YMODEM_Handle* handle, uint8_t* byte)
{
    // Check if data available in UART
    if (__HAL_UART_GET_FLAG(handle->huart, UART_FLAG_RXNE)) {
        *byte = (uint8_t)(handle->huart->Instance->DR & 0xFF);
        return YMODEM_OK;
    }
    return YMODEM_ERROR_TIMEOUT;
}

bool YMODEM_CheckStart(YMODEM_Handle* handle)
{
    if (!handle) return false;

    uint8_t byte;

    // Check if any byte available
    if (YMODEM_ReceiveByteNonBlocking(handle, &byte) == YMODEM_OK) {
        // YMODEM sender starts with SOH or STX after receiving 'C'
        if (byte == YMODEM_SOH || byte == YMODEM_STX) {
            // Store first byte and set state
            handle->packet_buffer[0] = byte;
            handle->state = YMODEM_STATE_WAIT_START;
            return true;
        }
    }

    return false;
}

void YMODEM_StartListen(YMODEM_Handle* handle)
{
    if (!handle) return;

    // Reset state
    handle->state = YMODEM_STATE_IDLE;
    handle->expected_packet_num = 0;
    handle->bytes_received = 0;
    handle->packet_count = 0;
    handle->current_address = handle->flash_start_address;
    handle->retry_count = 0;
    handle->crc_errors = 0;
    memset(&handle->file_info, 0, sizeof(YMODEM_FileInfo));

    // Send 'C' to request CRC mode transfer
    YMODEM_SendByte(handle, YMODEM_CRC_CHAR);
}

bool YMODEM_IsTransferActive(YMODEM_Handle* handle)
{
    if (!handle) return false;

    return (handle->state != YMODEM_STATE_IDLE &&
            handle->state != YMODEM_STATE_COMPLETE &&
            handle->state != YMODEM_STATE_ERROR);
}

YMODEM_Status YMODEM_Poll(YMODEM_Handle* handle)
{
    if (!handle) return YMODEM_ERROR_PARAM;

    // If idle, nothing to do
    if (handle->state == YMODEM_STATE_IDLE) {
        return YMODEM_OK;
    }

    // If complete or error, return status
    if (handle->state == YMODEM_STATE_COMPLETE) {
        handle->state = YMODEM_STATE_IDLE;
        return YMODEM_OK;
    }

    if (handle->state == YMODEM_STATE_ERROR) {
        handle->state = YMODEM_STATE_IDLE;
        return YMODEM_ERROR_CANCEL;
    }

    YMODEM_Status status;
    uint8_t byte;

    // Handle WAIT_START state (packet 0 detection)
    if (handle->state == YMODEM_STATE_WAIT_START) {
        // First byte already in buffer from CheckStart
        byte = handle->packet_buffer[0];

        // Receive rest of packet 0 (blocking for this packet)
        status = YMODEM_ReceivePacket(handle, byte);

        if (status == YMODEM_OK) {
            // Check packet number is 0
            if (handle->packet_buffer[1] != 0) {
                YMODEM_SendByte(handle, YMODEM_NAK);
                handle->state = YMODEM_STATE_IDLE;
                return YMODEM_ERROR_SEQUENCE;
            }

            // Parse file info
            if (!YMODEM_ParsePacket0(handle)) {
                YMODEM_SendCAN(handle);
                handle->state = YMODEM_STATE_ERROR;
                return YMODEM_ERROR_INVALID_PACKET;
            }

            // Check for end of batch (empty filename)
            if (!handle->file_info.valid) {
                YMODEM_SendByte(handle, YMODEM_ACK);
                handle->state = YMODEM_STATE_COMPLETE;
                return YMODEM_OK;
            }

            // Check file size
            if (handle->flash_max_size > 0 && handle->file_info.filesize > handle->flash_max_size) {
                YMODEM_SendCAN(handle);
                handle->state = YMODEM_STATE_ERROR;
                return YMODEM_ERROR_FILE_TOO_LARGE;
            }

            // Call file start callback if set
            if (handle->file_start) {
                if (!handle->file_start(handle->file_info.filename, handle->file_info.filesize)) {
                    YMODEM_SendCAN(handle);
                    handle->state = YMODEM_STATE_ERROR;
                    return YMODEM_ERROR_CANCEL;
                }
            }

            // Erase flash if callback provided
            if (handle->flash_erase) {
                if (!handle->flash_erase(handle->flash_start_address, handle->file_info.filesize)) {
                    YMODEM_SendCAN(handle);
                    handle->state = YMODEM_STATE_ERROR;
                    return YMODEM_ERROR_FLASH;
                }
            }

            // ACK packet 0 and send 'C' to start data transfer
            YMODEM_SendByte(handle, YMODEM_ACK);
            YMODEM_SendByte(handle, YMODEM_CRC_CHAR);

            handle->expected_packet_num = 1;
            handle->state = YMODEM_STATE_RECEIVING;

            return YMODEM_ERROR_BUSY;  // Still in progress
        } else {
            YMODEM_SendByte(handle, YMODEM_NAK);
            handle->retry_count++;
            if (handle->retry_count >= YMODEM_MAX_RETRIES) {
                handle->state = YMODEM_STATE_ERROR;
                return YMODEM_ERROR_RETRIES;
            }
            handle->state = YMODEM_STATE_IDLE;  // Wait for next start
            return YMODEM_ERROR_BUSY;
        }
    }

    // Handle RECEIVING state
    if (handle->state == YMODEM_STATE_RECEIVING) {
        // Wait for next packet (with timeout)
        status = YMODEM_WaitForPacket(handle, &byte, YMODEM_TIMEOUT_MS);

        if (status == YMODEM_OK) {
            if (byte == YMODEM_SOH || byte == YMODEM_STX) {
                status = YMODEM_ReceivePacket(handle, byte);

                if (status == YMODEM_OK) {
                    uint8_t packet_num = handle->packet_buffer[1];

                    // Handle duplicate packet
                    if (packet_num == (uint8_t)(handle->expected_packet_num - 1)) {
                        YMODEM_SendByte(handle, YMODEM_ACK);
                        return YMODEM_ERROR_BUSY;
                    }

                    // Check sequence
                    if (packet_num != handle->expected_packet_num) {
                        YMODEM_SendByte(handle, YMODEM_NAK);
                        handle->retry_count++;
                        if (handle->retry_count >= YMODEM_MAX_RETRIES) {
                            YMODEM_SendCAN(handle);
                            handle->state = YMODEM_STATE_ERROR;
                            return YMODEM_ERROR_RETRIES;
                        }
                        return YMODEM_ERROR_BUSY;
                    }

                    // Calculate bytes to write
                    uint32_t bytes_to_write = handle->current_packet_size;
                    if (handle->file_info.filesize > 0) {
                        uint32_t remaining = handle->file_info.filesize - handle->bytes_received;
                        if (bytes_to_write > remaining) {
                            bytes_to_write = remaining;
                        }
                    }

                    // Write to flash
                    if (bytes_to_write > 0 && handle->flash_write) {
                        if (!handle->flash_write(handle->current_address,
                                                 &handle->packet_buffer[3],
                                                 bytes_to_write)) {
                            YMODEM_SendCAN(handle);
                            handle->state = YMODEM_STATE_ERROR;
                            return YMODEM_ERROR_FLASH;
                        }

                        handle->current_address += bytes_to_write;
                        handle->bytes_received += bytes_to_write;
                    }

                    handle->packet_count++;
                    handle->expected_packet_num++;

                    // Progress callback
                    if (handle->progress) {
                        handle->progress(handle->bytes_received,
                                        handle->file_info.filesize,
                                        handle->file_info.filename);
                    }

                    YMODEM_SendByte(handle, YMODEM_ACK);
                    handle->retry_count = 0;

                    return YMODEM_ERROR_BUSY;  // More packets expected

                } else {
                    YMODEM_SendByte(handle, YMODEM_NAK);
                    handle->retry_count++;
                    if (handle->retry_count >= YMODEM_MAX_RETRIES) {
                        YMODEM_SendCAN(handle);
                        handle->state = YMODEM_STATE_ERROR;
                        return YMODEM_ERROR_RETRIES;
                    }
                    return YMODEM_ERROR_BUSY;
                }

            } else if (byte == YMODEM_EOT) {
                // First EOT - NAK it per YMODEM spec
                YMODEM_SendByte(handle, YMODEM_NAK);

                // Wait for second EOT
                status = YMODEM_WaitForPacket(handle, &byte, YMODEM_TIMEOUT_MS);
                if (status == YMODEM_OK && byte == YMODEM_EOT) {
                    YMODEM_SendByte(handle, YMODEM_ACK);

                    // Send 'C' for batch mode end
                    YMODEM_SendByte(handle, YMODEM_CRC_CHAR);

                    // Wait for final empty packet 0
                    status = YMODEM_WaitForPacket(handle, &byte, YMODEM_TIMEOUT_MS);
                    if (status == YMODEM_OK && (byte == YMODEM_SOH || byte == YMODEM_STX)) {
                        YMODEM_ReceivePacket(handle, byte);
                        YMODEM_SendByte(handle, YMODEM_ACK);
                    }
                } else {
                    YMODEM_SendByte(handle, YMODEM_ACK);
                }

                handle->state = YMODEM_STATE_COMPLETE;
                return YMODEM_OK;  // Transfer complete!

            } else if (byte == YMODEM_CAN) {
                status = YMODEM_ReceiveByte(handle, &byte, 100);
                if (status == YMODEM_OK && byte == YMODEM_CAN) {
                    handle->state = YMODEM_STATE_ERROR;
                    return YMODEM_ERROR_CANCEL;
                }
            }

        } else if (status == YMODEM_ERROR_TIMEOUT) {
            handle->retry_count++;
            if (handle->retry_count >= YMODEM_MAX_RETRIES) {
                YMODEM_SendCAN(handle);
                handle->state = YMODEM_STATE_ERROR;
                return YMODEM_ERROR_TIMEOUT;
            }
            YMODEM_SendByte(handle, YMODEM_NAK);
            return YMODEM_ERROR_BUSY;
        }
    }

    return YMODEM_ERROR_BUSY;
}
