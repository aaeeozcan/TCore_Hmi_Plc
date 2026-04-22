/*
 * flash_fs.h
 * Simple Flash File System for SPI Flash
 * Lightweight file management without FatFS
 *
 * Supports: SST25VF016B (2MB)
 *
 * Flash Layout:
 * +------------------+ 0x000000
 * | File Table       | 4KB (1 sector) - Max 63 files
 * +------------------+ 0x001000
 * | File Data Area   | ~2MB
 * +------------------+ 0x200000 (End of Flash)
 */

#ifndef FLASH_FS_H
#define FLASH_FS_H

#include <stdint.h>
#include <stdbool.h>
#include "sst25vf016b.h"

// =============================================================================
// CONFIGURATION
// =============================================================================

#define FFS_MAX_FILENAME_LEN    24      // Max filename length (including null)
#define FFS_MAX_FILES           63      // Max number of files
#define FFS_SECTOR_SIZE         4096    // Flash sector size (4KB)

// Flash layout (SST25VF016B - 2MB)
#define FFS_TABLE_ADDRESS       0x000000    // File table location
#define FFS_TABLE_SIZE          FFS_SECTOR_SIZE  // 4KB for file table
#define FFS_DATA_START          0x001000    // File data starts here
#define FFS_DATA_END            0x200000    // 2MB total flash
#define FFS_FLASH_SIZE          (2 * 1024 * 1024)

// Magic numbers
#define FFS_TABLE_MAGIC         0x46465331  // "FFS1" - Flash File System v1
#define FFS_FILE_MAGIC          0x46494C45  // "FILE"

// =============================================================================
// FILE ENTRY STRUCTURE (64 bytes each)
// =============================================================================

typedef struct __attribute__((packed)) {
    uint32_t magic;                     // FFS_FILE_MAGIC if valid
    char     filename[FFS_MAX_FILENAME_LEN];  // Null-terminated filename
    uint32_t start_address;             // Start address in flash
    uint32_t file_size;                 // File size in bytes
    uint32_t allocated_size;            // Allocated size (sector-aligned)
    uint32_t crc32;                     // CRC32 of file data
    uint32_t timestamp;                 // Creation/modification time
    uint8_t  file_type;                 // File type (FlashFileType)
    uint8_t  flags;                     // File flags
    uint8_t  reserved[10];              // Reserved for future use
} FFS_FileEntry;  // Total: 64 bytes

// File flags
#define FFS_FLAG_VALID      0x01    // File is valid
#define FFS_FLAG_READONLY   0x02    // File is read-only
#define FFS_FLAG_SYSTEM     0x04    // System file
#define FFS_FLAG_DELETED    0x80    // File marked for deletion

// =============================================================================
// FILE TABLE STRUCTURE (4KB total)
// =============================================================================

typedef struct __attribute__((packed)) {
    uint32_t magic;                     // FFS_TABLE_MAGIC
    uint32_t version;                   // File system version
    uint32_t file_count;                // Number of valid files
    uint32_t next_free_address;         // Next free address for allocation
    uint32_t total_used;                // Total bytes used
    uint32_t reserved[11];              // Reserved (pad to 64 bytes header)
    FFS_FileEntry files[FFS_MAX_FILES]; // File entries (63 * 64 = 4032 bytes)
} FFS_FileTable;  // Total: 64 + 4032 = 4096 bytes (1 sector)

// =============================================================================
// FILE HANDLE
// =============================================================================

typedef struct {
    uint8_t file_index;                 // Index in file table
    uint32_t start_address;             // File start address
    uint32_t file_size;                 // File size
    uint32_t position;                  // Current read/write position
    bool is_open;                       // File is open
    bool is_write;                      // Open for writing
} FFS_File;

// =============================================================================
// FILE SYSTEM HANDLE
// =============================================================================

typedef struct {
    W25Q_Handle* flash;                 // Flash handle
    FFS_FileTable table;                // Cached file table
    bool initialized;                   // FS initialized
    bool table_dirty;                   // Table needs to be written
} FFS_Handle;

// =============================================================================
// ERROR CODES
// =============================================================================

typedef enum {
    FFS_OK = 0,
    FFS_ERROR_NOT_INITIALIZED,
    FFS_ERROR_FLASH,
    FFS_ERROR_NO_SPACE,
    FFS_ERROR_FILE_NOT_FOUND,
    FFS_ERROR_FILE_EXISTS,
    FFS_ERROR_INVALID_NAME,
    FFS_ERROR_TABLE_FULL,
    FFS_ERROR_INVALID_PARAM,
    FFS_ERROR_FILE_OPEN,
    FFS_ERROR_CRC
} FFS_Status;

// =============================================================================
// FILE TYPE (same as xmodem_flash.h)
// =============================================================================

typedef enum {
    FFS_TYPE_UNKNOWN = 0,
    FFS_TYPE_PLC_PROGRAM = 1,
    FFS_TYPE_CONFIG = 2,
    FFS_TYPE_FIRMWARE = 3,
    FFS_TYPE_DATA = 4,
    FFS_TYPE_LOG = 5
} FFS_FileType;

// =============================================================================
// FILE INFO STRUCTURE
// =============================================================================

typedef struct {
    char     filename[FFS_MAX_FILENAME_LEN];
    uint32_t file_size;
    uint32_t crc32;
    uint32_t timestamp;
    uint8_t  file_type;
    uint8_t  flags;
    uint8_t  index;                     // Index in file table
} FFS_FileInfo;

// =============================================================================
// INITIALIZATION
// =============================================================================

