#include <colors.h>
#include <graphics.h>
#include <gps.h>
#include <files.h>
#include <maps.h>
#include <canvas.h>

#include "../lib/conf.h"

#include "draw.h"
#ifdef ARDUINO
#include "arduino_app.h"
#else
#include "sdl_app.h"
#endif

MemCache memCache;
Coord coord;
std::vector<Coord> samples;

int zoom_level = PIXEL_SIZE_DEF; // zoom_level = 1 correspond aprox to 1 meter / pixel
int mode = DEVMODE_NAV;

double prev_lat = 500;
double prev_lng = 500;

Point32 map_center(INIT_POS);
ViewPort viewPort(map_center, zoom_level, TFT_WIDTH, TFT_HEIGHT);

const char* base_folder = "/mymap/";
IFileSystem* fileSystem = get_file_system(base_folder);

void Run()
{ 
    tft_init();

    if (!init_file_system()) {
        tft_println("Error: SD Card Mount Failed!");
        while(true);
    }

    tft_println("Reading map...");

    Point32 map_center(INIT_POS);
    // TODO: keep and show last position
    // viewPort.setCenter(map_center);
    // get_map_blocks(fileSystem, viewPort.bbox, memCache );
    // draw(viewPort, memCache);

    tft_msg("Waiting for satellites...");
}