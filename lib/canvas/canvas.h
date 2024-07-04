#pragma once

#include <gps.h>

void tft_init();
void tft_println(const char* text);
void tft_header(const Coord& pos, const int mode);
void tft_footer(const char* msg);
void tft_msg(const char* msg);
void tft_header_msg(const char* msg);
void tft_fill_screen();
void tft_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
void tft_draw_wide_line(float ax, float ay, float bx, float by, float wd, uint32_t fg_color, uint32_t bg_color);
void tft_fill_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);