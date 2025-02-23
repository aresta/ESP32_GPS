#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include "gps.h"
#include "colors.h"
#include <stdint.h>

extern TFT_eSPI tft;

struct Point16; // Forward declaration of Point16

// Templated base class for a point. CRTP-based (Curiously Recurring Template Pattern).
template<typename Derived, typename T>
struct PointBase {
  T x; // either int16_t or int32_t
  T y;
  PointBase() : x(0), y(0) {}
  PointBase(T x, T y): x(x), y(y) {}
  inline Derived operator+(const Derived &p) const { return Derived(x + p.x, y + p.y); }
  inline Derived operator-(const Derived &p) const { return Derived(x - p.x, y - p.y); }
  inline Derived operator/(uint8_t factor) const { return Derived(x / factor, y / factor); }
  inline Derived operator*(uint8_t factor) const { return Derived(x * factor, y * factor); }
  inline bool operator==(const Derived p){ return x==p.x && y==p.y; };
  inline Derived toScreenCoord(uint8_t zoom, const Derived &offset) const { return (*this / zoom) + offset;};
};


/// @brief Point in 16 bits projected coordinates (x,y) 
struct Point16 : public PointBase<Point16, int16_t>{
  using PointBase<Point16, int16_t>::PointBase;
  using PointBase<Point16, int16_t>::operator/; // Ensure operator/ is visible.
  Point16( char *coords_pair);
};


/// @brief Point in 32 bits projected coordinates (x,y) 
struct Point32 : public PointBase<Point32, int32_t>{
  using PointBase<Point32, int32_t>::PointBase;
  Point32(Point16 p) : PointBase<Point32, int32_t>(p.x, p.y) {}
  Point16 toPoint16(){ return Point16( x, y);}; // TODO: check limits
};


// @brief Bounding Box
struct BBox {
  BBox() {};
  // @brief Bounding Box
  // @param min top left corner
  // @param max bottim right corner
  BBox( Point32 min, Point32 max): min(min), max(max) {};
  inline BBox operator-(const Point32 p) const { return BBox( min-p, max-p);};
  inline bool contains_point(const Point32 p) const { return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;};
  inline bool contained_in(const BBox b) const{ return b.contains_point( min) && b.contains_point( max);};
  bool intersects(const BBox b) const;
  Point32 min;
  Point32 max;
};


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


struct ViewArea {
  void setCenter(Point32& pcenter, int16_t width, int16_t eight);
  Point32 center;
  BBox bbox;
};


/////////////////////  Functions  /////////////////////////

struct MemCache; // forward declarations
struct MapBlock;
struct Coord;

void draw( ViewArea& viewArea, MemCache& memCache, TFT_eSprite *spr);
void stats( ViewArea& viewArea, MapBlock& mblock);
void header_msg( String msg);

void tft_msg( const char *msg);
void refresh_display();


