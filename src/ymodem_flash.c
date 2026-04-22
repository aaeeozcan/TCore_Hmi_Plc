/*
 * ymodem_flash.c
 * YMODEM to W25Q Flash Integration
 * Automatic filename extraction and Flash File System support
 */

#include <ymodem_flash.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static YMODEM_Flash_Handle* g_current_handle = NULL;
static uint32_t g_write_address = 0;
__attribute__((unused)) static uint32_t g_max_size = 0;

// For FFS mode
static char g_pending_filename[YMODEM_MAX_FILENAME_LEN];
__attribute__((unused)) static uint32_t g_pending_filesize = 0;
static FFS_FileType g_pending_filetype = FFS_TYPE_UNKNOWN;

// =============================================================================
// CRC32 TABLE (IEEE 802.3)
// =============================================================================

static const uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD706B3,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

uint32_t YMODEM_Flash_CalculateCRC32(const uint8_t* data, uint32_t size)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < size; i++) {
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFF;
}

// =============================================================================
// FLASH CALLBACK FUNCTIONS - SIMPLE MODE
// =============================================================================

static bool flash_write_callback(uint32_t address, const uint8_t* data, uint32_t size)
{
    if (!g_current_handle || !g_current_handle->flash) {
        return false;
    }

    W25Q_Status status = W25Q_Write(g_current_handle->flash, address, data, size);
    return (status == W25Q_OK);
}

static bool flash_erase_callback(uint32_t address, uint32_t size)
{
    if (!g_current_handle || !g_current_handle->flash) {
        return false;
    }

    // Calculate sectors to erase
    uint32_t start_sector = address / W25Q_SECTOR_SIZE;
    uint32_t end_address = address + size;
    uint32_t end_sector = (end_address + W25Q_SECTOR_SIZE - 1) / W25Q_SECTOR_SIZE;

    for (uint32_t sector = start_sector; sector < end_sector; sector++) {
        if (W25Q_EraseSector(g_current_handle->flash, sector) != W25Q_OK) {
            return false;
        }
    }

    return true;
}

// =============================================================================
// FFS CALLBACK FUNCTIONS
// =============================================================================

static bool ffs_file_start_callback(const char* filename, uint32_t filesize)
{
    if (!g_current_handle || !g_current_handle->ffs) {
        return false;
    }

    // Store filename for later use
    strncpy(g_pending_filename, filename, YMODEM_MAX_FILENAME_LEN - 1);
    g_pending_filename[YMODEM_MAX_FILENAME_LEN - 1] = '\0';
    g_pending_filesize = filesize;

    FFS_Handle* ffs = g_current_handle->ffs;
    FFS_Status status;
    uint32_t max_size;

    // Check if file already exists
    if (FFS_Exists(ffs, filename)) {
        // Get existing file info
        FFS_FileInfo existing_info;
        status = FFS_GetInfo(ffs, filename, &existing_info);

        if (status == FFS_OK) {
            // Check if new file fits in existing allocated space
            int file_idx = FFS_FindFile(ffs, filename);
            if (file_idx >= 0) {
                FFS_FileEntry* entry = &ffs->table.files[file_idx];

                if (filesize <= entry->allocated_size) {
                    // Reuse existing address - just erase and overwrite
                    g_write_address = entry->start_address;

                    // Erase sectors for the file
                    uint32_t sectors_needed = (filesize + FFS_SECTOR_SIZE - 1) / FFS_SECTOR_SIZE;
                    uint32_t start_sector = g_write_address / FFS_SECTOR_SIZE;
                    for (uint32_t i = 0; i < sectors_needed; i++) {
                        W25Q_EraseSector(g_current_handle->flash, start_sector + i);
                    }

                    // Update file entry (size will be updated in FinalizeWrite)
                    entry->file_size = 0;  // Will be set after transfer
                    entry->file_type = g_pending_filetype;

                    //printf("Overwriting existing file at 0x%06lX\r\n", g_write_address);
                    return true;
                }
            }
        }

        // File exists but new size is larger - delete and create new
        printf("File too large for existing slot, reallocating...\r\n");
        FFS_Delete(ffs, filename);
    }

    // Create new file in FFS
    status = FFS_Create(ffs, filename, filesize, g_pending_filetype);
    if (status != FFS_OK) {
        printf("FFS_Create failed: %s\r\n", FFS_GetStatusString(status));
        return false;
    }

    // Get write address
    status = FFS_GetWriteAddress(ffs, filename, &g_write_address, &max_size);
    if (status != FFS_OK) {
        printf("FFS_GetWriteAddress failed: %s\r\n", FFS_GetStatusString(status));
        return false;
    }

    return true;
}

static bool ffs_write_callback(uint32_t address, const uint8_t* data, uint32_t size)
{
    if (!g_current_handle || !g_current_handle->flash) {
        return false;
    }

    // YMODEM passes offset from FFS_DATA_START, we need to write to actual file address
    // address from YMODEM is relative offset, g_write_address is absolute flash address
    uint32_t actual_address = g_write_address + (address - FFS_DATA_START);

    W25Q_Status status = W25Q_Write(g_current_handle->flash, actual_address, data, size);
    return (status == W25Q_OK);
}

