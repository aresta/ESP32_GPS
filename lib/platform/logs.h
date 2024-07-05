#pragma once

#include <stdio.h>
#include <stdarg.h>

inline void log_d(const char* format, ...) {
    va_list vl;
    va_start(vl, format);
        // Printable.c = va_arg( vl, char );
        // printf_s( "%c\n", Printable.c );
    va_end(vl);
}

inline void log_e(const char* format, ...) {
    va_list vl;
    va_start(vl, format);
        // Printable.c = va_arg( vl, char );
        // printf_s( "%c\n", Printable.c );
    va_end(vl);
}

inline void log_v(const char* format, ...) {
    va_list vl;
    va_start(vl, format);
        // Printable.c = va_arg( vl, char );
        // printf_s( "%c\n", Printable.c );
    va_end(vl);
}

inline void log_i(const char* format, ...) {
    va_list vl;
    va_start(vl, format);
        // Printable.c = va_arg( vl, char );
        // printf_s( "%c\n", Printable.c );
    va_end(vl);
}

inline unsigned long esp_get_free_heap_size()
{
    return 0; // TODO
} 

inline unsigned long millis()
{
    return 0; // TODO
}