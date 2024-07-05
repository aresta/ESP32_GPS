#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

#include <colors.h>
#include <graphics.h>
#include <gps.h>
#include <files.h>
#include <maps.h>
#include <canvas.h>

#include "../lib/conf.h"

void printFreeMem()
{
    log_i("FreeHeap: %i", esp_get_free_heap_size());
    log_i("Heap minimum_free_heap_size: %i", esp_get_minimum_free_heap_size());
    log_i("Heap largest_free_block: %i", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    log_i("Task watermark: %i", uxTaskGetStackHighWaterMark(NULL));
}

void Run();

bool ReadInput(int pin)
{
    return digitalRead(pin) == LOW;
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
    gpsInit();

    digitalWrite(SD_CS_PIN, HIGH); // SD card chips select

    Run();

    // stats(viewPort, mmap);
    // printFreeMem();

    // digitalWrite( GPS_CE, HIGH); // GPS low power mode disabled
    gpio_wakeup_enable( (gpio_num_t )TFT_OFF_BUTTON, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
}

void loop()
{
    Point32 p = viewPort.center;
    bool moved = false;

    if (mode == DEVMODE_NAV) {
        Coord coord;
        gpsGetPosition(coord);
        if (coord.isValid && 
            abs(coord.lat-prev_lat) > 0.00005 &&
            abs(coord.lng-prev_lng) > 0.00005) {
                p = coord.getPoint32();
                prev_lat = coord.lat;
                prev_lng = coord.lng;
                moved = true;
            }   
    }

    if (digitalRead(TFT_OFF_BUTTON) == LOW) {
        digitalWrite(TFT_BLK_PIN, LOW);
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
                gpsGetPosition(coord);
                // TODO
            }
        } while( wakeup_reason == ESP_SLEEP_WAKEUP_TIMER);
        log_d("Wakeup");
        digitalWrite( TFT_BLK_PIN, HIGH);
        delay(400); // button debounce
    }

    if (digitalRead(SELECT_BUTTON) == LOW) {
        mode += 1;
        if( mode > DEVMODE_ZOOM){ 
            mode = DEVMODE_NAV;
            moved = true; // recenter
        }
        tft_header(coord, mode);
        delay(200); // button debouncing
    }

    if (mode == DEVMODE_MOVE) {
        if( digitalRead( UP_BUTTON) == LOW)    { p.y += 40*zoom_level; moved = true; }
        if( digitalRead( DOWN_BUTTON) == LOW)  { p.y -= 40*zoom_level; moved = true; }
        if( digitalRead( LEFT_BUTTON) == LOW)  { p.x -= 40*zoom_level; moved = true; }
        if( digitalRead( RIGHT_BUTTON) == LOW) { p.x += 40*zoom_level; moved = true; }
    }

    if (mode == DEVMODE_ZOOM) {
        if( digitalRead( UP_BUTTON) == LOW)    { zoom_level += 1; moved = true; }
        if( digitalRead( DOWN_BUTTON) == LOW)  { zoom_level -= 1; moved = true; }
        if( zoom_level < 1){ zoom_level = 1; moved = false; } 
        if( zoom_level > MAX_ZOOM){ zoom_level = MAX_ZOOM; moved = false; }
    }

    if (moved) {
        viewPort.setCenter(p);
        get_map_blocks(fileSystem, viewPort.bbox, memCache);
        draw(viewPort, memCache);
        tft_header(coord, mode);
        tft_footer(String(zoom_level).c_str());
        delay(10);
    }
}
