#ifndef maps_h_
#define maps_h_
#include <ArduinoJson.h>
#include <stdint.h>
#include <vector>

struct MemMap;
struct Coord;

bool init_sd_card();
void import_map_features( MemMap& mmap);
#endif