/*
 * PLC Watchdog System
 * Hardware and Software Watchdog Implementation
 * 
 * MCU: STM32F103RCT6
 * Version: 1.0
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// WATCHDOG CONFIGURATION
// =============================================================================

// Hardware Watchdog (IWDG) Configuration
#define IWDG_PRESCALER          IWDG_PRESCALER_64    // 64 prescaler
#define IWDG_RELOAD_VALUE       2500                 // ~4 seconds timeout at 40kHz/64
#define IWDG_WINDOW_DISABLE     0xFFF                // No window mode

// Software Watchdog Configuration
#define SW_WDG_DEFAULT_TIMEOUT_MS   100              // Default cycle timeout: 100ms
#define SW_WDG_MIN_TIMEOUT_MS       10               // Minimum allowed timeout
#define SW_WDG_MAX_TIMEOUT_MS       5000             // Maximum allowed timeout: 5 seconds
#define SW_WDG_WARNING_THRESHOLD    80               // Warning at 80% of timeout

// Watchdog Error Codes
typedef enum {
    WDG_OK = 0,
    WDG_ERROR_INIT_FAILED,
    WDG_ERROR_TIMEOUT,
    WDG_ERROR_CYCLE_OVERRUN,
    WDG_ERROR_INVALID_CONFIG,
    WDG_ERROR_STACK_OVERFLOW,
    WDG_ERROR_MEMORY_CORRUPT
} WatchdogError;

// Watchdog Events for Logging
typedef enum {
    WDG_EVENT_STARTED,
    WDG_EVENT_REFRESHED,
    WDG_EVENT_TIMEOUT_WARNING,
    WDG_EVENT_TIMEOUT_ERROR,
    WDG_EVENT_RESET_PENDING,
    WDG_EVENT_CONFIG_CHANGED,
    WDG_EVENT_DISABLED,
    WDG_EVENT_ENABLED
} WatchdogEvent;

// =============================================================================
// SOFTWARE WATCHDOG STRUCTURE
// =============================================================================

typedef struct {
    // Configuration
    uint32_t timeout_ms;           // Cycle timeout in milliseconds
    uint32_t warning_threshold_ms; // Warning threshold
    bool enabled;                   // Enable/disable flag
    bool auto_reset;               // Auto reset on timeout
    
    // Runtime state
    uint32_t last_refresh_time;    // Last refresh timestamp
    uint32_t cycle_start_time;     // Current cycle start time
    uint32_t max_cycle_time;       // Maximum recorded cycle time
    uint32_t min_cycle_time;       // Minimum recorded cycle time
    uint32_t avg_cycle_time;       // Average cycle time
    
    // Statistics
    uint32_t refresh_count;        // Total refresh count
    uint32_t timeout_count;        // Total timeout count
    uint32_t warning_count;        // Total warning count
    uint32_t reset_count;          // System reset count
    
    // Error tracking
    WatchdogError last_error;      // Last error code
    uint32_t last_error_time;      // Last error timestamp
    
    // Callback functions
    void (*on_timeout)(void);      // Timeout callback
    void (*on_warning)(uint32_t remaining_ms); // Warning callback
    void (*on_refresh)(uint32_t cycle_time);   // Refresh callback
    
} SoftwareWatchdog;

// =============================================================================
// WATCHDOG MANAGER STRUCTURE
// =============================================================================

typedef struct {
    // Hardware watchdog state
    bool hw_wdg_enabled;
    uint32_t hw_wdg_counter;
    
    // Software watchdog
    SoftwareWatchdog sw_wdg;
    
    // System monitoring
    uint32_t stack_watermark;      // Stack usage monitoring
    uint32_t heap_watermark;       // Heap usage monitoring
    
    // Event logging
    WatchdogEvent event_log[32];   // Circular event buffer
    uint8_t event_log_index;
    
    // PLC specific monitoring
    uint32_t plc_cycle_counter;    // PLC cycle counter
    bool plc_running;              // PLC run state
    
} WatchdogManager;

// =============================================================================
// GLOBAL WATCHDOG INSTANCE
// =============================================================================

extern WatchdogManager g_watchdog;

// =============================================================================
// HARDWARE WATCHDOG FUNCTIONS
// =============================================================================

// Initialize hardware watchdog (IWDG)
WatchdogError watchdog_hw_init(void);

// Refresh hardware watchdog
void watchdog_hw_refresh(void);

// Enable/disable hardware watchdog
void watchdog_hw_enable(bool enable);

// Check if system reset was caused by watchdog
bool watchdog_hw_check_reset_source(void);

// =============================================================================
// SOFTWARE WATCHDOG FUNCTIONS
// =============================================================================

// Initialize software watchdog
WatchdogError watchdog_sw_init(uint32_t timeout_ms);

// Refresh software watchdog (call at start of each PLC cycle)
WatchdogError watchdog_sw_refresh(void);

// Check software watchdog status
bool watchdog_sw_check(void);

// Configure software watchdog
WatchdogError watchdog_sw_configure(uint32_t timeout_ms, bool auto_reset);

// Get software watchdog statistics
void watchdog_sw_get_stats(SoftwareWatchdog* stats);

// Reset software watchdog statistics
void watchdog_sw_reset_stats(void);

// =============================================================================
// WATCHDOG MANAGER FUNCTIONS
// =============================================================================

// Initialize complete watchdog system
WatchdogError watchdog_init(bool enable_hw, bool enable_sw, uint32_t sw_timeout_ms);

// Main watchdog service routine (call in main loop or timer interrupt)
void watchdog_service(void);

// Emergency stop - disable all outputs and halt
void watchdog_emergency_stop(void);

// Get system health status
bool watchdog_get_system_health(void);

// Log watchdog event
void watchdog_log_event(WatchdogEvent event);

// Print watchdog status (debug)
void watchdog_print_status(void);

// =============================================================================
// CALLBACK REGISTRATION
// =============================================================================

// Register timeout callback
void watchdog_register_timeout_callback(void (*callback)(void));

// Register warning callback
void watchdog_register_warning_callback(void (*callback)(uint32_t));

// Register refresh callback  
void watchdog_register_refresh_callback(void (*callback)(uint32_t));

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Get current system time in milliseconds
uint32_t watchdog_get_time_ms(void);

// Calculate time difference with overflow handling
uint32_t watchdog_time_diff(uint32_t start, uint32_t end);

// Check stack usage
uint32_t watchdog_check_stack_usage(void);

// Check heap usage
uint32_t watchdog_check_heap_usage(void);

#endif // WATCHDOG_H
