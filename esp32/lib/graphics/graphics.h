#ifndef graphics_h_
#define graphics_h_
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdint.h>
#include <vector>

#define SCREEN_WIDTH 240        // tft.width()
#define SCREEN_HEIGHT 320       // tft.height()
#define SCREEN_BUFFER_SIZE 4    // buffer around the displayed area to keep feature in memory
#define PIXEL_SIZE 2 // in meters

const uint16_t WHITE        =   0xFFFF;
const uint16_t BLACK        =   0x0000;
const uint16_t RED          =   0xFA45;
const uint16_t GREEN        =   0x76EE;
const uint16_t GREENCLEAR   =   0x9F93;
const uint16_t GREENCLEAR2  =   0xCF6E;
const uint16_t BLUE         =   0x227E;
const uint16_t BLUECLEAR    =   0x6D3E;
const uint16_t CYAN         =   0xAA1F;
const uint16_t YELLOW       =   0xFFF1;
const uint16_t ORANGE       =   0xFCC2;
const uint16_t GRAY         =   0x94B2;
const uint16_t GRAYCLEAR    =   0xAD55;
const uint16_t GRAYCLEAR2   =   0xD69A;
const uint16_t BROWN        =   0xAB00;
const uint16_t BACKGROUND_COLOR = 0xEF5D;


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


struct ViewPort {
    void setCenter(Point32 pcenter);
    Point32 center;
    BBox bbox;
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


/////////////////////  Functions  /////////////////////////

uint32_t get_color( String color);
void draw( TFT_eSPI& tft, ViewPort& display, MemMap& features);
Point16 toScreenCoords( Point16 p, Point16 screen_center);
void fill_polygon( TFT_eSPI& tft,  std::vector<Point16> points, int color);
std::vector<Point16> clip_polygon( BBox bbox, std::vector<Point16>  points);
void stats( ViewPort& viewPort, MemMap& mmap);

#endif