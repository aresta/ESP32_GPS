#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include "gps.h"
#include "colors.h"
#include <stdint.h>

extern TFT_eSPI tft;
extern TFT_eSprite spr;

/// @brief Point in 16 bits projected coordinates (x,y) 
struct Point16 {
  Point16(){};
  Point16( int16_t x, int16_t y) : x(x),y(y) {};
  inline Point16 operator-(const Point16 &p){ return Point16( x-p.x, y-p.y);};
  inline Point16 operator+(const Point16 &p){ return Point16( x+p.x, y+p.y);};
  inline Point16 operator/(uint8_t factor) { return Point16(x / factor, y / factor);}
  Point16( char *coords_pair); // parse char array like:  11.222,333.44
  inline Point16 toScreenCoord( uint8_t zoom, const Point16 &offset) { return (*this / zoom) + offset;}
  int16_t x;
  int16_t y;
};


/// @brief Point in 32 bits projected coordinates (x,y) 
struct Point32 {
  Point32(){};
  Point32( int32_t x, int32_t y) : x(x),y(y) {};
  Point32( Point16 p): x(p.x), y(p.y) {};
  inline Point32 operator-(const Point32 &p){ return Point32( x-p.x, y-p.y);};
  inline Point32 operator+(const Point32 &p){ return Point32( x+p.x, y+p.y);};
  inline Point32 operator/(uint8_t factor) { return Point32(x / factor, y / factor);}
  Point16 toPoint16(){ return Point16( x, y);}; // TODO: check limits
  bool operator==(const Point32 p){ return x==p.x && y==p.y; };

  /// @brief Parse char array with the coordinates
  /// @param coords_pair char array like:  11.222,333.44
  
  int32_t x;
  int32_t y;
};


// @brief Bounding Box
struct BBox {
  BBox() {};
  // @brief Bounding Box
  // @param min top left corner
  // @param max bottim right corner
  BBox( Point32 min, Point32 max): min(min), max(max) {};
  inline BBox operator-(const Point32 p){ return BBox( min-p, max-p);};
  inline bool contains_point(const Point32 p){ return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y; };
  bool intersects(const BBox b);
  Point32 min;
  Point32 max;
};

// // @brief Simple two points line 
// struct Line {
//   Line() {};
//   Line( Point16 p1, Point16 p2) : p1(p1), p2(p2) {};
//   Point16 p1;
//   Point16 p2;
//   uint16_t color;
//   uint8_t width;
// };

// @brief List of points, where line segments are drawn between consecutive points.
struct Polyline {
  std::vector<Point16> points;
  BBox bbox;
  uint16_t color;
  uint8_t width;
  uint8_t maxzoom;
};

// @brief Polygon feature 
struct Polygon {
  std::vector<Point16> points;
  BBox bbox;
  uint16_t color;
  uint8_t maxzoom;
};


struct ViewPort {
  void setCenter(Point32& pcenter);
  Point32 center;
  BBox bbox;
};


/////////////////////  Functions  /////////////////////////

struct MemCache; // forward declarations
struct MapBlock;
struct Coord;

void draw( ViewPort& viewPort, MemCache& memCache);
// std::vector<Point16> clip_polygon( BBox& bbox, std::vector<Point16>&  points);
void stats( ViewPort& viewPort, MapBlock& mblock);
void header_msg( String msg);

void tft_msg( const char *msg);
void refresh_display();

/// @brief Clips a segment against a bbox and returns the intersection point. 
/// You should know in advance which point in inside and which outside.
/// @param p_in Point of the segment that is in the bbox
/// @param p_out Point of the segment that is out of the bbox
/// @param bbox clipping area boudaries
/// @return 
// Point16 clip_segment( Point16 p_in, Point16 p_out, BBox bbox);
