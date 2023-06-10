#include <Arduino.h>
#include <TFT_eSPI.h> 
#include "maps.h"
#include "gps.h"
#include "graphics.h"
#include <TFTShape.h>

TFT_eSPI tft = TFT_eSPI();
HardwareSerial SerialGPS(2);
MemMap mmap;
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
    SerialGPS.begin(9600, SERIAL_8N1, 26, 27);  // rx=gpio26, tx=gpio27 (Wrover...)
#else   // any board with rx=gpio16, tx=gpio17
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
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
    Point32 map_center( 226540.24, 5083191.36); // TODO: change to last position
    viewPort.setCenter( map_center);
    int32_t map_width = SCREEN_WIDTH * PIXEL_SIZE * SCREEN_BUFFER_SIZE;
    int32_t map_height = SCREEN_HEIGHT * PIXEL_SIZE * SCREEN_BUFFER_SIZE;
    mmap.setBounds( map_center, map_width, map_height);
    log_d("mmap.bbox.min.x=%i, mmap.bbox.min.y=%i", mmap.bbox.min.x, mmap.bbox.min.y);
    import_map_features( mmap );
    draw( tft, viewPort, mmap);
    stats(viewPort, mmap);
    printFreeMem();
}

double prev_lat=500, prev_lng=500;
void loop()
{
    Coord pos = getPosition( SerialGPS );
    if( pos.isValid && 
        abs(pos.lat-prev_lat) > 0.00005 &&
        abs(pos.lng-prev_lng) > 0.00005 ){
            viewPort.setCenter( pos.getPoint32());
            draw( tft, viewPort, mmap);
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
        draw( tft, viewPort, mmap);
    }
    print_header( pos);
    delay(2000);
}
