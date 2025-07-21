#pragma once

#include <stdint.h>
#include <vector>
#include <array>
#include <math.h>
#include <map>
#include "graphics.h"

#define MAPBLOCKS_MAX 40        // max blocks in memory
#define MIN_FREE_HEAP 500*1000  // min heap free space in bytes, below this value it will release map blocks
#define MAPBLOCK_SIZE_BITS 12   // 4096 x 4096 coords (~meters) per block  
#define MAPFOLDER_SIZE_BITS 4   // 16 x 16 map blocks per folder
#define MAPBLOCK_SIDE_SIZE (1 << MAPBLOCK_SIZE_BITS)   // 4096   
const int32_t MAPBLOCK_MASK  = pow( 2, MAPBLOCK_SIZE_BITS) - 1;   // ...00000000111111111111
const int32_t MAPFOLDER_MASK = pow( 2, MAPFOLDER_SIZE_BITS) - 1;  // ...00001111


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

bool init_sd_card();
void get_map_blocks( BBox& bufferArea, MemCache& memCache);
