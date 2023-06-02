#include <Arduino.h>
#include <TFT_eSPI.h>
#include "features.h"
#include "display.h"

void ViewPort::setCenter(Point32 pcenter) {
    center = pcenter;
    bbox.min.x = pcenter.x - SCREEN_WIDTH  * PIXEL_SIZE / 2;
    bbox.min.y = pcenter.y - SCREEN_HEIGHT * PIXEL_SIZE / 2;
    bbox.max.x = pcenter.x + SCREEN_WIDTH  * PIXEL_SIZE / 2;
    bbox.max.y = pcenter.y + SCREEN_HEIGHT * PIXEL_SIZE / 2;
}

bool in_viewPort( Point16 p, BBox screen_bbox)
{
    return( p.x > screen_bbox.min.x &&
            p.x < screen_bbox.max.x &&
            p.y > screen_bbox.min.y &&
            p.y < screen_bbox.max.y );
}

Point16 toScreenCoords( Point16 p, Point16 screen_center)
{
    return Point16(
        ((p.x - screen_center.x) / PIXEL_SIZE) + SCREEN_WIDTH / 2,
        ((p.y - screen_center.y) / PIXEL_SIZE) + SCREEN_HEIGHT/ 2
    );
}

void draw( TFT_eSPI tft, ViewPort viewPort, MemMap mmap)
{
    log_d("Polylines: %i", mmap.polylines.size());
    Point32 screen_center_mc = viewPort.center - mmap.features_offset;  // screen center with features coordinates
    BBox screen_bbox_mc = viewPort.bbox - mmap.features_offset;  // screen boundaries with features coordinates
    tft.fillScreen( BACKGROUND_COLOR);
    for( Polyline polyline : mmap.polylines){
        uint16_t pl_color = 
            polyline.color == "white" ? WHITE : 
            polyline.color == "black" ? BLACK : 
            polyline.color == "red" ? RED : 
            polyline.color == "green" ? GREEN : 
            polyline.color == "cyan" ? CYAN : 
            polyline.color == "orange" ? ORANGE : 
            BLACK;
        for( int i=0; i < (polyline.points.size() - 1); i++){
            Point16 p1 = polyline.points[i];
            Point16 p2 = polyline.points[i+1];
            if( !in_viewPort( p1, screen_bbox_mc) && !in_viewPort( p2, screen_bbox_mc)) continue; // out of screen. TODO: could still cut the screen area!
            p1 = toScreenCoords( p1, screen_center_mc);  // TODO: clipping
            p2 = toScreenCoords( p2, screen_center_mc);
            log_v(" %s (%i,%i) (%i,%i) ", polyline.color, p1.x, p1.y, p2.x, p2.y);
            tft.drawWideLine(
                p1.x, SCREEN_HEIGHT - p1.y,
                p2.x, SCREEN_HEIGHT - p2.y,
                // pl_color);  
                polyline.width/PIXEL_SIZE ?: 1, pl_color, pl_color);  
        }
    }
    tft.fillTriangle( 
        SCREEN_WIDTH/2 - 4, SCREEN_HEIGHT/2 + 5, 
        SCREEN_WIDTH/2 + 4, SCREEN_HEIGHT/2 + 5, 
        SCREEN_WIDTH/2,     SCREEN_HEIGHT/2 - 6, 
        RED);
}