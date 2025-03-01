#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdint.h>
#include "graphics.h"
#include "conf.h"
#include "maps.h"


MemCache memCache;
ViewArea displayArea;
extern Point32 gps_pos_wc;
extern Coord gps_coord; 
extern uint8_t mode;
extern uint8_t zoom_level;
extern TFT_eSprite *spr[];

void ViewArea::setCenter(Point32& pcenter, int16_t width, int16_t eight) {
  center = pcenter;
  bbox.min.x = pcenter.x - (width  * zoom_level) / 2;
  bbox.min.y = pcenter.y - (eight * zoom_level) / 2;
  bbox.max.x = pcenter.x + (width  * zoom_level) / 2;
  bbox.max.y = pcenter.y + (eight * zoom_level) / 2;
}


Point16::Point16( char *coords_pair)
{
  char *next;
  x = (int16_t )round( strtod( coords_pair, &next));  // 1st coord // TODO: change by strtol and test
  y = (int16_t )round( strtod( ++next, NULL));  // 2nd coord
}


bool BBox::intersects(const BBox b) const{ 
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
  tft.fillRect(0, 0, 240, 30, gps_coord.fixAcquired ? YELLOWCLEAR : ORANGE);
  tft.setCursor(2,5,4);
  tft.print( gps_coord.lng, 4);
  tft.print(","); tft.print( gps_coord.lat, 4);
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
  tft.fillRect(0, 0, 240, 50, BLACK); // Clear with the background color
  tft.fillRect(0, 0, 240, 50, ORANGE);
  tft.setCursor(10,10,4);
  tft.print( msg);
}

/// @brief Updates the display with the corresponging area from the buffer. Refresh the buffer if needed.
void refresh_display() 
{
  LOGD("Fix?:%i, Sats:%i, isUpdated:%i", gps_coord.fixAcquired, gps_coord.satellites, gps_coord.isUpdated);
  LOGD("lat:%f, lon:%f", gps_coord.lat, gps_coord.lng);
  LOGD("Altitude: %i", gps_coord.altitude);
  
  // Set up displayArea based on current GPS position and display dimensions.
  displayArea.setCenter( gps_pos_wc, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  static bool buffer_valid[MAX_ZOOM+1] = { false }; // per zoom level cache
  static ViewArea bufferArea[MAX_ZOOM+1];

  // Check if we need to update the buffer
  if( !buffer_valid[zoom_level] || !displayArea.bbox.contained_in( bufferArea[zoom_level].bbox) ){
      setCpuFrequencyMhz(240); // speed up
      bufferArea[zoom_level].setCenter( gps_pos_wc, SCREEN_BUFFER_WIDTH, SCREEN_BUFFER_HEIGHT);
      get_map_blocks( bufferArea[zoom_level].bbox, memCache);
      draw( bufferArea[zoom_level], memCache, spr[zoom_level]);
      buffer_valid[zoom_level] = true;
      setCpuFrequencyMhz(40);
  }
  
  setCpuFrequencyMhz(80);
  // Calculate the square from the buffer to be pushed to the display
  Point32 src = (displayArea.bbox.min - bufferArea[zoom_level].bbox.min) / zoom_level;
  
  // Push the corresponding portion of the sprite to the TFT.
  tft.writecommand(0x00); // workaround to avoid strange display behaviours
  spr[zoom_level]->pushSprite(0, 0, src.x, SCREEN_HEIGHT - src.y, SCREEN_WIDTH, SCREEN_HEIGHT);

  // draw the position triangle in the center of the display
  tft.fillTriangle(
    SCREEN_WIDTH/2 - 4, SCREEN_HEIGHT/2 + 5, 
    SCREEN_WIDTH/2 + 4, SCREEN_HEIGHT/2 + 5, 
    SCREEN_WIDTH/2,   SCREEN_HEIGHT/2 - 6, 
    RED);
  
  // refreash header and footer
  tft_header();
  tft_footer();
  setCpuFrequencyMhz(40);
  LOGD("pushSprite done");
}


