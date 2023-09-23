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
    if( count <= 0){
        header_msg("Error: wrong number of poligons: " + count);
        while(true);
    }
    int line = 5;
    int total_points = 0;
    Polygon polygon;
    while( count > 0 && bufferedFile.available()){
        polygon.color = std::stoul( bufferedFile.readStringUntil('\n').c_str(), nullptr, 16);
        line++;
        polygon.maxzoom = bufferedFile.readStringUntil('\n').toInt() ?: MAX_ZOOM;
        line++;
        polygon.points.clear();
        parse_coords( bufferedFile, polygon.points);
        line++;
        mblock->polygons.push_back( polygon);
        total_points += polygon.points.size();
        count--;
    }
    if( count != 0){
        header_msg("ERROR: Polygons count don't match");
        while(true);
    }
    mblock->polygons.shrink_to_fit();

    // read lines
    feature_type = bufferedFile.readStringUntil(':');
    if( feature_type != "Polylines") log_e("Map error. Expected Polylines instead of: %s", feature_type);
    count = bufferedFile.readStringUntil('\n').toInt();
    if( count <= 0){
        header_msg("Error: wrong number of lines: " + count);
        while(true);
    }
    Polyline polyline;
    while( count > 0 && bufferedFile.available()){
        polyline.color = std::stoul( bufferedFile.readStringUntil('\n').c_str(), nullptr, 16);
        line++;
        polyline.width = bufferedFile.readStringUntil('\n').toInt() ?: 1;
        line++;
        polyline.maxzoom = bufferedFile.readStringUntil('\n').toInt() ?: MAX_ZOOM;
        line++;
        polyline.points.clear();
        parse_coords( bufferedFile, polyline.points);
        line++;
        mblock->polylines.push_back( polyline);
        total_points += polyline.points.size();
        count--;
    }
    if( count != 0){
        header_msg("ERROR: Lines count don't match");
        while(true);
    }
    mblock->polylines.shrink_to_fit();
    file.close();
    return mblock;
}


void get_map_blocks( BBox& bbox, MemCache& memCache)
{
    log_d("get_map_blocks %i", millis());
    for( MapBlock* block: memCache.blocks){
        // log_d("Block: %p", block);
        if( block) block->inView = false;
        else break;
    }
    // loop the 4 corners of the bbox and find the files that contain them
    for( Point32 point: { bbox.min, bbox.max, Point32( bbox.min.x, bbox.max.y), Point32( bbox.max.x, bbox.min.y) }){
        bool found = false;
        int32_t block_min_x = point.x & ( ~mapblock_mask);
        int32_t block_min_y = point.y & ( ~mapblock_mask);
        
        // check if the needed block is already in memory
        for( MapBlock* memblock : memCache.blocks){
            if( ! memblock) break;
            if( block_min_x == memblock->offset.x && block_min_y == memblock->offset.y){
                memblock->inView = true;
                found = true;
                break;
            }
        }
        if( found) continue;
        
        log_d("load from disk %i", millis());
        // block is not in memory => load from disk
        int32_t block_x = (block_min_x >> MAPBLOCK_SIZE_BITS) & mapfolder_mask;
        int32_t block_y = (block_min_y >> MAPBLOCK_SIZE_BITS) & mapfolder_mask;
        int32_t folder_name_x = block_min_x >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS);
        int32_t folder_name_y = block_min_y >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS);
        String file_name = base_folder + folder_name_x +"_"+ folder_name_y +"/"+ block_x +"_"+ block_y; //  /maps/123_456/777_888

        // check if cache is full
        if( memCache.size >= MAPBLOCKS_MAX){
            // remove the first block (the oldest) and shift the rest down
            delete memCache.blocks[0];
            for( int i=0; i < memCache.size-1; i++){
                memCache.blocks[i] = memCache.blocks[i+1];
            }
            memCache.size--;
            memCache.blocks[memCache.size] = NULL;
        }

        MapBlock* new_block = read_map_block( file_name);
        new_block->inView = true;
        new_block->offset = Point32( block_min_x, block_min_y);
        assert( !memCache.blocks[ memCache.size]);
        memCache.blocks[ memCache.size] = new_block; // add the block to the memory cache
        memCache.size++;
        assert( memCache.size <= MAPBLOCKS_MAX);

        log_d("Block readed from SD card: %p", new_block);
        log_d("FreeHeap: %i", esp_get_free_heap_size());
    
    }   
    log_d("memCache size: %i %i", memCache.size, millis());
}


