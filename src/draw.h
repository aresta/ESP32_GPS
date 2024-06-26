#pragma once

#include <graphics.h>
#include <maps.h>

void draw(ViewPort& viewPort, MemCache& memCache);

// std::vector<Point16> clip_polygon( BBox& bbox, std::vector<Point16>&  points);

void stats(ViewPort& viewPort, MapBlock& mblock);