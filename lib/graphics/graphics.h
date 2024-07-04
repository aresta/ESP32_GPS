#pragma once

#include <math.h>
#include <vector>
#include <stdint.h>
#include <stdlib.h>

/// @brief Point in 16 bits projected coordinates (x,y) 
struct Point16 {
    Point16()
    {};

    Point16(int16_t x, int16_t y) 
    : x(x), y(y) 
    {};

    Point16(char *coords_pair) // char array like:  11.222,333.44
    {
        char *next;
        x = (int16_t )round( strtod( coords_pair, &next));  // 1st coord // TODO: change by strtol and test
        y = (int16_t )round( strtod( ++next, NULL));  // 2nd coord
    }

    Point16 operator-(const Point16 p)
    {
        return Point16( x-p.x, y-p.y);
    };

    Point16 operator+(const Point16 p)
    { 
        return Point16( x+p.x, y+p.y);
    };

    int16_t x;
    int16_t y;
};

/// @brief Point in 32 bits projected coordinates (x,y) 
struct Point32 {
    Point32()
    {};

    Point32(int32_t x, int32_t y) 
    : x(x),y(y) 
    {};

    Point32(Point16 p)
    : x(p.x), y(p.y) 
    {};

    Point32 operator-(const Point32 p)
    { 
        return Point32( x-p.x, y-p.y);};

    Point32 operator+(const Point32 p)
    {
        return Point32( x+p.x, y+p.y);
    };

    Point16 toPoint16()
    { 
        return Point16(x, y);
    }; // TODO: check limits

    inline bool operator==(const Point32 p)
    {
        return x==p.x && y==p.y; 
    };

    /// @brief Parse char array with the coordinates
    /// @param coords_pair char array like:  11.222,333.44
    int32_t x;
    int32_t y;
};

// @brief Bounding Box
struct BBox {
    BBox() 
    {};

    // @brief Bounding Box
    // @param min top left corner
    // @param max bottom right corner
    BBox( Point32 min, Point32 max)
    : min(min), max(max)
    {};

    BBox operator-(const Point32& point)
    { 
        return BBox(min-point, max-point);
    };

    inline bool containsPoint(const Point32& p)
    { 
        return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y; 
    }

    inline bool intersects(const BBox& b)
    { 
        return !(b.min.x > max.x || b.max.x < min.x || b.min.y > max.y || b.max.y < min.y);
    }

    Point32 min;
    Point32 max;
};

// // @brief Simple two points line 
// struct Line {
//     Line() {};
//     Line( Point16 p1, Point16 p2) : p1(p1), p2(p2) {};
//     Point16 p1;
//     Point16 p2;
//     uint16_t color;
//     uint8_t width;
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

struct ViewPort 
{
    ViewPort(const Point32& location, const uint16_t zoom, const uint16_t w, const uint16_t h) 
    : center(location), zoom_level(zoom), screen_width(w), screen_height(h)
    {}

    inline void setCenter(const Point32& location) {
        center = location;
        bbox.min.x = location.x - screen_width  * zoom_level / 2;
        bbox.min.y = location.y - screen_height * zoom_level / 2;
        bbox.max.x = location.x + screen_width  * zoom_level / 2;
        bbox.max.y = location.y + screen_height * zoom_level / 2;
    }

    Point32 center;
    BBox bbox;

    uint16_t screen_width;
    uint16_t screen_height;
    uint16_t zoom_level;
};