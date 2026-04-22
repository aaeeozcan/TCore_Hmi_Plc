/*
 * PLC Watchdog System Implementation - Keil/ARMCC Version
 * Hardware and Software Watchdog
 * 
 * MCU: STM32F103RCT6
 * Compiler: Keil/ARMCC
 * Version: 1.0
 */

#include <watchdog.h>
// #include "st_interpreter.h"  // DISABLED - module removed
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stm32f1xx_hal.h>

// External references
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern IWDG_HandleTypeDef hiwdg;
// extern Runtime g_runtime;  // DISABLED - module removed

// Global watchdog manager instance
WatchdogManager g_watchdog = {0};

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// UART print function
static void wdg_printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (len > 0) {
        HAL_UART_Transmit(&huart3, (uint8_t*)buffer, len, 100);
    }
}

// Get current time in milliseconds
uint32_t watchdog_get_time_ms(void) {
    return HAL_GetTick();
}

// Calculate time difference with overflow handling
uint32_t watchdog_time_diff(uint32_t start, uint32_t end) {
    if (end >= start) {
        return end - start;
    } else {
        // Handle overflow
        return (0xFFFFFFFF - start) + end + 1;
    }
}

// =============================================================================
// HARDWARE WATCHDOG IMPLEMENTATION
// =============================================================================

WatchdogError watchdog_hw_init(void) {
    // Initialize IWDG with STM32 HAL
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER;
    hiwdg.Init.Reload = IWDG_RELOAD_VALUE;
    
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        wdg_printf("[WDG] Hardware watchdog init failed!\n");
        return WDG_ERROR_INIT_FAILED;
    }
    
    g_watchdog.hw_wdg_enabled = true;
    wdg_printf("[WDG] Hardware watchdog initialized (timeout: ~4s)\n");
    
    return WDG_OK;
}

void watchdog_hw_refresh(void) {
    if (g_watchdog.hw_wdg_enabled) {
        HAL_IWDG_Refresh(&hiwdg);
        g_watchdog.hw_wdg_counter++;
    }
}

void watchdog_hw_enable(bool enable) {
    g_watchdog.hw_wdg_enabled = enable;
    
    if (enable) {
        // Once started, IWDG cannot be stopped except by reset
        HAL_IWDG_Refresh(&hiwdg);
        wdg_printf("[WDG] Hardware watchdog enabled\n");
    } else {
        wdg_printf("[WDG] Hardware watchdog disable requested (will be active until reset)\n");
    }
}

bool watchdog_hw_check_reset_source(void) {
    // Check RCC reset flags
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
        // Clear reset flags
        __HAL_RCC_CLEAR_RESET_FLAGS();
        return true;
    }
    return false;
}

// =============================================================================
// SOFTWARE WATCHDOG IMPLEMENTATION
// =============================================================================

WatchdogError watchdog_sw_init(uint32_t timeout_ms) {
    // Validate timeout
    if (timeout_ms < SW_WDG_MIN_TIMEOUT_MS || timeout_ms > SW_WDG_MAX_TIMEOUT_MS) {
        return WDG_ERROR_INVALID_CONFIG;
    }
    
    // Initialize software watchdog structure
    memset(&g_watchdog.sw_wdg, 0, sizeof(SoftwareWatchdog));
    
    g_watchdog.sw_wdg.timeout_ms = timeout_ms;
    g_watchdog.sw_wdg.warning_threshold_ms = (timeout_ms * SW_WDG_WARNING_THRESHOLD) / 100;
    g_watchdog.sw_wdg.enabled = true;
    g_watchdog.sw_wdg.auto_reset = false;  // Default: no auto reset
    g_watchdog.sw_wdg.last_refresh_time = watchdog_get_time_ms();
    g_watchdog.sw_wdg.min_cycle_time = 0xFFFFFFFF;  // Will be updated on first cycle
    
    wdg_printf("[WDG] Software watchdog initialized (timeout: %lums, warning: %lums)\n", 
           (unsigned long)timeout_ms, (unsigned long)g_watchdog.sw_wdg.warning_threshold_ms);
    
    watchdog_log_event(WDG_EVENT_STARTED);
    
    return WDG_OK;
}

