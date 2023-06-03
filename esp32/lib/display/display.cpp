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

Point16 toScreenCoords( Point16 p, Point16 screen_center)
{
    return Point16(
        ((p.x - screen_center.x) / PIXEL_SIZE) + SCREEN_WIDTH / 2,
        ((p.y - screen_center.y) / PIXEL_SIZE) + SCREEN_HEIGHT/ 2
    );
}

uint32_t get_color( String color)
{
    return  
            color == "white"        ? WHITE :  // TODO: rework
            color == "black"        ? BLACK : 
            color == "gray"         ? GRAY : 
            color == "grayclear"    ? GRAYCLEAR : 
            color == "grayclear2"   ? GRAYCLEAR : 
            color == "red"          ? RED : 
            color == "green"        ? GREEN : 
            color == "greenclear"   ? GREENCLEAR : 
            color == "greenclear2"  ? GREENCLEAR2 : 
            color == "blue"         ? BLUE : 
            color == "blueclear"    ? BLUECLEAR : 
            color == "cyan"         ? CYAN : 
            color == "orange"       ? ORANGE : 
            color == "yellow"       ? TFT_YELLOW: 
            color == "pink"         ? TFT_PINK: 
            TFT_YELLOW;
}

void fill_polygon( TFT_eSPI& tft,  std::vector<Point16> points, int color) // scanline fill algorithm
{
    int16_t maxy = INT16_MIN, miny = INT16_MAX;

    for( Point16 p : points) { // TODO: precalculate at map file creation
        maxy = max( maxy, p.y);
        miny = min( miny, p.y);
    }
    // log_d("miny: %i, maxy: %i", miny, maxy);
    int16_t nodeX[points.size()], pixelY, swap;

    //  Loop through the rows of the image.
    for( pixelY=miny; pixelY < maxy; pixelY++) {
        //  Build a list of nodes.
        int16_t nodes=0;
        for( int i=0; i < (points.size() - 1); i++) {
            if( (points[i].y < pixelY && points[i+1].y >= pixelY) ||
                (points[i].y >= pixelY && points[i+1].y < pixelY)) {
                    nodeX[nodes++] = 
                        points[i].x + double(pixelY-points[i].y)/double(points[i+1].y-points[i].y) * 
                        double(points[i+1].x-points[i].x);
                }
        }
        // log_d("pixelY: %i, nodes: %i", pixelY, nodes);

        //  Sort the nodes, via a simple “Bubble” sort.
        int i=0;
        while( i < nodes-1) {   // TODO: rework
            if (nodeX[i]>nodeX[i+1]) {
                swap=nodeX[i]; nodeX[i]=nodeX[i+1]; nodeX[i+1]=swap; 
                i=0;  
            }
            else { i++; }
        }

        //  Fill the pixels between node pairs.
        // log_d("Polygon: %i, %i", nodes, color);
        for (i=0; i <= nodes-2; i+=2) {
            if( nodeX[i] >= SCREEN_WIDTH) break;
            if( nodeX[i+1] <= 0 ) continue;
            if (nodeX[i] < 0 ) nodeX[i] = 0;
            if (nodeX[i+1] > SCREEN_WIDTH) nodeX[i+1]=SCREEN_WIDTH;
            // log_d("drawLine: %i, %i, %i, %i", nodeX[i], pixelY, nodeX[i+1], pixelY);
            tft.drawLine( nodeX[i], SCREEN_HEIGHT - pixelY, nodeX[i+1], SCREEN_HEIGHT - pixelY, color);
        }
    }
}

void draw( TFT_eSPI& tft, ViewPort& viewPort, MemMap& mmap)
{
    log_d("Polylines: %i", mmap.polylines.size());
    Point32 screen_center_mc = viewPort.center - mmap.features_offset;  // screen center with features coordinates
    BBox screen_bbox_mc = viewPort.bbox - mmap.features_offset;  // screen boundaries with features coordinates
    tft.fillScreen( BACKGROUND_COLOR);

    ////// Polygons 
    for( Polygon polygon : mmap.polygons){
        uint16_t pl_color = get_color( polygon.color);
        if( pl_color == TFT_YELLOW) log_d("color: %s", polygon.color);
        std::vector<Point16> points2;    
        for( Point16 p : polygon.points){
            if( !screen_bbox_mc.contains_point( p)) continue;  // TODO
            points2.push_back( toScreenCoords( p, screen_center_mc));
            log_v("polygon: %s (%i,%i)", polygon.color, p.x, p.y);
        }
        fill_polygon( tft, points2, pl_color);
    }
    
    ////// Lines 
    for( Polyline polyline : mmap.polylines){
        uint16_t pl_color = get_color( polyline.color);        
        for( int i=0; i < (polyline.points.size() - 1); i++){
            Point16 p1 = polyline.points[i];
            Point16 p2 = polyline.points[i+1];
            if( !screen_bbox_mc.contains_point( p1) && !screen_bbox_mc.contains_point( p2)) continue; // out of screen. TODO: could still cut the screen area!
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