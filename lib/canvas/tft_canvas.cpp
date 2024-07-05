#include <colors.h>
#include <gps.h>

#include "canvas.h"
#include "../conf.h"

#ifdef ARDUINO

#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void tft_init()
{
    digitalWrite( TFT_CS, HIGH); // TFT chip select

    tft.init();
    tft.setRotation(0);  // portrait
    tft.invertDisplay( true);
    tft.fillScreen( CYAN);
    tft.setTextColor(TFT_BLACK);

    digitalWrite(TFT_BLK_PIN, HIGH);

    tft.setCursor(5,5,4);
    tft.println("Initializing...");

    digitalWrite( TFT_BLK_PIN, HIGH);
}

void tft_println(const char* text)
{
    tft.println(text);
}

void tft_header(const Coord& pos, const int mode)
{
    tft.fillRect(0, 0, 240, 25, YELLOWCLEAR);
    tft.setCursor(5,5,2);
    tft.print(pos.lng, 4);
    tft.print(" "); tft.print(pos.lat, 4);
    tft.print(" Sats: "); tft.print(pos.satellites);
    tft.print(" M: "); tft.print(mode);
}

void tft_footer(const char* msg)
{
    tft.fillRect(0, 300, 240, 320, CYAN);
    tft.setCursor(5,305,2);
    tft.println(msg);
}

void tft_msg(const char* msg)
{
    tft.fillRect(0, 0, 240, 25, CYAN);
    tft.setCursor(5,5,2);
    tft.println(msg);
}

void tft_fill_screen()
{
    tft.fillScreen(BACKGROUND_COLOR);
}

void tft_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
{
    tft.drawLine( x0, y0, x1, y1, color);
}

void tft_draw_wide_line(float ax, float ay, float bx, float by, float wd, uint32_t fg_color, uint32_t bg_color)
{
    tft.drawWideLine(ax, ay, bx, by, wd, fg_color, bg_color);
}

void tft_fill_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void tft_header_msg(const char* msg)
{
    tft.fillRect(0, 0, 240, 25, YELLOWCLEAR);
    tft.setCursor(5,5,2);
    tft.print( msg);
}

#endif