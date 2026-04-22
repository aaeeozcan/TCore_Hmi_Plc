#include <stdarg.h>
#include <stdio.h>
#include <stm32f1xx_hal.h>

extern UART_HandleTypeDef huart3;

int __io_putchar(int ch) {
    uint8_t byte = (uint8_t)ch;
    HAL_UART_Transmit(&huart3, &byte, 1, 100);
    return ch;
}

void uart_printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (len > 0) {
        HAL_UART_Transmit(&huart3, (uint8_t*)buffer, (uint16_t)len, 500);
    }
}
