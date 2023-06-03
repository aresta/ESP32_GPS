#ifndef features_h_
#define features_h_
#include <Arduino.h>
#include <stdint.h>
#include <vector>


/// @brief Point in 32 bits projected coordinates (x,y) 
struct Point32 {
    Point32(){};
    Point32( int32_t x, int32_t y) : x(x),y(y) {};
    Point32 operator-(const Point32 p){ return Point32( x-p.x, y-p.y);};
    Point32 operator+(const Point32 p){ return Point32( x+p.x, y+p.y);};

    /// @brief Parse char array with the coordinates
    /// @param coords_pair char array like:  11.222,333.44
    Point32( char *coords_pair); // char array like:  11.222,333.44
    
    int32_t x;
    int32_t y;
};


/// @brief Point in 16 bits projected coordinates (x,y) 
struct Point16 {
    Point16(){};
    Point16( int16_t x, int16_t y) : x(x),y(y) {};
    Point16( Point32 p): x(p.x), y(p.y) {};
    // Point16 operator-(const Point16 p){ return Point16( x-p.x, y-p.y);};
    // Point16 operator+(const Point16 p){ return Point16( x+p.x, y+p.y);};
    int16_t x;
    int16_t y;
};


/// @brief Point in geografic (lat,lon) coordinates and other gps data
struct Coord {
    Point32 getPoint32();
    double lat = 0;
    double lng = 0;
    int16_t altitude = 0;
    int16_t direction = 0;
    int16_t satellites = 0;
    bool isValid = false;
    bool isUpdated = false;
};


// struct Line {
//     Point16 p1;
//     Point16 p2;
// };


struct Polyline {
    std::vector<Point16> points;
    String color;
    int16_t width;
    int16_t z_order;
};


struct Polygon {
    std::vector<Point16> points;
    String color;
    int16_t z_order;
};


/// @brief Bounding Box
struct BBox {
    BBox() {};
    /// @brief Bounding Box
    /// @param min top left corner
    /// @param max bottim right corner
    BBox( Point32 min, Point32 max): min(min), max(max) {};
    BBox operator-(const Point32 p){ return BBox( min-p, max-p);};
    bool contains_point(const Point16 p){ return p.x > min.x && p.x < max.x && p.y > min.y && p.y < max.y; }
    Point32 min;
    Point32 max;
};


/// @brief Map Area with all features in memory
struct MemMap {

    /// @brief Map area in real world coordinates.
    BBox bbox;

    /// @brief Features come with coordinates relative to this point, to have smaller values and fit in 16 bits.
    Point32 features_offset;
    
    std::vector<Polyline> polylines;
    std::vector<Polygon> polygons;
    void setBounds( Point32 map_center, int32_t map_width, int32_t map_height);
};

#endif