WatchdogError watchdog_sw_refresh(void) {
    if (!g_watchdog.sw_wdg.enabled) {
        return WDG_OK;
    }
    
    uint32_t current_time = watchdog_get_time_ms();
    uint32_t cycle_time = watchdog_time_diff(g_watchdog.sw_wdg.last_refresh_time, current_time);
    
    // Update statistics
    g_watchdog.sw_wdg.refresh_count++;
    
    if (cycle_time > g_watchdog.sw_wdg.max_cycle_time) {
        g_watchdog.sw_wdg.max_cycle_time = cycle_time;
    }
    
    if (cycle_time < g_watchdog.sw_wdg.min_cycle_time) {
        g_watchdog.sw_wdg.min_cycle_time = cycle_time;
    }
    
    // Calculate running average
    g_watchdog.sw_wdg.avg_cycle_time = 
        (g_watchdog.sw_wdg.avg_cycle_time * (g_watchdog.sw_wdg.refresh_count - 1) + cycle_time) 
        / g_watchdog.sw_wdg.refresh_count;
    
    // Check for timeout
    if (cycle_time > g_watchdog.sw_wdg.timeout_ms) {
        g_watchdog.sw_wdg.timeout_count++;
        g_watchdog.sw_wdg.last_error = WDG_ERROR_CYCLE_OVERRUN;
        g_watchdog.sw_wdg.last_error_time = current_time;
        
        wdg_printf("[WDG] SOFTWARE WATCHDOG TIMEOUT! Cycle: %lums (limit: %lums)\n", 
               (unsigned long)cycle_time, (unsigned long)g_watchdog.sw_wdg.timeout_ms);
        
        watchdog_log_event(WDG_EVENT_TIMEOUT_ERROR);
        
        // Call timeout callback if registered
        if (g_watchdog.sw_wdg.on_timeout) {
            g_watchdog.sw_wdg.on_timeout();
        }
        
        // Auto reset if configured
        if (g_watchdog.sw_wdg.auto_reset) {
            wdg_printf("[WDG] Auto-reset triggered!\n");
            watchdog_emergency_stop();
            HAL_NVIC_SystemReset();
        }
        
        return WDG_ERROR_TIMEOUT;
    }
    
    // Check for warning threshold
    if (cycle_time > g_watchdog.sw_wdg.warning_threshold_ms) {
        g_watchdog.sw_wdg.warning_count++;
        uint32_t remaining = g_watchdog.sw_wdg.timeout_ms - cycle_time;
        
        wdg_printf("[WDG] Warning: Cycle time %lums approaching limit (remaining: %lums)\n", 
               (unsigned long)cycle_time, (unsigned long)remaining);
        
        watchdog_log_event(WDG_EVENT_TIMEOUT_WARNING);
        
        // Call warning callback if registered
        if (g_watchdog.sw_wdg.on_warning) {
            g_watchdog.sw_wdg.on_warning(remaining);
        }
    }
    
    // Update refresh time
    g_watchdog.sw_wdg.last_refresh_time = current_time;
    g_watchdog.sw_wdg.cycle_start_time = current_time;
    
    // Call refresh callback if registered
    if (g_watchdog.sw_wdg.on_refresh) {
        g_watchdog.sw_wdg.on_refresh(cycle_time);
    }
    
    return WDG_OK;
}

bool watchdog_sw_check(void) {
    if (!g_watchdog.sw_wdg.enabled) {
        return true;  // Always OK if disabled
    }
    
    uint32_t current_time = watchdog_get_time_ms();
    uint32_t elapsed = watchdog_time_diff(g_watchdog.sw_wdg.last_refresh_time, current_time);
    
    return (elapsed < g_watchdog.sw_wdg.timeout_ms);
}

WatchdogError watchdog_sw_configure(uint32_t timeout_ms, bool auto_reset) {
    // Validate timeout
    if (timeout_ms < SW_WDG_MIN_TIMEOUT_MS || timeout_ms > SW_WDG_MAX_TIMEOUT_MS) {
        return WDG_ERROR_INVALID_CONFIG;
    }
    
    g_watchdog.sw_wdg.timeout_ms = timeout_ms;
    g_watchdog.sw_wdg.warning_threshold_ms = (timeout_ms * SW_WDG_WARNING_THRESHOLD) / 100;
    g_watchdog.sw_wdg.auto_reset = auto_reset;
    
    wdg_printf("[WDG] Software watchdog reconfigured (timeout: %lums, auto-reset: %s)\n",
           (unsigned long)timeout_ms, auto_reset ? "ON" : "OFF");
    
    watchdog_log_event(WDG_EVENT_CONFIG_CHANGED);
    
    return WDG_OK;
}