static bool ffs_erase_callback(uint32_t address, uint32_t size)
{
    // FFS already handles sector erase in FFS_GetWriteAddress
    (void)address;
    (void)size;
    return true;
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

YMODEM_Status YMODEM_Flash_Init(YMODEM_Flash_Handle* handle,
                                 UART_HandleTypeDef* huart,
                                 W25Q_Handle* flash)
{
    if (!handle || !huart || !flash) {
        return YMODEM_ERROR_PARAM;
    }

    memset(handle, 0, sizeof(YMODEM_Flash_Handle));
    handle->flash = flash;
    handle->ffs = NULL;
    handle->use_ffs = false;

    return YMODEM_Init(&handle->ymodem, huart);
}

YMODEM_Status YMODEM_Flash_InitWithFFS(YMODEM_Flash_Handle* handle,
                                        UART_HandleTypeDef* huart,
                                        W25Q_Handle* flash,
                                        FFS_Handle* ffs)
{
    if (!handle || !huart || !flash || !ffs) {
        return YMODEM_ERROR_PARAM;
    }

    memset(handle, 0, sizeof(YMODEM_Flash_Handle));
    handle->flash = flash;
    handle->ffs = ffs;
    handle->use_ffs = true;

    return YMODEM_Init(&handle->ymodem, huart);
}

YMODEM_Status YMODEM_Flash_Receive(YMODEM_Flash_Handle* handle,
                                    uint32_t flash_address,
                                    uint32_t max_size,
                                    YMODEM_Flash_Result* result)
{
    if (!handle || !handle->flash) {
        return YMODEM_ERROR_PARAM;
    }

    // Set global handle
    g_current_handle = handle;
    g_write_address = flash_address;
    g_max_size = max_size;

    // Configure YMODEM
    YMODEM_Status status = YMODEM_ConfigureFlash(&handle->ymodem,
                                                  flash_address,
                                                  max_size,
                                                  flash_write_callback,
                                                  flash_erase_callback);
    if (status != YMODEM_OK) {
        g_current_handle = NULL;
        return status;
    }

    // Start transfer
    YMODEM_FileInfo file_info;
    status = YMODEM_Receive(&handle->ymodem, &file_info);

    // Fill result
    if (result) {
        if (file_info.valid) {
            strncpy(result->filename, file_info.filename, YMODEM_MAX_FILENAME_LEN);
            result->filesize = file_info.filesize;
        } else {
            result->filename[0] = '\0';
            result->filesize = 0;
        }
        result->bytes_received = YMODEM_GetBytesReceived(&handle->ymodem);
        result->success = (status == YMODEM_OK);
        result->status = status;
    }

    g_current_handle = NULL;
    return status;
}

YMODEM_Status YMODEM_Flash_ReceiveToFFS(YMODEM_Flash_Handle* handle,
                                         uint32_t max_size,
                                         FFS_FileType file_type,
                                         YMODEM_Flash_Result* result)
{
    if (!handle || !handle->flash || !handle->ffs) {
        return YMODEM_ERROR_PARAM;
    }

    // Set global handles
    g_current_handle = handle;
    g_pending_filetype = file_type;
    g_pending_filename[0] = '\0';
    g_pending_filesize = 0;

    // Configure YMODEM with FFS callbacks
    // Note: start address will be set by file_start callback
    YMODEM_Status status = YMODEM_ConfigureFlash(&handle->ymodem,
                                                  FFS_DATA_START,  // Placeholder
                                                  max_size,
                                                  ffs_write_callback,
                                                  ffs_erase_callback);
    if (status != YMODEM_OK) {
        g_current_handle = NULL;
        return status;
    }

    // Set file start callback to handle FFS file creation
    YMODEM_SetFileStartCallback(&handle->ymodem, ffs_file_start_callback);

    // Start transfer
    YMODEM_FileInfo file_info;
    status = YMODEM_Receive(&handle->ymodem, &file_info);

    // Finalize FFS file if successful
    if (status == YMODEM_OK && file_info.valid && g_pending_filename[0] != '\0') {
        uint32_t bytes_received = YMODEM_GetBytesReceived(&handle->ymodem);
        FFS_FinalizeWrite(handle->ffs, g_pending_filename, bytes_received);

        // Auto-defragment if fragmentation is high (>25% wasted space)
        uint32_t used = FFS_GetUsedSpace(handle->ffs);
        uint32_t actual_data = handle->ffs->table.next_free_address - FFS_DATA_START;
        if (actual_data > 0 && used < actual_data) {
            uint32_t wasted = actual_data - used;
            uint32_t wasted_percent = (wasted * 100) / actual_data;
            if (wasted_percent > 25) {
                printf("Auto-defrag: %lu%% fragmentation detected\r\n", (unsigned long)wasted_percent);
                FFS_Defragment(handle->ffs);
            }
        }
    }

    // Fill result
    if (result) {
        if (file_info.valid) {
            strncpy(result->filename, file_info.filename, YMODEM_MAX_FILENAME_LEN);
            result->filesize = file_info.filesize;
        } else {
            result->filename[0] = '\0';
            result->filesize = 0;
        }
        result->bytes_received = YMODEM_GetBytesReceived(&handle->ymodem);
        result->success = (status == YMODEM_OK);
        result->status = status;
    }

    // Clear callback
    YMODEM_SetFileStartCallback(&handle->ymodem, NULL);

    g_current_handle = NULL;
    return status;
}

const YMODEM_FileInfo* YMODEM_Flash_GetFileInfo(YMODEM_Flash_Handle* handle)
{
    if (!handle) return NULL;
    return YMODEM_GetFileInfo(&handle->ymodem);
}

void YMODEM_Flash_Cancel(YMODEM_Flash_Handle* handle)
{
    if (handle) {
        YMODEM_Cancel(&handle->ymodem);
    }
}

// =============================================================================
// NON-BLOCKING (POLLING) FUNCTIONS
// =============================================================================

// Static variables for polling mode
__attribute__((unused)) static uint32_t g_poll_max_size = 0;
__attribute__((unused)) static FFS_FileType g_poll_file_type = FFS_TYPE_UNKNOWN;
static uint32_t g_last_ready_signal_time = 0;

void YMODEM_Flash_StartListen(YMODEM_Flash_Handle* handle, uint32_t max_size, FFS_FileType file_type)
{
    if (!handle) return;

    g_current_handle = handle;
    g_poll_max_size = max_size;
    g_poll_file_type = file_type;
    g_pending_filetype = file_type;
    g_pending_filename[0] = '\0';
    g_pending_filesize = 0;

    // Configure YMODEM
    if (handle->use_ffs && handle->ffs) {
        YMODEM_ConfigureFlash(&handle->ymodem,
                              FFS_DATA_START,
                              max_size,
                              ffs_write_callback,
                              ffs_erase_callback);
        YMODEM_SetFileStartCallback(&handle->ymodem, ffs_file_start_callback);
    } else {
        YMODEM_ConfigureFlash(&handle->ymodem,
                              g_write_address,
                              max_size,
                              flash_write_callback,
                              flash_erase_callback);
    }

    // Start listening
    YMODEM_StartListen(&handle->ymodem);
    g_last_ready_signal_time = HAL_GetTick();
}

bool YMODEM_Flash_IsTransferActive(YMODEM_Flash_Handle* handle)
{
    if (!handle) return false;
    return YMODEM_IsTransferActive(&handle->ymodem);
}

void YMODEM_Flash_SendReadySignal(YMODEM_Flash_Handle* handle, uint32_t interval_ms)
{
    if (!handle) return;

    // Don't send if transfer active
    if (YMODEM_IsTransferActive(&handle->ymodem)) return;

    uint32_t now = HAL_GetTick();
    if ((now - g_last_ready_signal_time) >= interval_ms) {
        YMODEM_StartListen(&handle->ymodem);
        g_last_ready_signal_time = now;
    }
}

YMODEM_Status YMODEM_Flash_Poll(YMODEM_Flash_Handle* handle, YMODEM_Flash_Result* result)
{
    if (!handle) return YMODEM_ERROR_PARAM;

    // Clear result
    if (result) {
        memset(result, 0, sizeof(YMODEM_Flash_Result));
    }

    // Check for incoming transfer start
    if (!YMODEM_IsTransferActive(&handle->ymodem)) {
        if (YMODEM_CheckStart(&handle->ymodem)) {
            // Transfer starting, poll will handle it
            g_current_handle = handle;
        } else {
            // No transfer yet
            return YMODEM_ERROR_BUSY;
        }
    }

    // Poll transfer state machine
    YMODEM_Status status = YMODEM_Poll(&handle->ymodem);

    // If transfer complete
    if (status == YMODEM_OK) {
        const YMODEM_FileInfo* file_info = YMODEM_GetFileInfo(&handle->ymodem);

        // Finalize FFS file if needed
        if (handle->use_ffs && handle->ffs && file_info && file_info->valid && g_pending_filename[0] != '\0') {
            uint32_t bytes_received = YMODEM_GetBytesReceived(&handle->ymodem);
            FFS_FinalizeWrite(handle->ffs, g_pending_filename, bytes_received);
        }

        // Fill result
        if (result) {
            if (file_info && file_info->valid) {
                strncpy(result->filename, file_info->filename, YMODEM_MAX_FILENAME_LEN);
                result->filesize = file_info->filesize;
            }
            result->bytes_received = YMODEM_GetBytesReceived(&handle->ymodem);
            result->success = true;
            result->status = YMODEM_OK;
        }

        // Reset for next transfer
        g_pending_filename[0] = '\0';
        g_current_handle = NULL;

        return YMODEM_OK;
    }

    // If error
    if (status != YMODEM_ERROR_BUSY) {
        if (result) {
            result->success = false;
            result->status = status;
            result->bytes_received = YMODEM_GetBytesReceived(&handle->ymodem);
        }

        // Reset for next transfer
        g_pending_filename[0] = '\0';
        g_current_handle = NULL;

        return status;
    }

    // Still in progress
    return YMODEM_ERROR_BUSY;
}
