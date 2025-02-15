#include <Arduino.h>
#include <TFT_eSPI.h>
#include "graphics.h"
#include "maps.h"
#include "conf.h"

extern uint8_t zoom_level;

void fill_polygon( Polygon p) // scanline fill algorithm
{
  int16_t maxy = p.bbox.max.y;
  int16_t miny = p.bbox.min.y;

  if( maxy >= SCREEN_HEIGHT) maxy = SCREEN_HEIGHT-1;
  if( miny < 0) miny = 0;
  if(miny >= maxy) return;
  
  std::vector<int16_t> nodeX;
  nodeX.reserve(p.points.size());

  //  Loop through the rows of the image.
  int16_t nodes, i , swap;
  for( int16_t pixelY=miny; pixelY <= maxy; pixelY++) {  //  Build a list of nodes.    
    nodeX.clear();
    nodes=0;
    for( int i=0; i < (p.points.size() - 1); i++) {
      if( (p.points[i].y < pixelY && p.points[i+1].y >= pixelY) ||
        (p.points[i].y >= pixelY && p.points[i+1].y < pixelY)) {
          int16_t intersectX = p.points[i].x + 
            (int16_t)((double(pixelY - p.points[i].y) / double(p.points[i+1].y - p.points[i].y)) *
            (p.points[i+1].x - p.points[i].x));
          nodeX.push_back(intersectX);
          nodes++;
        }
    }

    // Sort intersections.
    std::sort(nodeX.begin(), nodeX.end());

    //  Fill the pixels between node pairs.
    for (i=0; i <= nodes-2; i+=2) {
      if( nodeX[i] > SCREEN_WIDTH) break;
      if( nodeX[i+1] < 0 ) continue;
      
      int16_t startX = (nodeX[i] < 0) ? 0 : nodeX[i];
      int16_t endX = (nodeX[i+1] > SCREEN_WIDTH) ? SCREEN_WIDTH : nodeX[i+1];
      spr.drawLine(startX, SCREEN_HEIGHT - pixelY, endX, SCREEN_HEIGHT - pixelY, p.color);
    }
  }
}

/// @brief Draw to the display the map visible in the viewPort area
/// @param viewPort area to be drawn
/// @param memCache map blocks in memory
void draw( ViewPort& viewPort, MemCache& memCache)
{
  Polygon new_polygon;
  spr.fillScreen( BACKGROUND_COLOR);
  uint32_t total_time = millis();
  log_v("Draw start %i", total_time);
  for( MapBlock* mblock: memCache.blocks){
    uint32_t block_time = millis();
    if( !mblock->inView) continue;

    // block to draw
    Point16 screen_center_mc = viewPort.center.toPoint16() - mblock->offset.toPoint16();  // screen center with features coordinates
    BBox screen_bbox_mc = viewPort.bbox - mblock->offset;  // screen boundaries with features coordinates
    
    Point16 offset(
      SCREEN_WIDTH / 2 - (screen_center_mc.x / zoom_level),
      SCREEN_HEIGHT / 2 - (screen_center_mc.y / zoom_level));

    ////// Polygons 
    for( Polygon polygon : mblock->polygons){
      if( zoom_level > polygon.maxzoom) continue;
      if( !polygon.bbox.intersects( screen_bbox_mc)) continue;
      new_polygon.color = polygon.color;
      new_polygon.bbox.min = (polygon.bbox.min / zoom_level) + offset;
      new_polygon.bbox.max = (polygon.bbox.max / zoom_level) + offset;
      
      new_polygon.points.clear();
      for( Point16 p : polygon.points){ // TODO: move to fill_polygon
        new_polygon.points.push_back( p.toScreenCoord( zoom_level, offset));
      }
      fill_polygon( new_polygon);
      
    }
    log_v("Block polygons done %i ms", millis()-block_time);
    block_time = millis();
    
    ////// Lines
    for( Polyline line : mblock->polylines){
      if( zoom_level > line.maxzoom) continue;
      if( !line.bbox.intersects( screen_bbox_mc)) continue;

      for( int i=0; i < line.points.size() - 1; i++) {  //TODO optimize
        Point16 p1 = line.points[i].toScreenCoord( zoom_level, offset);
        Point16 p2 = line.points[i+1].toScreenCoord( zoom_level, offset);
        spr.drawWideLine(
          p1.x, SCREEN_HEIGHT - p1.y,
          p2.x, SCREEN_HEIGHT - p2.y,
          (line.width/zoom_level) ?: 1, line.color, line.color);
      }
    }
    log_v("Block lines done %i ms", millis()-block_time);
  }
  log_v("Total %i ms", millis()-total_time);


  // TODO: paint only in NAV mode
  spr.fillTriangle(
    SCREEN_WIDTH/2 - 4, SCREEN_HEIGHT/2 + 5, 
    SCREEN_WIDTH/2 + 4, SCREEN_HEIGHT/2 + 5, 
    SCREEN_WIDTH/2,   SCREEN_HEIGHT/2 - 6, 
    RED);
  log_v("Draw done! %i", millis());
}

void stats( ViewPort& viewPort, MapBlock& mblock)
{
  Point32 screen_center_mc = viewPort.center - mblock.offset;  // screen center with features coordinates
  BBox screen_bbox_mc = viewPort.bbox - mblock.offset;  // screen boundaries with features coordinates

  ////// Polygons 
  int in_screen = 0, in_map = 0,  points_total = 0;
  for( Polygon polygon : mblock.polygons){
    bool hit = false;
    for( Point16 p : polygon.points){
      points_total++;
      if( screen_bbox_mc.contains_point( p)) in_screen++;
      // if( map_bbox_mc.contains_point( p)) in_map++;
    }
  }
  log_i("Polygons points.  in_screen: %i, in_map: %i,  total: %i", in_screen, in_map, points_total);
  
  ////// Lines 
  in_screen = 0;
  in_map = 0;
  points_total = 0;
  for( Polyline polyline : mblock.polylines){
    for( Point16 p : polyline.points){
      points_total++;
      if( screen_bbox_mc.contains_point( p)) in_screen++;
      // if( map_bbox_mc.contains_point( p)) in_map++;
    }
  }
  log_i("Lines points. in_screen: %i,  in_map: %i,  total: %i", in_screen, in_map, points_total);
}