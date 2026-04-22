/*
 * flash_fs.c
 * Simple Flash File System for W25Q128JW
 * Lightweight file management without FatFS
 */

#include <flash_fs.h>
#include <string.h>
#include <stdio.h>

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

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

static uint32_t align_to_sector(uint32_t size)
{
    return ((size + FFS_SECTOR_SIZE - 1) / FFS_SECTOR_SIZE) * FFS_SECTOR_SIZE;
}

static FFS_Status write_table(FFS_Handle* handle)
{
    if (!handle || !handle->flash) {
        return FFS_ERROR_INVALID_PARAM;
    }

    // Erase table sector
    if (W25Q_EraseSector(handle->flash, 0) != W25Q_OK) {
        return FFS_ERROR_FLASH;
    }

    // Write table
    if (W25Q_Write(handle->flash, FFS_TABLE_ADDRESS,
                   (uint8_t*)&handle->table, sizeof(FFS_FileTable)) != W25Q_OK) {
        return FFS_ERROR_FLASH;
    }

    handle->table_dirty = false;
    return FFS_OK;
}

static FFS_Status read_table(FFS_Handle* handle)
{
    if (!handle || !handle->flash) {
        return FFS_ERROR_INVALID_PARAM;
    }

    if (W25Q_Read(handle->flash, FFS_TABLE_ADDRESS,
                  (uint8_t*)&handle->table, sizeof(FFS_FileTable)) != W25Q_OK) {
        return FFS_ERROR_FLASH;
    }

    return FFS_OK;
}

static uint32_t calculate_file_crc(FFS_Handle* handle, uint32_t address, uint32_t size)
{
    uint32_t crc = 0xFFFFFFFF;
    uint8_t buffer[256];
    uint32_t remaining = size;
    uint32_t addr = address;

    while (remaining > 0) {
        uint32_t chunk = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining;

        if (W25Q_Read(handle->flash, addr, buffer, chunk) != W25Q_OK) {
            return 0;
        }

        for (uint32_t i = 0; i < chunk; i++) {
            crc = crc32_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
        }

        addr += chunk;
        remaining -= chunk;
    }

    return crc ^ 0xFFFFFFFF;
}

// =============================================================================
// PUBLIC FUNCTIONS - INITIALIZATION
// =============================================================================

FFS_Status FFS_Init(FFS_Handle* handle, W25Q_Handle* flash)
{
    if (!handle || !flash) {
        return FFS_ERROR_INVALID_PARAM;
    }

    memset(handle, 0, sizeof(FFS_Handle));
    handle->flash = flash;

    // Try to read existing table
    FFS_Status status = read_table(handle);
    if (status != FFS_OK) {
        return status;
    }

    // Check if table is valid
    if (handle->table.magic != FFS_TABLE_MAGIC) {
        // No valid file system - format it
        return FFS_Format(handle);
    }

    handle->initialized = true;
    return FFS_OK;
}

FFS_Status FFS_Format(FFS_Handle* handle)
{
    if (!handle || !handle->flash) {
        return FFS_ERROR_INVALID_PARAM;
    }

    // Initialize empty table
    memset(&handle->table, 0xFF, sizeof(FFS_FileTable));
    handle->table.magic = FFS_TABLE_MAGIC;
    handle->table.version = 1;
    handle->table.file_count = 0;
    handle->table.next_free_address = FFS_DATA_START;
    handle->table.total_used = 0;

    // Clear all file entries
    for (int i = 0; i < FFS_MAX_FILES; i++) {
        handle->table.files[i].magic = 0xFFFFFFFF;
    }

    // Write table to flash
    FFS_Status status = write_table(handle);
    if (status != FFS_OK) {
        return status;
    }

    handle->initialized = true;
    return FFS_OK;
}

bool FFS_IsValid(FFS_Handle* handle)
{
    if (!handle) return false;
    return handle->initialized && (handle->table.magic == FFS_TABLE_MAGIC);
}

// =============================================================================
// PUBLIC FUNCTIONS - FILE OPERATIONS
// =============================================================================

int FFS_FindFile(FFS_Handle* handle, const char* filename)
{
    if (!handle || !filename || !handle->initialized) {
        return -1;
    }

    for (int i = 0; i < FFS_MAX_FILES; i++) {
        if (handle->table.files[i].magic == FFS_FILE_MAGIC &&
            (handle->table.files[i].flags & FFS_FLAG_VALID) &&
            !(handle->table.files[i].flags & FFS_FLAG_DELETED)) {

            if (strncmp(handle->table.files[i].filename, filename, FFS_MAX_FILENAME_LEN) == 0) {
                return i;
            }
        }
    }

    return -1;
}

