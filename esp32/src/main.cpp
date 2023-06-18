#include <Arduino.h>
#include <TFT_eSPI.h> 
#include "maps.h"
#include "gps.h"
#include "graphics.h"
#include <TFTShape.h>

TFT_eSPI tft = TFT_eSPI();
HardwareSerial SerialGPS(2);
MemBlocks memBlocks;
ViewPort viewPort;

void print_header( Coord& pos)
{
    tft.fillRect(0, 0, 240, 25, TFT_YELLOW);
    tft.setCursor(5,5,2);
    tft.print(pos.lng, 4);
    tft.print(" "); tft.print(pos.lat, 4);
    tft.print(" Sats: "); tft.print(pos.satellites);
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
    
    tft.init();
    if(!init_sd_card()) while(true);
    delay(500);
    tft.setRotation(2);  // portrait
    tft.invertDisplay(true);
    tft.fillScreen( BACKGROUND_COLOR);
    tft.fillRect(0, 0, 240, 25, TFT_YELLOW);
    tft.setCursor(5,5,2);
    tft.setTextColor(TFT_BLACK);
    tft.print("Reading map...");

    // Point32 map_center( 225400.0, 5085200.0); // TODO: change to last position
    Point32 map_center( 226023.09, 5085200.0); // TODO: change to last position
    viewPort.setCenter( map_center);
    get_map_blocks( memBlocks, viewPort.bbox );

    draw( tft, viewPort, memBlocks);
    // stats(viewPort, mmap);
    printFreeMem();
}

double prev_lat=500, prev_lng=500;
void loop()
{
    Coord pos = getPosition( SerialGPS );
    if( pos.isValid && 
        abs(pos.lat-prev_lat) > 0.00005 &&
        abs(pos.lng-prev_lng) > 0.00005 ){
            Point32 map_center = pos.getPoint32();
            viewPort.setCenter( map_center);
            get_map_blocks( memBlocks, viewPort.bbox);
            draw( tft, viewPort, memBlocks);
            prev_lat = pos.lat;
            prev_lng = pos.lng;
    }   
    bool moved = false;
    Point32 p = viewPort.center;
    while( Serial.available()){
        char key = Serial.read();
        if( key == 0x1B || key == 0x5B) continue;  // skip ESC...
        if( key == 0x44)     p.x -= 50; // left arrow key
        else if(key == 0x43) p.x += 50; // rigth
        else if(key == 0x41) p.y += 50; // up
        else if(key == 0x42) p.y -= 50; // down
        moved = true;
    }
    if( moved) {
        viewPort.setCenter( p);
        get_map_blocks( memBlocks, viewPort.bbox);
        draw( tft, viewPort, memBlocks);
    }
    print_header( pos);
    delay(500);
}
