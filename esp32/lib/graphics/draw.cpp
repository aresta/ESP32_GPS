#include <Arduino.h>
#include <TFT_eSPI.h>
#include "graphics.h"



void draw( TFT_eSPI& tft, ViewPort& viewPort, MemMap& mmap)
{
    log_d("Polylines: %i Polygons: %i", mmap.polylines.size(), mmap.polygons.size());
    Point32 screen_center_mc = viewPort.center - mmap.features_offset;  // screen center with features coordinates
    BBox screen_bbox_mc = viewPort.bbox - mmap.features_offset;  // screen boundaries with features coordinates
    tft.fillScreen( BACKGROUND_COLOR);

    ////// Polygons 
    for( Polygon polygon : mmap.polygons){
        uint16_t pl_color = get_color( polygon.color);
        if( pl_color == TFT_YELLOW) log_d("color: %s", polygon.color);
        std::vector<Point16> points2;
        bool hit = false;
        for( Point16 p : polygon.points){
            if( screen_bbox_mc.contains_point( p)) hit = true;
            points2.push_back( toScreenCoords( p, screen_center_mc));
            log_v("polygon: %s (%i,%i)", polygon.color, p.x, p.y);
        }
        if( hit) fill_polygon( tft, points2, pl_color);
    }
    
    ////// Lines 
    for( Polyline polyline : mmap.polylines){
        uint16_t pl_color = get_color( polyline.color);        
        for( int i=0; i < (polyline.points.size() - 1); i++) {
            Point16 p1 = polyline.points[i];
            Point16 p2 = polyline.points[i+1];
            if( !screen_bbox_mc.contains_point( p1) && !screen_bbox_mc.contains_point( p2)) continue; // TODO: could still cut the screen area!
            p1 = toScreenCoords( p1, screen_center_mc);  // TODO: clipping
            p2 = toScreenCoords( p2, screen_center_mc);
            log_v(" %s (%i,%i) (%i,%i) ", polyline.color, p1.x, p1.y, p2.x, p2.y);
            tft.drawWideLine(
                p1.x, SCREEN_HEIGHT - p1.y,
                p2.x, SCREEN_HEIGHT - p2.y,
                polyline.width/PIXEL_SIZE ?: 1, pl_color, pl_color);  
        }
    }

    tft.fillTriangle( 
        SCREEN_WIDTH/2 - 4, SCREEN_HEIGHT/2 + 5, 
        SCREEN_WIDTH/2 + 4, SCREEN_HEIGHT/2 + 5, 
        SCREEN_WIDTH/2,     SCREEN_HEIGHT/2 - 6, 
        RED);
}

void stats( ViewPort& viewPort, MemMap& mmap)
{
    Point32 screen_center_mc = viewPort.center - mmap.features_offset;  // screen center with features coordinates
    BBox screen_bbox_mc = viewPort.bbox - mmap.features_offset;  // screen boundaries with features coordinates
    BBox map_bbox_mc = mmap.bbox - mmap.features_offset;  // screen boundaries with features coordinates

    ////// Polygons 
    int in_screen = 0, in_map = 0,  points_total = 0;
    for( Polygon polygon : mmap.polygons){
        bool hit = false;
        for( Point16 p : polygon.points){
            points_total++;
            if( screen_bbox_mc.contains_point( p)) in_screen++;
            if( map_bbox_mc.contains_point( p)) in_map++;
        }
    }
    log_i("Polygons points.  in_screen: %i, in_map: %i,  total: %i", in_screen, in_map, points_total);
    
    ////// Lines 
    in_screen = 0;
    in_map = 0;
    points_total = 0;
    for( Polyline polyline : mmap.polylines){
        for( Point16 p : polyline.points){
            points_total++;
            if( screen_bbox_mc.contains_point( p)) in_screen++;
            if( map_bbox_mc.contains_point( p)) in_map++;
        }
    }
    log_i("Lines points. in_screen: %i,  in_map: %i,  total: %i", in_screen, in_map, points_total);
}