bool FFS_Exists(FFS_Handle* handle, const char* filename)
{
    return FFS_FindFile(handle, filename) >= 0;
}

FFS_Status FFS_Create(FFS_Handle* handle, const char* filename,
                       uint32_t max_size, FFS_FileType file_type)
{
    if (!handle || !filename || !handle->initialized) {
        return FFS_ERROR_NOT_INITIALIZED;
    }

    // Validate filename
    size_t len = strlen(filename);
    if (len == 0 || len >= FFS_MAX_FILENAME_LEN) {
        return FFS_ERROR_INVALID_NAME;
    }

    // Check if file already exists
    if (FFS_Exists(handle, filename)) {
        return FFS_ERROR_FILE_EXISTS;
    }

    // Find free slot
    int slot = -1;
    for (int i = 0; i < FFS_MAX_FILES; i++) {
        if (handle->table.files[i].magic != FFS_FILE_MAGIC ||
            (handle->table.files[i].flags & FFS_FLAG_DELETED)) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        return FFS_ERROR_TABLE_FULL;
    }

    // Calculate allocated size (sector-aligned)
    uint32_t allocated = align_to_sector(max_size);

    // Check available space
    if (handle->table.next_free_address + allocated > FFS_DATA_END) {
        return FFS_ERROR_NO_SPACE;
    }

    // Create file entry
    FFS_FileEntry* entry = &handle->table.files[slot];
    memset(entry, 0, sizeof(FFS_FileEntry));

    entry->magic = FFS_FILE_MAGIC;
    strncpy(entry->filename, filename, FFS_MAX_FILENAME_LEN - 1);
    entry->start_address = handle->table.next_free_address;
    entry->file_size = 0;  // Will be updated when data is written
    entry->allocated_size = allocated;
    entry->crc32 = 0;
    entry->timestamp = HAL_GetTick();
    entry->file_type = (uint8_t)file_type;
    entry->flags = FFS_FLAG_VALID;

    // Update table
    handle->table.next_free_address += allocated;
    handle->table.file_count++;
    handle->table.total_used += allocated;

    // Erase allocated sectors
    uint32_t start_sector = entry->start_address / FFS_SECTOR_SIZE;
    uint32_t num_sectors = allocated / FFS_SECTOR_SIZE;

    for (uint32_t i = 0; i < num_sectors; i++) {
        if (W25Q_EraseSector(handle->flash, start_sector + i) != W25Q_OK) {
            return FFS_ERROR_FLASH;
        }
    }

    // Write updated table
    return write_table(handle);
}

FFS_Status FFS_Delete(FFS_Handle* handle, const char* filename)
{
    if (!handle || !filename || !handle->initialized) {
        return FFS_ERROR_NOT_INITIALIZED;
    }

    int index = FFS_FindFile(handle, filename);
    if (index < 0) {
        return FFS_ERROR_FILE_NOT_FOUND;
    }

    FFS_FileEntry* entry = &handle->table.files[index];
    uint32_t deleted_end = entry->start_address + entry->allocated_size;

    // Mark as deleted
    entry->flags |= FFS_FLAG_DELETED;
    entry->flags &= ~FFS_FLAG_VALID;
    handle->table.file_count--;
    handle->table.total_used -= entry->allocated_size;

    // If this was the last file in flash, reclaim the space
    if (deleted_end == handle->table.next_free_address) {
        handle->table.next_free_address = entry->start_address;

        // Also check for consecutive deleted files at the end and reclaim them too
        bool found_more = true;
        while (found_more) {
            found_more = false;
            for (int i = 0; i < FFS_MAX_FILES; i++) {
                FFS_FileEntry* e = &handle->table.files[i];
                if ((e->flags & FFS_FLAG_DELETED) &&
                    (e->start_address + e->allocated_size == handle->table.next_free_address)) {
                    handle->table.next_free_address = e->start_address;
                    // Clear the deleted entry completely
                    memset(e, 0, sizeof(FFS_FileEntry));
                    found_more = true;
                    break;
                }
            }
        }

        // Clear this entry completely since space is reclaimed
        memset(entry, 0, sizeof(FFS_FileEntry));
    }

    return write_table(handle);
}

