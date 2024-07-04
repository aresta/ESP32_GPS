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

inline unsigned long millis()
{
    return 0;
}