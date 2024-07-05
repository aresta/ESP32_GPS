#include <colors.h>
#include <gps.h>

#include "canvas.h"
#include "../conf.h"

#ifdef ARDUINO

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

#else

#include <SDL2/SDL.h>

extern SDL_Renderer* _sdl;

#define RED(r)   (uint8_t)(((r) & 0xf800) >> 11)
#define GREEN(g) (uint8_t)(((g) & 0x07e0) >> 5)
#define BLUE(b)  (uint8_t)((b) & 0x001f)

inline int SDL_SetRenderDrawColor(SDL_Renderer* render, uint16_t color565)
{
    return SDL_SetRenderDrawColor(
        render, 
        RED(color565),
        GREEN(color565),
        BLUE(color565),
        0); //SDL_ALPHA_OPAQUE
}

void tft_init()
{
// TODO
}

void tft_println(const char* text)
{
// TODO
}

void tft_header(const Coord& pos, const int mode)
{
    SDL_Rect rect { 0, 0, 240, 25 };

    SDL_SetRenderDrawColor(_sdl, YELLOWCLEAR);
    SDL_RenderFillRect(_sdl, &rect);

    // tft.fillRect(0, 0, 240, 25, YELLOWCLEAR);
    // tft.setCursor(5,5,2);
    // tft.print(pos.lng, 4);
    // tft.print(" "); tft.print(pos.lat, 4);
    // tft.print(" Sats: "); tft.print(pos.satellites);
    // tft.print(" M: "); tft.print(mode);
}

void tft_footer(const char* msg)
{
    SDL_Rect rect { 0, 300, 240, 320};

    SDL_SetRenderDrawColor(_sdl, CYAN);
    SDL_RenderFillRect(_sdl, &rect);

    // tft.fillRect(0, 300, 240, 320, CYAN);
    // tft.setCursor(5,305,2);
    // tft.println(msg);
}

void tft_msg(const char* msg)
{
// TODO
}

void tft_fill_screen()
{
    SDL_SetRenderDrawColor(_sdl, BACKGROUND_COLOR);
	SDL_RenderClear(_sdl);
}

void tft_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
{
    SDL_SetRenderDrawColor(_sdl, color);
    auto res = SDL_RenderDrawLine(_sdl, x0, y0, x1, y1);
}

void tft_draw_wide_line(float ax, float ay, float bx, float by, float wd, uint32_t fg_color, uint32_t bg_color)
{
    SDL_SetRenderDrawColor(_sdl, fg_color);
    auto res = SDL_RenderDrawLine(_sdl, ax, ay, bx, by);
}

void tft_fill_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    SDL_SetRenderDrawColor(_sdl, color);

    const SDL_Color sdl_color 
    { 
        RED(color),
        GREEN(color),
        BLUE(color),
     };

    const std::vector<SDL_Vertex> verts =
    {
        { SDL_FPoint{ (float)x0, (float)y0 }, sdl_color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ (float)x1, (float)y1 }, sdl_color, SDL_FPoint{ 0 }, },
        { SDL_FPoint{ (float)x2, (float)y2 }, sdl_color, SDL_FPoint{ 0 }, },
    };

   auto res = SDL_RenderGeometry(_sdl, nullptr, verts.data(), verts.size(), nullptr, 0);
}

void tft_header_msg(const char* msg)
{
    SDL_Rect rect { 0, 0, 240, 25 };

    SDL_SetRenderDrawColor(_sdl, YELLOWCLEAR);
    SDL_RenderFillRect(_sdl, &rect);

    // tft.fillRect(0, 0, 240, 25, YELLOWCLEAR);
    // tft.setCursor(5,5,2);
    // tft.print(msg);
}

#endif