FFS_Status FFS_GetInfo(FFS_Handle* handle, const char* filename, FFS_FileInfo* info)
{
    if (!handle || !filename || !info || !handle->initialized) {
        return FFS_ERROR_INVALID_PARAM;
    }

    int index = FFS_FindFile(handle, filename);
    if (index < 0) {
        return FFS_ERROR_FILE_NOT_FOUND;
    }

    return FFS_GetInfoByIndex(handle, index, info);
}

FFS_Status FFS_GetInfoByIndex(FFS_Handle* handle, uint8_t index, FFS_FileInfo* info)
{
    if (!handle || !info || !handle->initialized) {
        return FFS_ERROR_INVALID_PARAM;
    }

    if (index >= FFS_MAX_FILES) {
        return FFS_ERROR_INVALID_PARAM;
    }

    FFS_FileEntry* entry = &handle->table.files[index];

    if (entry->magic != FFS_FILE_MAGIC ||
        !(entry->flags & FFS_FLAG_VALID) ||
        (entry->flags & FFS_FLAG_DELETED)) {
        return FFS_ERROR_FILE_NOT_FOUND;
    }

    strncpy(info->filename, entry->filename, FFS_MAX_FILENAME_LEN);
    info->file_size = entry->file_size;
    info->crc32 = entry->crc32;
    info->timestamp = entry->timestamp;
    info->file_type = entry->file_type;
    info->flags = entry->flags;
    info->index = index;

    return FFS_OK;
}

// =============================================================================
// PUBLIC FUNCTIONS - READ/WRITE
// =============================================================================

FFS_Status FFS_Write(FFS_Handle* handle, const char* filename,
                      const uint8_t* data, uint32_t size)
{
    if (!handle || !filename || !data || !handle->initialized) {
        return FFS_ERROR_INVALID_PARAM;
    }

    int index = FFS_FindFile(handle, filename);
    if (index < 0) {
        return FFS_ERROR_FILE_NOT_FOUND;
    }

    FFS_FileEntry* entry = &handle->table.files[index];

    // Check size
    if (size > entry->allocated_size) {
        return FFS_ERROR_NO_SPACE;
    }

    // Erase sectors
    uint32_t start_sector = entry->start_address / FFS_SECTOR_SIZE;
    uint32_t num_sectors = align_to_sector(size) / FFS_SECTOR_SIZE;

    for (uint32_t i = 0; i < num_sectors; i++) {
        if (W25Q_EraseSector(handle->flash, start_sector + i) != W25Q_OK) {
            return FFS_ERROR_FLASH;
        }
    }

    // Write data
    if (W25Q_Write(handle->flash, entry->start_address, data, size) != W25Q_OK) {
        return FFS_ERROR_FLASH;
    }

    // Update entry
    entry->file_size = size;
    entry->crc32 = FFS_CalculateCRC32(data, size);
    entry->timestamp = HAL_GetTick();

    return write_table(handle);
}

FFS_Status FFS_Read(FFS_Handle* handle, const char* filename,
                     uint8_t* buffer, uint32_t offset, uint32_t size,
                     uint32_t* bytes_read)
{
    if (!handle || !filename || !buffer || !handle->initialized) {
        return FFS_ERROR_INVALID_PARAM;
    }

    int index = FFS_FindFile(handle, filename);
    if (index < 0) {
        return FFS_ERROR_FILE_NOT_FOUND;
    }

    FFS_FileEntry* entry = &handle->table.files[index];

    // Check bounds
    if (offset >= entry->file_size) {
        if (bytes_read) *bytes_read = 0;
        return FFS_OK;
    }

    uint32_t to_read = size;
    if (offset + size > entry->file_size) {
        to_read = entry->file_size - offset;
    }

    // Read data
    if (W25Q_Read(handle->flash, entry->start_address + offset, buffer, to_read) != W25Q_OK) {
        return FFS_ERROR_FLASH;
    }

    if (bytes_read) {
        *bytes_read = to_read;
    }

    return FFS_OK;
}

FFS_Status FFS_GetWriteAddress(FFS_Handle* handle, const char* filename,
                                uint32_t* address, uint32_t* max_size)
{
    if (!handle || !filename || !address || !handle->initialized) {
        return FFS_ERROR_INVALID_PARAM;
    }

    int index = FFS_FindFile(handle, filename);
    if (index < 0) {
        return FFS_ERROR_FILE_NOT_FOUND;
    }

    FFS_FileEntry* entry = &handle->table.files[index];

    *address = entry->start_address;
    if (max_size) {
        *max_size = entry->allocated_size;
    }

    // Erase allocated sectors for writing
    uint32_t start_sector = entry->start_address / FFS_SECTOR_SIZE;
    uint32_t num_sectors = entry->allocated_size / FFS_SECTOR_SIZE;

    for (uint32_t i = 0; i < num_sectors; i++) {
        if (W25Q_EraseSector(handle->flash, start_sector + i) != W25Q_OK) {
            return FFS_ERROR_FLASH;
        }
    }

    return FFS_OK;
}

