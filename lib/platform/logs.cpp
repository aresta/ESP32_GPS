#include <chrono>
#include <stdio.h>
#include <stdarg.h>

#include "logs.h"

void log_d(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf(format, args);
    va_end(args);
}

void log_e(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf(format, args);
    va_end(args);
}

void log_v(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf(format, args);
    va_end(args);
}

void log_i(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf(format, args);
    va_end(args);
}

unsigned long esp_get_free_heap_size()
{
    return 0;
} 

long long millis()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    return milliseconds;
}