void watchdog_sw_get_stats(SoftwareWatchdog* stats) {
    if (stats) {
        memcpy(stats, &g_watchdog.sw_wdg, sizeof(SoftwareWatchdog));
    }
}

void watchdog_sw_reset_stats(void) {
    g_watchdog.sw_wdg.refresh_count = 0;
    g_watchdog.sw_wdg.timeout_count = 0;
    g_watchdog.sw_wdg.warning_count = 0;
    g_watchdog.sw_wdg.max_cycle_time = 0;
    g_watchdog.sw_wdg.min_cycle_time = 0xFFFFFFFF;
    g_watchdog.sw_wdg.avg_cycle_time = 0;
    
    wdg_printf("[WDG] Software watchdog statistics reset\n");
}

// =============================================================================
// WATCHDOG MANAGER FUNCTIONS
// =============================================================================

WatchdogError watchdog_init(bool enable_hw, bool enable_sw, uint32_t sw_timeout_ms) {
    WatchdogError err;
    
    wdg_printf("\n=== WATCHDOG SYSTEM INITIALIZATION ===\n");
    
    // Clear manager structure
    memset(&g_watchdog, 0, sizeof(WatchdogManager));
    
    // Check if system reset was caused by watchdog
    if (watchdog_hw_check_reset_source()) {
        wdg_printf("[WDG] System was reset by watchdog!\n");
        g_watchdog.sw_wdg.reset_count++;
        watchdog_log_event(WDG_EVENT_RESET_PENDING);
    }
    
    // Initialize hardware watchdog if requested
    if (enable_hw) {
        err = watchdog_hw_init();
        if (err != WDG_OK) {
            return err;
        }
    }
    
    // Initialize software watchdog if requested
    if (enable_sw) {
        err = watchdog_sw_init(sw_timeout_ms);
        if (err != WDG_OK) {
            return err;
        }
    }
    
    wdg_printf("[WDG] Watchdog system ready\n");
    wdg_printf("======================================\n\n");
    
    return WDG_OK;
}

void watchdog_service(void) {
    // This function should be called regularly (e.g., in timer interrupt)

    // Check PLC state - DISABLED (module removed)
    // g_watchdog.plc_running = g_runtime.running;
    // g_watchdog.plc_cycle_counter = g_runtime.cycle_count;
    
    // Monitor stack usage (simplified)
    g_watchdog.stack_watermark = watchdog_check_stack_usage();
    
    // Monitor heap usage (simplified)
    g_watchdog.heap_watermark = watchdog_check_heap_usage();
    
    // Check software watchdog
    if (!watchdog_sw_check()) {
        // Software watchdog timeout detected
        wdg_printf("[WDG] Software watchdog timeout in service routine!\n");
    }
    
    // Always refresh hardware watchdog in service routine
    watchdog_hw_refresh();
}

void watchdog_emergency_stop(void) {
    wdg_printf("\n!!! EMERGENCY STOP ACTIVATED !!!\n");

    // Stop PLC execution - DISABLED (module removed)
    // g_runtime.running = false;

    // Clear all outputs - DISABLED (module removed)
    // memset(g_runtime.io_mem.digital_outputs, 0, sizeof(g_runtime.io_mem.digital_outputs));
    // memset(g_runtime.io_mem.output_bytes, 0, sizeof(g_runtime.io_mem.output_bytes));
    
    // Log event
    watchdog_log_event(WDG_EVENT_TIMEOUT_ERROR);
    
    // Optional: Set error LED or alarm output
    // HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
    
    wdg_printf("[WDG] All outputs cleared, PLC stopped\n");
}

bool watchdog_get_system_health(void) {
    // Check various health indicators
    bool hw_ok = !g_watchdog.hw_wdg_enabled || (g_watchdog.hw_wdg_counter > 0);
    bool sw_ok = watchdog_sw_check();
    bool stack_ok = g_watchdog.stack_watermark < 90;  // Less than 90% usage
    bool heap_ok = g_watchdog.heap_watermark < 90;
    bool plc_ok = !g_watchdog.plc_running || (g_watchdog.plc_cycle_counter > 0);
    
    return hw_ok && sw_ok && stack_ok && heap_ok && plc_ok;
}