/**
 * @brief Initialize flash file system
 * @param handle: FFS handle
 * @param flash: W25Q flash handle (must be initialized)
 * @return FFS_Status
 */
FFS_Status FFS_Init(FFS_Handle* handle, W25Q_Handle* flash);

/**
 * @brief Format flash file system (erase all files)
 * @param handle: FFS handle
 * @return FFS_Status
 */
FFS_Status FFS_Format(FFS_Handle* handle);

/**
 * @brief Check if file system is valid
 * @param handle: FFS handle
 * @return true if valid
 */
bool FFS_IsValid(FFS_Handle* handle);

// =============================================================================
// FILE OPERATIONS
// =============================================================================

/**
 * @brief Create a new file
 * @param handle: FFS handle
 * @param filename: File name
 * @param max_size: Maximum file size (will be sector-aligned)
 * @param file_type: File type
 * @return FFS_Status
 */
FFS_Status FFS_Create(FFS_Handle* handle, const char* filename,
                       uint32_t max_size, FFS_FileType file_type);

/**
 * @brief Delete a file
 * @param handle: FFS handle
 * @param filename: File name to delete
 * @return FFS_Status
 */
FFS_Status FFS_Delete(FFS_Handle* handle, const char* filename);

/**
 * @brief Check if file exists
 * @param handle: FFS handle
 * @param filename: File name
 * @return true if exists
 */
bool FFS_Exists(FFS_Handle* handle, const char* filename);

/**
 * @brief Get file info
 * @param handle: FFS handle
 * @param filename: File name
 * @param info: Pointer to store file info
 * @return FFS_Status
 */
FFS_Status FFS_GetInfo(FFS_Handle* handle, const char* filename, FFS_FileInfo* info);

/**
 * @brief Get file info by index
 * @param handle: FFS handle
 * @param index: File index (0 to file_count-1)
 * @param info: Pointer to store file info
 * @return FFS_Status
 */
FFS_Status FFS_GetInfoByIndex(FFS_Handle* handle, uint8_t index, FFS_FileInfo* info);

// =============================================================================
// FILE READ/WRITE
// =============================================================================

/**
 * @brief Write data to file (overwrites existing content)
 * @param handle: FFS handle
 * @param filename: File name
 * @param data: Data to write
 * @param size: Data size
 * @return FFS_Status
 */
FFS_Status FFS_Write(FFS_Handle* handle, const char* filename,
                      const uint8_t* data, uint32_t size);

/**
 * @brief Read data from file
 * @param handle: FFS handle
 * @param filename: File name
 * @param buffer: Buffer to store data
 * @param offset: Offset in file
 * @param size: Number of bytes to read
 * @param bytes_read: Actual bytes read (can be NULL)
 * @return FFS_Status
 */
FFS_Status FFS_Read(FFS_Handle* handle, const char* filename,
                     uint8_t* buffer, uint32_t offset, uint32_t size,
                     uint32_t* bytes_read);

/**
 * @brief Get flash address for direct XMODEM write
 * @param handle: FFS handle
 * @param filename: File name
 * @param address: Pointer to store start address
 * @param max_size: Pointer to store max size
 * @return FFS_Status
 */
FFS_Status FFS_GetWriteAddress(FFS_Handle* handle, const char* filename,
                                uint32_t* address, uint32_t* max_size);

/**
 * @brief Update file size and CRC after XMODEM write
 * @param handle: FFS handle
 * @param filename: File name
 * @param actual_size: Actual bytes written
 * @return FFS_Status
 */
FFS_Status FFS_FinalizeWrite(FFS_Handle* handle, const char* filename,
                              uint32_t actual_size);

// =============================================================================
// FILE VERIFICATION
// =============================================================================

/**
 * @brief Verify file CRC
 * @param handle: FFS handle
 * @param filename: File name
 * @return true if CRC matches
 */
bool FFS_Verify(FFS_Handle* handle, const char* filename);

// =============================================================================
// STATISTICS
// =============================================================================

/**
 * @brief Get file count
 * @param handle: FFS handle
 * @return Number of files
 */
uint32_t FFS_GetFileCount(FFS_Handle* handle);

/**
 * @brief Get total used space
 * @param handle: FFS handle
 * @return Bytes used
 */
uint32_t FFS_GetUsedSpace(FFS_Handle* handle);

/**
 * @brief Get free space
 * @param handle: FFS handle
 * @return Bytes free
 */
uint32_t FFS_GetFreeSpace(FFS_Handle* handle);

/**
 * @brief List all files (prints to console)
 * @param handle: FFS handle
 */
void FFS_ListFiles(FFS_Handle* handle);

// =============================================================================
// UTILITY
// =============================================================================

/**
 * @brief Find file by name
 * @param handle: FFS handle
 * @param filename: File name
 * @return File index or -1 if not found
 */
int FFS_FindFile(FFS_Handle* handle, const char* filename);

/**
 * @brief Defragment file system (compact files)
 * @param handle: FFS handle
 * @return FFS_Status
 * @note This is a slow operation!
 */
FFS_Status FFS_Defragment(FFS_Handle* handle);

/**
 * @brief Calculate CRC32
 * @param data: Data buffer
 * @param size: Data size
 * @return CRC32 value
 */
uint32_t FFS_CalculateCRC32(const uint8_t* data, uint32_t size);

/**
 * @brief Get status string
 * @param status: FFS_Status code
 * @return Status string
 */
const char* FFS_GetStatusString(FFS_Status status);

#endif // FLASH_FS_H
