#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdint.h>
#include "graphics.h"
#include "../conf.h"


void ViewPort::setCenter(Point32& pcenter) {
    center = pcenter;
    bbox.min.x = pcenter.x - SCREEN_WIDTH  * zoom_level / 2;
    bbox.min.y = pcenter.y - SCREEN_HEIGHT * zoom_level / 2;
    bbox.max.x = pcenter.x + SCREEN_WIDTH  * zoom_level / 2;
    bbox.max.y = pcenter.y + SCREEN_HEIGHT * zoom_level / 2;
}

Point16 toScreenCoords( Point32 p, Point32 screen_center)
{
    return Point16(
        ((p.x - screen_center.x) / zoom_level) + SCREEN_WIDTH / 2,
        ((p.y - screen_center.y) / zoom_level) + SCREEN_HEIGHT/ 2
    );
}


std::vector<Point16> clip_polygon( BBox& bbox, std::vector<Point16>& points)
{
    std::vector<Point16> clipped;
    int16_t dx, dy, bbpx;
    for( int i=0; i < (points.size() - 1); i++){
        Point16 p1 = points[i];
        Point16 p2 = points[i+1];
        // cut vert left side
        if( p1.x < bbox.min.x && p2.x > bbox.min.x) {
            dx = p2.x - p1.x;
            dy = abs( p2.y - p1.y);
            bbpx = bbox.min.x - p1.x;
            assert( dx > 0); assert( dy > 0); assert( bbpx > 0);
            p1.y = double(bbpx * dy) / dx;
            p1.x = bbox.min.x;
        }

        if( p1.x > bbox.min.x && p2.x < bbox.min.x) {

        }

    }
    return clipped;
}

Point16::Point16( char *coords_pair)
{
    char *next;
    x = (int16_t )round( strtod( coords_pair, &next));  // 1st coord // TODO: change by strtol and test
    y = (int16_t )round( strtod( ++next, NULL));  // 2nd coord
}

bool BBox::contains_point(const Point32 p){ return p.x > min.x && p.x < max.x && p.y > min.y && p.y < max.y; }

bool BBox::intersects( BBox b){ return 
    b.contains_point( max) || b.contains_point( min) ||
    b.contains_point( Point16(min.x, max.y)) || b.contains_point( Point16(max.x, min.y)) ||
    contains_point( b.min) || contains_point( b.max) ||
    contains_point( Point16( b.min.x, b.max.y)) || contains_point( Point16(b.max.x, b.min.y));
    }

void header_msg( String msg)
{
    tft.fillRect(0, 0, 240, 25, YELLOWCLEAR);
    tft.setCursor(5,5,2);
    tft.print( msg);
}

