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

int16_t toScreenCoord( const int32_t pxy, const int32_t screen_centerxy) // work with primitives for performance
{
    return round((double )(pxy - screen_centerxy) / zoom_level) + (double )SCREEN_WIDTH / 2;
}


Point16::Point16( char *coords_pair)
{
    char *next;
    x = (int16_t )round( strtod( coords_pair, &next));  // 1st coord // TODO: change by strtol and test
    y = (int16_t )round( strtod( ++next, NULL));  // 2nd coord
}

bool BBox::contains_point(const Point32 p){ return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y; }

bool BBox::intersects( BBox b){ 
    if( b.min.x > max.x || 
        b.max.x < min.x || 
        b.min.y > max.y || 
        b.max.y < min.y) return false;
        return true;
    }

void header_msg( String msg)
{
    tft.fillRect(0, 0, 240, 25, YELLOWCLEAR);
    tft.setCursor(5,5,2);
    tft.print( msg);
}

