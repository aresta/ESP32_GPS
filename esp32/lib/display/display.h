#ifndef display_h_
#define display_h_
#include "features.h"

class TFT_eSPI;

#define SCREEN_WIDTH 240        // tft.width()
#define SCREEN_HEIGHT 320       // tft.height()
#define SCREEN_BUFFER_SIZE 2    // buffer around the displayed area to keep feature in memory
#define PIXEL_SIZE 2 // in meters

const uint16_t WHITE        =   0xFFFF;
const uint16_t BLACK        =   0x0000;
const uint16_t RED          =   0xF800;
const uint16_t GREEN        =   0x07C0;
const uint16_t GREENCLEAR   =   0x9FD3;
const uint16_t GREENCLEAR2  =   0x0540;
const uint16_t BLUE         =   0x227E;
const uint16_t BLUECLEAR    =   0x6D3E;
const uint16_t CYAN         =   0xAA1F;
const uint16_t ORANGE       =   0xF8AA;
const uint16_t GRAY         =   0x94B2;
const uint16_t GRAYCLEAR    =   0xBDD7;
const uint16_t GRAYCLEAR2    =  0xE73C;
const uint16_t BACKGROUND_COLOR = GRAYCLEAR2;

struct ViewPort {
    void setCenter(Point32 pcenter);
    Point32 center;
    BBox bbox;
};

void draw( TFT_eSPI& tft, ViewPort& display, MemMap& features);

#endif