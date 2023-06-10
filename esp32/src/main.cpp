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

void setup()
{
    Serial.begin(115200);
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
    Serial.println("Olakease!!!");

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

    // tft.setCursor(20,20,4);
    // tft.setTextColor(TFT_BLACK);
    // tft.println ("Olakease!");
    // tft.fillRect(0, 0, 20, 20, 0xf800);
    // tft.fillRect(20, 0, 20, 20, 0x07E0);
    // tft.fillRect(40, 0, 20, 20, 0x001F);
    // tft.drawWideLine(0,0,100,100,10,0xf800);
    // tft.drawWideLine(0,0,100,100,5,0xFFFF);
    // tft.drawWideLine(50,0,150,100,10,0xFFFF,0xf800);
    // tft.drawLine(180, 50, 220, 100, TFT_GREEN);
    // tft.invertDisplay(true);  // TODO: check
    // tft.setRotation(2);  // portrait
    // tft.fillScreen( BACKGROUND_COLOR);
    // tft.fillRect(0, 0, 240, 25, TFT_YELLOW);
    // tft.setCursor(0,0,2);
    // tft.print(pos.lng, 4);
    // tft.print(" "); tft.print(pos.lat, 4);
    // tft.print(" Sat: "); tft.print(pos.satellites);

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