FFS_Status FFS_FinalizeWrite(FFS_Handle* handle, const char* filename,
                              uint32_t actual_size)
{
    if (!handle || !filename || !handle->initialized) {
        return FFS_ERROR_INVALID_PARAM;
    }

    int index = FFS_FindFile(handle, filename);
    if (index < 0) {
        return FFS_ERROR_FILE_NOT_FOUND;
    }

    FFS_FileEntry* entry = &handle->table.files[index];

    // Update file size
    entry->file_size = actual_size;
    entry->timestamp = HAL_GetTick();

    // Calculate CRC
    entry->crc32 = calculate_file_crc(handle, entry->start_address, actual_size);

    return write_table(handle);
}

// =============================================================================
// PUBLIC FUNCTIONS - VERIFICATION
// =============================================================================

bool FFS_Verify(FFS_Handle* handle, const char* filename)
{
    if (!handle || !filename || !handle->initialized) {
        return false;
    }

    int index = FFS_FindFile(handle, filename);
    if (index < 0) {
        return false;
    }

    FFS_FileEntry* entry = &handle->table.files[index];

    if (entry->file_size == 0) {
        return true;  // Empty file is valid
    }

    uint32_t calculated_crc = calculate_file_crc(handle, entry->start_address, entry->file_size);

    return (calculated_crc == entry->crc32);
}

// =============================================================================
// PUBLIC FUNCTIONS - STATISTICS
// =============================================================================

uint32_t FFS_GetFileCount(FFS_Handle* handle)
{
    if (!handle || !handle->initialized) return 0;
    return handle->table.file_count;
}

uint32_t FFS_GetUsedSpace(FFS_Handle* handle)
{
    if (!handle || !handle->initialized) return 0;
    return handle->table.total_used;
}

uint32_t FFS_GetFreeSpace(FFS_Handle* handle)
{
    if (!handle || !handle->initialized) return 0;
    return FFS_DATA_END - handle->table.next_free_address;
}

void FFS_ListFiles(FFS_Handle* handle)
{
    if (!handle || !handle->initialized) {
        printf("File system not initialized\r\n");
        return;
    }

    printf("\r\n=== Flash File System ===\r\n");
    printf("Files: %lu / %d\r\n", (unsigned long)handle->table.file_count, FFS_MAX_FILES);
    printf("Used: %lu bytes\r\n", (unsigned long)handle->table.total_used);
    printf("Free: %lu bytes\r\n", (unsigned long)FFS_GetFreeSpace(handle));
    printf("\r\n");

    if (handle->table.file_count == 0) {
        printf("No files.\r\n");
        return;
    }

    printf("%-24s %10s %8s %s\r\n", "Filename", "Size", "Type", "Status");
    printf("----------------------------------------------------\r\n");

    int count = 0;
    for (int i = 0; i < FFS_MAX_FILES && count < (int)handle->table.file_count; i++) {
        FFS_FileEntry* entry = &handle->table.files[i];

        if (entry->magic == FFS_FILE_MAGIC &&
            (entry->flags & FFS_FLAG_VALID) &&
            !(entry->flags & FFS_FLAG_DELETED)) {

            const char* type_str = "???";
            switch (entry->file_type) {
                case FFS_TYPE_PLC_PROGRAM: type_str = "PLC"; break;
                case FFS_TYPE_CONFIG:      type_str = "CFG"; break;
                case FFS_TYPE_FIRMWARE:    type_str = "FW"; break;
                case FFS_TYPE_DATA:        type_str = "DAT"; break;
                case FFS_TYPE_LOG:         type_str = "LOG"; break;
                default:                   type_str = "UNK"; break;
            }

            const char* status = FFS_Verify(handle, entry->filename) ? "OK" : "ERR";

            printf("%-24s %10lu %8s %s\r\n",
                   entry->filename, (unsigned long)entry->file_size, type_str, status);
            count++;
        }
    }
    printf("============================\r\n");
}

// =============================================================================
// PUBLIC FUNCTIONS - UTILITY
// =============================================================================

uint32_t FFS_CalculateCRC32(const uint8_t* data, uint32_t size)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < size; i++) {
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFF;
}

