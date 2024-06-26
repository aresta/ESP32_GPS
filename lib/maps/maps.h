#ifndef maps_h_
#define maps_h_

#include <stdint.h>
#include <vector>
#include <array>
#include <math.h>
#include <map>

#include "files.h"
#include "graphics.h"

#define MAPBLOCKS_MAX 6         // max blocks in memory
#define MAPBLOCK_SIZE_BITS 12   // 4096 x 4096 coords (~meters) per block  
#define MAPFOLDER_SIZE_BITS 4   // 16 x 16 map blocks per folder

const int32_t MAPBLOCK_MASK  = pow( 2, MAPBLOCK_SIZE_BITS) - 1;     // ...00000000111111111111
const int32_t MAPFOLDER_MASK = pow( 2, MAPFOLDER_SIZE_BITS) - 1;    // ...00001111

/// @brief Map square area of aprox 4096 meters side. Correspond to one single map file.
struct MapBlock {
    Point32 offset;
    // BBox bbox;
    bool inView = false;
    std::vector<Polyline> polylines;
    std::vector<Polygon> polygons;
};

/// @brief MapBlocks stored in memory
struct MemCache {
    std::vector<MapBlock* > blocks;
    // uint32_t total_polylines = 0;
    // uint32_t total_polygons = 0;
    // uint32_t total_inview_polylines = 0;
    // uint32_t total_inview_polygons = 0;
};

void get_map_blocks(const IFileSystem* fileSystem, BBox& bbox, MemCache& memCache);

#endif