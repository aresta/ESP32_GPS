#include <Arduino.h>
#include <TFT_eSPI.h> 
#include "maps.h"
#include "gps.h"
#include "graphics.h"
#include "../lib/conf.h"

TFT_eSPI tft = TFT_eSPI();
HardwareSerial SerialGPS(2);
MemBlocks memBlocks;
ViewPort viewPort;
int pixel_size = PIXEL_SIZE_DEF;
int mode = DEVMODE_NAV;

void print_header( Coord& pos)
{
    tft.fillRect(0, 0, 240, 25, YELLOWCLEAR);
    tft.setCursor(5,5,2);
    tft.print(pos.lng, 4);
    tft.print(" "); tft.print(pos.lat, 4);
    tft.print(" Sats: "); tft.print(pos.satellites);
    tft.print(" M: "); tft.print( mode);
}

void printFreeMem()
{
    log_i("FreeHeap: %i", esp_get_free_heap_size());
    log_i("Heap minimum_free_heap_size: %i", esp_get_minimum_free_heap_size());
    log_i("Heap largest_free_block: %i", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    log_i("Task watermark: %i", uxTaskGetStackHighWaterMark(NULL));
}

void setup()
{
    Serial.begin(115200);
    printFreeMem();
#ifdef ARDUINO_uPesy_WROVER
    SerialGPS.begin(9600, SERIAL_8N1, 26, 27);  // rx=gpio26, tx=gpio27 (Wrover)
#else   
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17);  // other boards with rx=gpio16, tx=gpio17
#endif
    digitalWrite(15, HIGH); // TFT screen chip select
    digitalWrite(13, HIGH); // SD card chips select

    // butons
    pinMode( UP_BUTTON, INPUT_PULLUP);
    pinMode( DOWN_BUTTON, INPUT_PULLUP);
    pinMode( LEFT_BUTTON, INPUT_PULLUP);
    pinMode( RIGHT_BUTTON, INPUT_PULLUP);
    pinMode( SELECT_BUTTON, INPUT_PULLUP);

    tft.init();
    delay(100);
    tft.setRotation(0);  // portrait
    tft.invertDisplay( true);
    tft.fillScreen( BACKGROUND_COLOR);
    tft.setTextColor(TFT_BLACK);
    header_msg("Initializing...");    
    if(!init_sd_card()) while(true);
    header_msg("Reading map...");

    Point32 map_center( 225680.32, 5084950.61);
    // Point32 map_center( 224672.31, 5107378.91); // La Mola
    // Point32 map_center( 235664.91, 5074788.07); // Tibidabo
    // Point32 map_center( 244808.69, 5070020.31); // bcn
    // TODO: keep and show last position
    viewPort.setCenter( map_center);
    get_map_blocks( memBlocks, viewPort.bbox );
    draw( viewPort, memBlocks);
    header_msg( "Waiting for satellites...");
    // stats(viewPort, mmap);
    printFreeMem();
}

double prev_lat=500, prev_lng=500;
Coord pos;
void loop()
{
    Point32 p = viewPort.center;
    bool moved = false;
    if( mode == DEVMODE_NAV){
        pos = getPosition( SerialGPS );
        if( pos.isValid && 
            abs(pos.lat-prev_lat) > 0.00005 &&
            abs(pos.lng-prev_lng) > 0.00005 ){
                p = pos.getPoint32();
                prev_lat = pos.lat;
                prev_lng = pos.lng;
                moved = true;
        }   
    }
    if( digitalRead( SELECT_BUTTON) == LOW){
        mode += 1;
        if( mode > DEVMODE_MENU){ 
            mode = DEVMODE_NAV;
            moved = true; // recenter
        }
        print_header( pos);
        delay( 400); // for button debouncing
    }
    if( mode == DEVMODE_MOVE){
        if( digitalRead( UP_BUTTON) == LOW)    { p.y += 40*pixel_size; moved = true; }
        if( digitalRead( DOWN_BUTTON) == LOW)  { p.y -= 40*pixel_size; moved = true; }
        if( digitalRead( LEFT_BUTTON) == LOW)  { p.x -= 40*pixel_size; moved = true; }
        if( digitalRead( RIGHT_BUTTON) == LOW) { p.x += 40*pixel_size; moved = true; }
    }
    if( mode == DEVMODE_ZOOM){
        if( digitalRead( UP_BUTTON) == LOW)    { pixel_size += 1; moved = true; }
        if( digitalRead( DOWN_BUTTON) == LOW)  { pixel_size -= 1; moved = true; }
        if( pixel_size < 1){ pixel_size = 1; moved = false; } 
        if( pixel_size > 6){ pixel_size = 6; moved = false; }
    }
    // while( Serial.available()){
    //     char key = Serial.read();
    //     if( key == 0x1B || key == 0x5B) continue;  // skip ESC...
    //     if( key == 0x44)     p.x -= 50; // left arrow key
    //     else if(key == 0x43) p.x += 50; // rigth
    //     else if(key == 0x41) p.y += 50; // up
    //     else if(key == 0x42) p.y -= 50; // down
    //     moved = true;
    // }
    if( moved) {
        viewPort.setCenter( p);
        get_map_blocks( memBlocks, viewPort.bbox);
        draw( viewPort, memBlocks);
        print_header( pos);
        delay( 10);
    }
}