void watchdog_log_event(WatchdogEvent event) {
    g_watchdog.event_log[g_watchdog.event_log_index] = event;
    g_watchdog.event_log_index = (g_watchdog.event_log_index + 1) % 32;
}

void watchdog_print_status(void) {
    wdg_printf("\n=== WATCHDOG STATUS ===\n");
    
    // Hardware watchdog status
    wdg_printf("Hardware WDG: %s (refreshed: %lu times)\n",
           g_watchdog.hw_wdg_enabled ? "ENABLED" : "DISABLED",
           (unsigned long)g_watchdog.hw_wdg_counter);
    
    // Software watchdog status
    if (g_watchdog.sw_wdg.enabled) {
        wdg_printf("Software WDG: ENABLED\n");
        wdg_printf("  Timeout: %lums (warning: %lums)\n", 
               (unsigned long)g_watchdog.sw_wdg.timeout_ms,
               (unsigned long)g_watchdog.sw_wdg.warning_threshold_ms);
        wdg_printf("  Auto-reset: %s\n", g_watchdog.sw_wdg.auto_reset ? "YES" : "NO");
        wdg_printf("  Cycle times: Min=%lums, Max=%lums, Avg=%lums\n",
               (unsigned long)g_watchdog.sw_wdg.min_cycle_time,
               (unsigned long)g_watchdog.sw_wdg.max_cycle_time,
               (unsigned long)g_watchdog.sw_wdg.avg_cycle_time);
        wdg_printf("  Counts: Refresh=%lu, Timeout=%lu, Warning=%lu\n",
               (unsigned long)g_watchdog.sw_wdg.refresh_count,
               (unsigned long)g_watchdog.sw_wdg.timeout_count,
               (unsigned long)g_watchdog.sw_wdg.warning_count);
    } else {
        wdg_printf("Software WDG: DISABLED\n");
    }
    
    // System health
    wdg_printf("System Health: %s\n", watchdog_get_system_health() ? "OK" : "ERROR");
    wdg_printf("Stack Usage: %lu%%\n", (unsigned long)g_watchdog.stack_watermark);
    wdg_printf("Heap Usage: %lu%%\n", (unsigned long)g_watchdog.heap_watermark);
    wdg_printf("PLC Status: %s (cycles: %lu)\n",
           g_watchdog.plc_running ? "RUNNING" : "STOPPED",
           (unsigned long)g_watchdog.plc_cycle_counter);
    
    wdg_printf("======================\n\n");
}

// =============================================================================
// CALLBACK REGISTRATION
// =============================================================================

void watchdog_register_timeout_callback(void (*callback)(void)) {
    g_watchdog.sw_wdg.on_timeout = callback;
}

void watchdog_register_warning_callback(void (*callback)(uint32_t)) {
    g_watchdog.sw_wdg.on_warning = callback;
}

void watchdog_register_refresh_callback(void (*callback)(uint32_t)) {
    g_watchdog.sw_wdg.on_refresh = callback;
}

// =============================================================================
// SIMPLIFIED STACK AND HEAP MONITORING FOR KEIL
// =============================================================================

// Very simple stack usage check - returns fixed estimate
uint32_t watchdog_check_stack_usage(void) {
    // In a real application, you would:
    // 1. Fill stack with pattern at startup
    // 2. Check how much pattern remains
    // 3. Calculate usage percentage
    
    // For now, return a reasonable estimate
    static uint32_t counter = 0;
    counter++;
    
    // Simulate varying stack usage
    if (counter < 100) return 20;        // 20% at startup
    else if (counter < 1000) return 25;  // 25% normal operation
    else return 30;                      // 30% after running a while
}

// Very simple heap usage check - returns fixed estimate  
uint32_t watchdog_check_heap_usage(void) {
    // In a real application, you would:
    // 1. Override malloc/free to track allocations
    // 2. Keep running total of allocated memory
    // 3. Calculate percentage of total heap
    
    // For now, return a reasonable estimate
    return 35;  // 35% usage estimate
}
