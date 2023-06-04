#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdint.h>
#include "graphics.h"


uint32_t get_color( String color)
{
    return  
            color == "white"        ? WHITE :  // TODO: rework
            color == "black"        ? BLACK : 
            color == "gray"         ? GRAY : 
            color == "grayclear"    ? GRAYCLEAR : 
            color == "grayclear2"   ? GRAYCLEAR2 : 
            color == "red"          ? RED : 
            color == "green"        ? GREEN : 
            color == "greenclear"   ? GREENCLEAR : 
            color == "greenclear2"  ? GREENCLEAR2 : 
            color == "blue"         ? BLUE : 
            color == "blueclear"    ? BLUECLEAR : 
            color == "cyan"         ? CYAN : 
            color == "orange"       ? ORANGE : 
            color == "yellow"       ? YELLOW: 
            color == "pink"         ? TFT_PINK: 
            TFT_YELLOW;
}


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


void fill_polygon( TFT_eSPI& tft,  std::vector<Point16> points, int color) // scanline fill algorithm
{
    int16_t maxy = INT16_MIN, miny = INT16_MAX;

    for( Point16 p : points) { // TODO: precalculate at map file creation
        maxy = max( maxy, p.y);
        miny = min( miny, p.y);
    }
    if( maxy > SCREEN_HEIGHT) maxy = SCREEN_HEIGHT;
    if( miny < 0) miny = 0;
    assert( miny < maxy);

    // log_d("miny: %i, maxy: %i", miny, maxy);
    int16_t nodeX[points.size()], pixelY;

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
        assert( nodes < points.size());
        // log_d("pixelY: %i, nodes: %i", pixelY, nodes);

        //  Sort the nodes, via a simple “Bubble” sort.
        int16_t i=0, swap;
        while( i < nodes-1) {   // TODO: rework
            if( nodeX[i] > nodeX[i+1]) {
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


std::vector<Point16> clip_polygon( BBox bbox, std::vector<Point16> points)
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