const char* FFS_GetStatusString(FFS_Status status)
{
    switch (status) {
        case FFS_OK:                    return "OK";
        case FFS_ERROR_NOT_INITIALIZED: return "Not initialized";
        case FFS_ERROR_FLASH:           return "Flash error";
        case FFS_ERROR_NO_SPACE:        return "No space";
        case FFS_ERROR_FILE_NOT_FOUND:  return "File not found";
        case FFS_ERROR_FILE_EXISTS:     return "File exists";
        case FFS_ERROR_INVALID_NAME:    return "Invalid name";
        case FFS_ERROR_TABLE_FULL:      return "Table full";
        case FFS_ERROR_INVALID_PARAM:   return "Invalid param";
        case FFS_ERROR_FILE_OPEN:       return "File open";
        case FFS_ERROR_CRC:             return "CRC error";
        default:                        return "Unknown";
    }
}

FFS_Status FFS_Defragment(FFS_Handle* handle)
{
    if (!handle || !handle->initialized) {
        return FFS_ERROR_NOT_INITIALIZED;
    }

    // Simple defragmentation: move files to fill gaps
    // Uses 4KB buffer to copy sector by sector (RAM efficient)
    static uint8_t sector_buffer[FFS_SECTOR_SIZE];

    uint32_t next_addr = FFS_DATA_START;
    uint32_t files_moved = 0;

    // Sort files by address (bubble sort - simple, few files)
    // First, collect valid file indices
    int valid_indices[FFS_MAX_FILES];
    int valid_count = 0;

    for (int i = 0; i < FFS_MAX_FILES; i++) {
        FFS_FileEntry* e = &handle->table.files[i];
        if ((e->flags & FFS_FLAG_VALID) && !(e->flags & FFS_FLAG_DELETED)) {
            valid_indices[valid_count++] = i;
        }
    }

    // Sort by start_address
    for (int i = 0; i < valid_count - 1; i++) {
        for (int j = i + 1; j < valid_count; j++) {
            if (handle->table.files[valid_indices[i]].start_address >
                handle->table.files[valid_indices[j]].start_address) {
                int tmp = valid_indices[i];
                valid_indices[i] = valid_indices[j];
                valid_indices[j] = tmp;
            }
        }
    }

    // Move files to compact them
    for (int i = 0; i < valid_count; i++) {
        FFS_FileEntry* entry = &handle->table.files[valid_indices[i]];

        if (entry->start_address != next_addr) {
            // Need to move this file
            uint32_t sectors = (entry->allocated_size + FFS_SECTOR_SIZE - 1) / FFS_SECTOR_SIZE;

            printf("Moving %s: 0x%06lX -> 0x%06lX\r\n",
                   entry->filename, (unsigned long)entry->start_address, (unsigned long)next_addr);

            for (uint32_t s = 0; s < sectors; s++) {
                // Read sector from old location
                uint32_t old_addr = entry->start_address + (s * FFS_SECTOR_SIZE);
                uint32_t new_addr = next_addr + (s * FFS_SECTOR_SIZE);

                if (W25Q_Read(handle->flash, old_addr, sector_buffer, FFS_SECTOR_SIZE) != W25Q_OK) {
                    return FFS_ERROR_FLASH;
                }

                // Erase new sector
                uint32_t new_sector = new_addr / FFS_SECTOR_SIZE;
                if (W25Q_EraseSector(handle->flash, new_sector) != W25Q_OK) {
                    return FFS_ERROR_FLASH;
                }

                // Write to new location
                if (W25Q_Write(handle->flash, new_addr, sector_buffer, FFS_SECTOR_SIZE) != W25Q_OK) {
                    return FFS_ERROR_FLASH;
                }
            }

            // Update file entry
            entry->start_address = next_addr;
            files_moved++;
        }

        next_addr += entry->allocated_size;
    }

    // Clear deleted entries from table
    for (int i = 0; i < FFS_MAX_FILES; i++) {
        FFS_FileEntry* e = &handle->table.files[i];
        if (e->flags & FFS_FLAG_DELETED) {
            memset(e, 0, sizeof(FFS_FileEntry));
        }
    }

    // Update next free address
    handle->table.next_free_address = next_addr;

    // Recalculate total used
    handle->table.total_used = 0;
    for (int i = 0; i < valid_count; i++) {
        handle->table.total_used += handle->table.files[valid_indices[i]].allocated_size;
    }

    printf("Defragment: %lu files moved, free addr: 0x%06lX\r\n",
           (unsigned long)files_moved, (unsigned long)handle->table.next_free_address);

    return write_table(handle);
}
