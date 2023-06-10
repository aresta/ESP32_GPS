#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "../conf.h"
#include "graphics.h"
#include "maps.h"

bool init_sd_card()
{
    if (!SD.begin(13)) {
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD.cardType();
    
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);   
    Serial.println("initialisation done.");
    return true;
}

void parse_coords( ReadBufferingStream& file, std::vector<Point16>& points){
    char coord[20];
    int i, c;
    while(true){
        i = 0;
        c = file.read();
        if( c == '\n') break;
        while(c >= 0 && c != ';' && i < 20) {
            coord[i++] = (char )c;
            c = file.read();
        }
        coord[i] = '\0';
        points.push_back( Point16( coord));
    }
    points.shrink_to_fit();
}


void import_lines( MemMap& mmap)
{
    File file = SD.open( POLYLINES_FILE);
    ReadBufferingStream bufferedFile{ file, 500};
    String features = bufferedFile.readStringUntil('\n');
    String offset_x = bufferedFile.readStringUntil('\n');
    String offset_y = bufferedFile.readStringUntil('\n');
    if( !offset_x.startsWith("offset_x:") || !offset_y.startsWith("offset_y:")) {
        log_e("Wrong map file");
    }
    mmap.features_offset = Point32( 
        offset_x.substring(10).toInt(), 
        offset_y.substring(10).toInt());
    log_d("features_offset: (%i,%i)", mmap.features_offset.x, mmap.features_offset.y);
    BBox mmap_features_bbox( mmap.bbox.min - mmap.features_offset, mmap.bbox.max - mmap.features_offset);
    bufferedFile.readStringUntil('\n'); // blank line
    Polyline polyline;
    int line = 5;
    int total_points = 0;
    while( bufferedFile.available()){
        polyline.color = get_color( bufferedFile.readStringUntil('\n'));
        line++;
        polyline.width = bufferedFile.readStringUntil('\n').toInt() ?: 1;
        line++;
        polyline.z_order = bufferedFile.readStringUntil('\n').toInt();
        line++;
        const int num_coord = bufferedFile.readStringUntil('\n').toInt();
        line++;
        if( polyline.width < 1 ||  // basic checks
            polyline.width > 50 ||
            polyline.z_order > 125 ||
            polyline.z_order < -125 ||
            num_coord <= 0 ){
                log_e("ERROR reading map file in line %i", line);
        }
        polyline.points.clear();
        parse_coords( bufferedFile, polyline.points);
        line++;
        for( Point16 point : polyline.points){
            if( mmap_features_bbox.contains_point( point)) { 
                mmap.polylines.push_back( polyline);
                total_points += polyline.points.size();
                log_v("Added polyline. Total: %i", mmap.polylines.size());
                break;
            }
        }
        // log_d("FreeHeap: %i %i", ESP.getFreeHeap(), uxTaskGetStackHighWaterMark(NULL));
    }
    mmap.polylines.shrink_to_fit();
    file.close();
    log_d("Done! Polylines: %i Points:%i Total memory:%i", mmap.polylines.size(), total_points, mmap.polylines.size()*20 + total_points*4);
}

void import_polygons( MemMap& mmap)
{
    File file = SD.open( POLYGONS_FILE);
    ReadBufferingStream bufferedFile{ file, 500};
    String features = bufferedFile.readStringUntil('\n');
    String offset_x = bufferedFile.readStringUntil('\n');
    String offset_y = bufferedFile.readStringUntil('\n');
    if( !offset_x.startsWith("offset_x:") || !offset_y.startsWith("offset_y:")) {
        log_e("Wrong map file");
    }
    mmap.features_offset = Point32( 
        offset_x.substring(10).toInt(), 
        offset_y.substring(10).toInt());
    log_d("features_offset: (%i,%i)", mmap.features_offset.x, mmap.features_offset.y);
    BBox mmap_features_bbox = mmap.bbox - mmap.features_offset;
    bufferedFile.readStringUntil('\n'); // blank line
    Polygon polygon;
    int line = 5;
    int total_points = 0;
    while( bufferedFile.available()){
        polygon.color = get_color( bufferedFile.readStringUntil('\n'));
        line++;
        polygon.z_order = bufferedFile.readStringUntil('\n').toInt();
        line++;
        const int num_coord = bufferedFile.readStringUntil('\n').toInt();
        line++;
        if( num_coord <= 0 ||
            polygon.z_order > 125 ||
            polygon.z_order < -125  ){   // basic checks
                log_e("ERROR reading map file in line %i", line);
        }

        polygon.points.clear();
        parse_coords( bufferedFile, polygon.points);
        // clip_polygon( mmap_features_bbox, polygon.points);
        line++;
        for( Point16 point : polygon.points){
            if( mmap_features_bbox.contains_point( point)) { 
                mmap.polygons.push_back( polygon);
                total_points += polygon.points.size();
                break;
            }
        }
        // log_d("FreeHeap: %i %i", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    }
    mmap.polygons.shrink_to_fit();
    file.close();
    log_d("Done! polygons: %i Points:%i Total memory:%i", mmap.polygons.size(), total_points, mmap.polygons.size()*20 + total_points*4);
}

void import_map_features( MemMap& mmap)
{
    import_lines( mmap);
    import_polygons( mmap);
}

void MemMap::setBounds( Point32 map_center, int32_t map_width, int32_t map_height)
{
    bbox = BBox(
        Point32(    map_center.x - map_width  / 2,
                    map_center.y - map_height / 2),
        Point32(    map_center.x + map_width  / 2,
                    map_center.y + map_height / 2));
}


Point32::Point32( char *coords_pair)
{
    char *next;
    x = round( strtod( coords_pair, &next));  // 1st coord
    y = round( strtod( ++next, NULL));  // 2nd coord
}