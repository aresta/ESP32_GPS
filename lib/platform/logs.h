#pragma once

void log_d(const char* format, ...);

void log_e(const char* format, ...);

void log_v(const char* format, ...);

void log_i(const char* format, ...);

unsigned long esp_get_free_heap_size();

long long millis();