#ifndef maps_h_
#define maps_h_
#include <ArduinoJson.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <math.h>
#include "graphics.h"

#define MAPBLOCK_SIZE_BITS 12 
#define MAPFILE_SIZE 16 
const int32_t MAPBLOCK_SIZE = pow( 2, MAPBLOCK_SIZE_BITS); // 4096

bool init_sd_card();
void import_map_features( MemMap& mmap);

/// @brief Map square area of aprox 4096 meters side 
struct MapBlock {
    Point32 offset;
    BBox bbox;
    std::vector<Polyline> polylines;
    std::vector<Polygon> polygons;
};

/// @brief File containing aprox 16x16 MapBlocks 
struct MapFile {
    Point32 offset;
    BBox bbox;
    std::array<MapBlock, MAPFILE_SIZE * MAPFILE_SIZE> mapBlocks;
};


#endif