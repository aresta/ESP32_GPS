#include <Arduino.h>
#include <TFT_eSPI.h>
#include "graphics.h"
#include "maps.h"
#include "../conf.h"



void fill_polygon( std::vector<Point16> points, int color) // scanline fill algorithm
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


void draw( ViewPort& viewPort, MemBlocks& memblocks)
{
    // tft.fillScreen( BACKGROUND_COLOR);
    std::vector<Polygon> polygons_to_draw;
    std::vector<Polyline> lines_to_draw;
    for( MapBlock* mblock: memblocks.blocks){
        if( !mblock || !mblock->inView) continue;
        Point32 screen_center_mc = viewPort.center - mblock->offset;  // screen center with features coordinates
        BBox screen_bbox_mc = viewPort.bbox - mblock->offset;  // screen boundaries with features coordinates
        
        ////// Polygons 
        for( Polygon polygon : mblock->polygons){
            if( polygon.color == TFT_YELLOW) log_w("Polygon type unknown");
            Polygon new_polygon;
            // std::vector<Point16> points2;
            bool hit = false;
            for( Point16 p : polygon.points){
                if( screen_bbox_mc.contains_point( p)) hit = true;
                new_polygon.points.push_back( toScreenCoords( p, screen_center_mc));
            }
            if( hit){
                new_polygon.color = polygon.color;
                polygons_to_draw.push_back( new_polygon);
            }
            // fill_polygon( points2, polygon.color);
        }
        
        ////// Lines 
        for( Polyline line : mblock->polylines){
            Polyline new_line;
            bool hit = false;
            bool prev_in_screen = false;
            for( int i=0; i < (line.points.size()); i++) {
                bool curr_in_screen = screen_bbox_mc.contains_point( line.points[i]);
                if( !prev_in_screen && !curr_in_screen){  // TODO: could still cut the screen area!
                    prev_in_screen = curr_in_screen;
                    continue;
                    }
                if( !prev_in_screen && curr_in_screen && i > 0){
                    new_line.points.push_back( toScreenCoords( line.points[i-1], screen_center_mc));
                }
                new_line.points.push_back( toScreenCoords( line.points[i], screen_center_mc));
                hit = true;
                prev_in_screen = curr_in_screen;
            }
            if( hit){
                new_line.color = line.color;
                new_line.width = line.width;
                lines_to_draw.push_back( new_line);
            }
        }
        tft.fillScreen( BACKGROUND_COLOR);
    }

    for( Polygon pol: polygons_to_draw){
        fill_polygon( pol.points, pol.color);
        // empty polygons test
        // for( int i=0; i < (pol.points.size() - 1); i++) {  
        //         tft.drawLine(
        //         pol.points[i].x, SCREEN_HEIGHT - pol.points[i].y,
        //         pol.points[i+1].x, SCREEN_HEIGHT - pol.points[i+1].y,
        //         pol.color);
        // }    
    }
    for( Polyline line: lines_to_draw){
        for( int i=0; i < (line.points.size() - 1); i++) {
                tft.drawWideLine(
                line.points[i].x, SCREEN_HEIGHT - line.points[i].y,
                line.points[i+1].x, SCREEN_HEIGHT - line.points[i+1].y,
                line.width/pixel_size ?: 1, line.color, line.color);
        }      
    }

    tft.fillTriangle( 
        SCREEN_WIDTH/2 - 4, SCREEN_HEIGHT/2 + 5, 
        SCREEN_WIDTH/2 + 4, SCREEN_HEIGHT/2 + 5, 
        SCREEN_WIDTH/2,     SCREEN_HEIGHT/2 - 6, 
        RED);
    log_v("Draw done!");
}

void stats( ViewPort& viewPort, MapBlock* mblock)
{
    Point32 screen_center_mc = viewPort.center - mblock->offset;  // screen center with features coordinates
    BBox screen_bbox_mc = viewPort.bbox - mblock->offset;  // screen boundaries with features coordinates
    BBox map_bbox_mc = mblock->bbox - mblock->offset;  // screen boundaries with features coordinates

    ////// Polygons 
    int in_screen = 0, in_map = 0,  points_total = 0;
    for( Polygon polygon : mblock->polygons){
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
    for( Polyline polyline : mblock->polylines){
        for( Point16 p : polyline.points){
            points_total++;
            if( screen_bbox_mc.contains_point( p)) in_screen++;
            if( map_bbox_mc.contains_point( p)) in_map++;
        }
    }
    log_i("Lines points. in_screen: %i,  in_map: %i,  total: %i", in_screen, in_map, points_total);
}