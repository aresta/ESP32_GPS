#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <StreamUtils.h>
#include <string>
#include "graphics.h"
#include "maps.h"
#include "../conf.h"

const String base_folder = "/mymap/"; // TODO: folder selection

bool init_sd_card()
{
    if (!SD.begin(13)) {
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD.cardType();
    
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        header_msg("No SD card attached");
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

void parse_coords( ReadBufferingStream& file, std::vector<Point16>& points)
{
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
    // points.shrink_to_fit();
}

BBox parse_bbox(String str)
{
    char *next;
    int32_t x1 = (int32_t )strtol( str.c_str(), &next, 10);
    int32_t y1 = (int32_t )strtol( ++next, &next, 10);
    int32_t x2 = (int32_t )strtol( ++next, &next, 10);
    int32_t y2 = (int32_t )strtol( ++next, NULL, 10);
    return BBox( Point32( x1, y1), Point32( x2, y2));
}

MapBlock* read_map_block( String file_name)
{
    MapBlock* mblock = new MapBlock();
    File file = SD.open( file_name + ".fmp");
    if( !file){
        header_msg("Map file not found in folder: " + base_folder);
        while(true);
    }
    ReadBufferingStream bufferedFile{ file, 1000};

    // read polygons
    String feature_type = bufferedFile.readStringUntil(':');
    if( feature_type != "Polygons") log_e("Map error. Expected Polygons instead of: %s", feature_type);
    u_int32_t count = bufferedFile.readStringUntil('\n').toInt();
    assert( count > 0);

    int line = 5;
    int total_points = 0;
    Polygon polygon;
    while( count > 0 && bufferedFile.available()){
        polygon.color = (u_int16_t )std::stoul( bufferedFile.readStringUntil('\n').c_str(), nullptr, 16);
        line++;
        polygon.maxzoom = (u_int8_t )bufferedFile.readStringUntil('\n').toInt() ?: MAX_ZOOM;
        line++;

        String tag = bufferedFile.readStringUntil(':');
        assert( tag == "bbox");
        polygon.bbox = parse_bbox( bufferedFile.readStringUntil('\n'));
        line++;
        polygon.points.clear();
        tag = bufferedFile.readStringUntil(':');
        assert( tag == "coords");
        parse_coords( bufferedFile, polygon.points);
        line++;
        mblock->polygons.push_back( polygon);
        total_points += polygon.points.size();
        count--;
    }
    assert( count == 0);
    
    // read lines
    feature_type = bufferedFile.readStringUntil(':');
    if( feature_type != "Polylines") log_e("Map error. Expected Polylines instead of: %s", feature_type);
    count = bufferedFile.readStringUntil('\n').toInt();
    assert( count > 0);

    Polyline polyline;
    while( count > 0 && bufferedFile.available()){
        polyline.color = (u_int16_t )std::stoul( bufferedFile.readStringUntil('\n').c_str(), nullptr, 16);
        line++;
        polyline.width = (u_int8_t )bufferedFile.readStringUntil('\n').toInt() ?: 1;
        line++;
        polyline.maxzoom = (u_int8_t )bufferedFile.readStringUntil('\n').toInt() ?: MAX_ZOOM;
        line++;

        String tag = bufferedFile.readStringUntil(':');
        assert( tag == "bbox");
        polyline.bbox = parse_bbox( bufferedFile.readStringUntil('\n'));
        line++;
        polyline.points.clear();
        tag = bufferedFile.readStringUntil(':');
        assert( tag == "coords");
        parse_coords( bufferedFile, polyline.points);
        line++;
        mblock->polylines.push_back( polyline);
        total_points += polyline.points.size();
        count--;
    }
    assert( count == 0);
    file.close();
    return mblock;
}


void get_map_blocks( BBox& bbox, MemCache& memCache)
{
    log_d("get_map_blocks %i", millis());
    for( MapBlock* block: memCache.blocks){
        block->inView = false;
    }
    // loop the 4 corners of the bbox and find the files that contain them
    for( Point32 point: { bbox.min, bbox.max, Point32( bbox.min.x, bbox.max.y), Point32( bbox.max.x, bbox.min.y) }){
        bool found = false;
        int32_t block_min_x = point.x & ( ~MAPBLOCK_MASK);
        int32_t block_min_y = point.y & ( ~MAPBLOCK_MASK);
        
        // check if the needed block is already in memory
        for( MapBlock* memblock : memCache.blocks){
            if( block_min_x == memblock->offset.x && block_min_y == memblock->offset.y){
                memblock->inView = true;
                found = true;
                break;
            }
        }
        if( found) continue;
        
        log_d("load from disk (%i, %i) %i", block_min_x, block_min_y, millis());
        // block is not in memory => load from disk
        int32_t block_x = (block_min_x >> MAPBLOCK_SIZE_BITS) & MAPFOLDER_MASK;
        int32_t block_y = (block_min_y >> MAPBLOCK_SIZE_BITS) & MAPFOLDER_MASK;
        int32_t folder_name_x = block_min_x >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS);
        int32_t folder_name_y = block_min_y >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS);
        String file_name = base_folder + folder_name_x +"_"+ folder_name_y +"/"+ block_x +"_"+ block_y; //  /maps/123_456/777_888

        // check if cache is full
        if( memCache.blocks.size() >= MAPBLOCKS_MAX){
            // remove first one, the oldest
            log_v("Deleteing freeHeap: %i", esp_get_free_heap_size());
            MapBlock* first_block = memCache.blocks.front();
            delete first_block; // free memory
            memCache.blocks.erase( memCache.blocks.begin()); // remove pointer from the vector
            log_v("Deleted freeHeap: %i", esp_get_free_heap_size());
        }

        MapBlock* new_block = read_map_block( file_name);
        new_block->inView = true;
        new_block->offset = Point32( block_min_x, block_min_y);
        memCache.blocks.push_back( new_block); // add the block to the memory cache
        assert( memCache.blocks.size() <= MAPBLOCKS_MAX);

        log_d("Block readed from SD card: %p", new_block);
        log_d("FreeHeap: %i", esp_get_free_heap_size());
    
    }   
    log_d("memCache size: %i %i", memCache.blocks.size(), millis());
}


