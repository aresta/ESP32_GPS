#include <Arduino.h>
#include <TFT_eSPI.h>
#include "colors.h"
#include <log.h>
#include "files.h"
#include "graphics.h"
#include "maps.h"
#include "gps.h"
#include "draw.h"

#include "../lib/conf.h"
#include "env_example.h"

TFT_eSPI tft = TFT_eSPI();
HardwareSerial SerialGPS(1);

Point32 map_center(INIT_POS);
MemCache memCache;

const char* base_folder = "/mymap/"; // TODO: folder selection

int zoom_level = PIXEL_SIZE_DEF; // zoom_level = 1 correspond aprox to 1 meter / pixel
int mode = DEVMODE_NAV;

ViewPort viewPort(map_center, zoom_level, TFT_WIDTH, TFT_HEIGHT);

const IFileSystem* fileSystem = get_file_system(base_folder);

void tft_header( Coord& pos)
{
    tft.fillRect(0, 0, 240, 25, YELLOWCLEAR);
    tft.setCursor(5,5,2);
    tft.print(pos.lng, 4);
    tft.print(" "); tft.print(pos.lat, 4);
    tft.print(" Sats: "); tft.print(pos.satellites);
    tft.print(" M: "); tft.print( mode);
}

void tft_footer( String msg)
{
    tft.fillRect(0, 300, 240, 320, CYAN);
    tft.setCursor(5,305,2);
    tft.println(msg);
}

void tft_msg( String msg)
{
    tft.fillRect(0, 0, 240, 25, CYAN);
    tft.setCursor(5,5,2);
    tft.println(msg);
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
    pinMode( UP_BUTTON, INPUT_PULLUP);
    pinMode( DOWN_BUTTON, INPUT_PULLUP);
    pinMode( LEFT_BUTTON, INPUT_PULLUP);
    pinMode( RIGHT_BUTTON, INPUT_PULLUP);
    pinMode( SELECT_BUTTON, INPUT_PULLUP);
    pinMode( TFT_OFF_BUTTON, INPUT_PULLUP);
    pinMode( MENU_BUTTON, INPUT_PULLUP);
    pinMode( TFT_BLK_PIN, OUTPUT);
    // pinMode( GPS_CE, OUTPUT);
    digitalWrite( TFT_BLK_PIN, LOW); // switch off display

    Serial.begin(115200);
    // printFreeMem();
#ifdef ARDUINO_uPesy_WROVER
#else   
    SerialGPS.begin(9600, SERIAL_8N1, GPS_TX, GPS_RX);
#endif
    digitalWrite( SD_CS_PIN, HIGH); // SD card chips select
    digitalWrite( TFT_CS, HIGH); // TFT chip select

    tft.init();
    tft.setRotation(0);  // portrait
    tft.invertDisplay( true);
    tft.fillScreen( CYAN);
    tft.setTextColor(TFT_BLACK);
    digitalWrite( TFT_BLK_PIN, HIGH);
    tft.setCursor(5,5,4);
    tft.println("Initializing...");
    digitalWrite( TFT_BLK_PIN, HIGH);
    if(!init_file_system()) {
        tft.println("Error: SD Card Mount Failed!");
        while(true);
    }
    tft.println("Reading map...");

    Point32 map_center( INIT_POS);
    // TODO: keep and show last position
    viewPort.setCenter( map_center);
    get_map_blocks(fileSystem, viewPort.bbox, memCache );
    draw( viewPort, memCache);
    tft_msg("Waiting for satellites...");
    // stats(viewPort, mmap);
    // printFreeMem();

    // digitalWrite( GPS_CE, HIGH); // GPS low power mode disabled
    gpio_wakeup_enable( (gpio_num_t )TFT_OFF_BUTTON, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
}

double prev_lat=500, prev_lng=500;
Coord coord;
std::vector<Coord> samples;
void loop()
{
    Point32 p = viewPort.center;
    bool moved = false;

    if( mode == DEVMODE_NAV){
        coord = getPosition( SerialGPS );
        if( coord.isValid && 
            abs(coord.lat-prev_lat) > 0.00005 &&
            abs(coord.lng-prev_lng) > 0.00005 ){
                p = coord.getPoint32();
                prev_lat = coord.lat;
                prev_lng = coord.lng;
                moved = true;
        }   
    }

    if( digitalRead( TFT_OFF_BUTTON) == LOW){
        digitalWrite( TFT_BLK_PIN, LOW);
        // digitalWrite( GPS_CE, LOW); // GPS low power mode. TODO: this way needs cold restart => don't work for tracking
        // setCpuFrequencyMhz(40); // TODO: check 20,10
        log_d("Enter TFT_OFF_BUTTON");
        esp_sleep_wakeup_cause_t wakeup_reason;
        delay(400); // button debounce
        do {
            // esp_sleep_enable_timer_wakeup( 10 * 1000000);
            log_d("esp_light_sleep_start");
            esp_light_sleep_start();
            wakeup_reason = esp_sleep_get_wakeup_cause();
            if( wakeup_reason == ESP_SLEEP_WAKEUP_TIMER){
                coord = getPosition( SerialGPS );
                // TODO
            }
        } while( wakeup_reason == ESP_SLEEP_WAKEUP_TIMER);
        log_d("Wakeup");
        digitalWrite( TFT_BLK_PIN, HIGH);
        delay(400); // button debounce
    }

    if( digitalRead( SELECT_BUTTON) == LOW){
        mode += 1;
        if( mode > DEVMODE_ZOOM){ 
            mode = DEVMODE_NAV;
            moved = true; // recenter
        }
        tft_header( coord);
        delay( 200); // button debouncing
    }

    if( mode == DEVMODE_MOVE){
        if( digitalRead( UP_BUTTON) == LOW)    { p.y += 40*zoom_level; moved = true; }
        if( digitalRead( DOWN_BUTTON) == LOW)  { p.y -= 40*zoom_level; moved = true; }
        if( digitalRead( LEFT_BUTTON) == LOW)  { p.x -= 40*zoom_level; moved = true; }
        if( digitalRead( RIGHT_BUTTON) == LOW) { p.x += 40*zoom_level; moved = true; }
    }

    if( mode == DEVMODE_ZOOM){
        if( digitalRead( UP_BUTTON) == LOW)    { zoom_level += 1; moved = true; }
        if( digitalRead( DOWN_BUTTON) == LOW)  { zoom_level -= 1; moved = true; }
        if( zoom_level < 1){ zoom_level = 1; moved = false; } 
        if( zoom_level > MAX_ZOOM){ zoom_level = MAX_ZOOM; moved = false; }
    }

    if( moved) {
        viewPort.setCenter( p);
        get_map_blocks(fileSystem, viewPort.bbox, memCache);
        draw( viewPort, memCache);
        tft_header( coord);
        tft_footer( String(zoom_level));
        delay( 10);
    }
}
