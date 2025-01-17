#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdint.h>
#include "graphics.h"
#include "conf.h"
#include "maps.h"


MemCache memCache;
extern ViewPort viewPort;
extern Point32 display_pos;
extern Coord gps_coord; 
extern uint8_t mode;
extern uint8_t zoom_level;

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

void tft_header()
{
  tft.fillRect(0, 0, 240, 30, YELLOWCLEAR);
  if( gps_coord.fixAcquired){
    tft.setCursor(2,5,4);
    tft.print( gps_coord.lng, 3);
    tft.print(", "); tft.print( gps_coord.lat, 3);
  } else {
    tft.setCursor(2,5,2);
    tft.print("Waiting for sats");
  }
  tft.setCursor(185,7,2);
  tft.print("Sats: "); tft.print( gps_coord.satellites);
}

void tft_footer()
{
  tft.fillRect(0, 290, 240, 320, CYAN);
  tft.setCursor(2,295,4);
  switch ( mode){
    case DEVMODE_NAV:  tft.print("Nav"); break;
    case DEVMODE_MOVE: tft.print("Move"); break;
    case DEVMODE_ZOOM: tft.print("Zoom"); break;  
  }
  tft.setCursor(130,295,4);
  tft.print("Zoom: "); tft.print( zoom_level);
}

void tft_msg( const char *msg)
{
  tft.fillRect(0, 0, 240, 25, BLACK); // Clear with the background color
  tft.fillRect(0, 0, 240, 25, CYAN);
  tft.setCursor(5,5,2);
  tft.print( msg);
}

void refresh_display() 
{
  viewPort.setCenter( display_pos);
  get_map_blocks( viewPort.bbox, memCache);
  draw( viewPort, memCache);
  tft_header();
  tft_footer();
  // delay( 10);
}


