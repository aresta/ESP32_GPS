#ifdef ARDUINO
    #include <Arduino.h>
#else
    #include <logs.h>
    #include <cassert>
#endif

#include <files.h>
#include <graphics.h>
#include <canvas.h>
#include "../conf.h"

#include "maps.h"

// @brief Returns the int16 or 0 if empty
int16_t parse_int16(IReadStream& file)
{
    char num[16];
    uint8_t i;
    char c;
    i=0;
    c = (char )file.read();
    if( c == '\n') return 0;
    while( c>='0' && c <= '9'){
        assert( i < 15);
        num[i++] = c;
        c = (char )file.read();
    }
    num[i] = '\0';
    if( c != ';' && c != ',' && c != '\n'){
        log_e("parse_int16 error: %c %i", c, c);
        log_e("Num: [%s]", num);
        while(1);
    }
    try{
        return std::stoi( num);
    } catch( std::invalid_argument){
        log_e("parse_int16 invalid_argument: [%c] [%s]", c, num);
    } catch( std::out_of_range){
        log_e("parse_int16 out_of_range: [%c] [%s]", c, num);
    }
    return -1;
}

// @brief Returns the string until terminator char or newline. The terminator character is not included but comsumed from stream.
void parse_str_until(IReadStream& file, char terminator, char *str)
{
    uint8_t i;
    char c;
    i=0;
    c = (char )file.read();  
    while( c != terminator && c != '\n'){
        assert( i < 29);
        str[i++] = c;
        c = (char )file.read();
    }
    str[i] = '\0';
}

void parse_coords(IReadStream& file, std::vector<Point16>& points)
{
    char str[30];
    assert(points.size() == 0);
    Point16 point;
    while( true){
        try{
            parse_str_until( file, ',', str);
            if( str[0] == '\0') break;
            point.x = (int16_t )std::stoi( str);
            parse_str_until( file, ';', str);
            assert( str[0] != '\0');
            point.y = (int16_t )std::stoi( str);
            // log_d("point: %i %i", point.x, point.y);
        } catch( std::invalid_argument){
            log_e("parse_coords invalid_argument: %s", str);
        } catch( std::out_of_range){
            log_e("parse_coords out_of_range: %s", str);
        }
        points.push_back( point);
    }
    // points.shrink_to_fit();
}

void read_map_block(IReadStream& file, MapBlock* result)
{ 
    char c;
    char str[30];
    uint32_t line = 0;

    // read polygons
    parse_str_until(file, ':', str);

    if (strcmp( str, "Polygons") != 0) {
        log_e("Map error. Expected Polygons instead of: %s", str);
        while(0);
    }

    int16_t count = parse_int16( file);
    assert(count > 0);
    line++;
    log_d("count: %i", count);

    uint32_t total_points = 0;
    Polygon polygon;
    Point16 p;
    int16_t maxzoom;
    
    while (count > 0) {
        // log_d("line: %i", line);
        parse_str_until( file, '\n', str); // color
        assert( str[0] == '0' && str[1] == 'x');
        polygon.color = (uint16_t )std::stoul( str, nullptr, 16);
        // log_d("polygon.color: %i", polygon.color);
        line++;
        parse_str_until( file, '\n', str); // maxzoom
        polygon.maxzoom = str[0] ? (uint8_t )std::stoi( str) : MAX_ZOOM;
        // log_d("polygon.maxzoom: %i", polygon.maxzoom);
        line++;

        parse_str_until( file, ':', str);

        if (strcmp( str, "bbox") != 0){
            log_e("bbox error tag. Line %i : %s", line, str);
            while(true);
        }

        polygon.bbox.min.x = parse_int16( file);
        polygon.bbox.min.y = parse_int16( file);
        polygon.bbox.max.x = parse_int16( file);
        polygon.bbox.max.y = parse_int16( file);

        line++;
        polygon.points.clear();
        parse_str_until( file, ':', str);
        if( strcmp( str, "coords") != 0){
            log_e("coords error tag. Line %i : %s", line, str);
            while(true);
        }
        parse_coords( file, polygon.points);
        line++;
        result->polygons.push_back( polygon);
        total_points += polygon.points.size();
        count--;
    }
    assert( count == 0);
    
    // read lines
    parse_str_until( file, ':', str);

    if( strcmp( str, "Polylines") != 0) 
        log_e("Map error. Expected Polylines instead of: %s", str);

    count = parse_int16( file);
    assert( count > 0);
    line++;
    log_d("count: %i", count);
    
    Polyline polyline;
    while (count > 0) {
        // log_d("line: %i", line);
        parse_str_until( file, '\n', str); // color
        assert( str[0] == '0' && str[1] == 'x');
        polyline.color = (uint16_t )std::stoul( str, nullptr, 16);
        line++;
        parse_str_until( file, '\n', str); // width
        polyline.width = str[0] ? (uint8_t )std::stoi( str) : 1;
        line++;
        parse_str_until( file, '\n', str); // maxzoom
        polyline.maxzoom = str[0] ? (uint8_t )std::stoi( str) : MAX_ZOOM;
        line++;

        parse_str_until( file, ':', str);
        if( strcmp( str, "bbox") != 0){
            log_e("bbox error tag. Line %i : %s", line, str);
            while(true);
        }

        polyline.bbox.min.x = parse_int16( file);
        polyline.bbox.min.y = parse_int16( file);
        polyline.bbox.max.x = parse_int16( file);
        polyline.bbox.max.y = parse_int16( file);

        // if( line > 4050){
        //     log_e("polyline.bbox %i %i %i %i", polyline.bbox.min.x, polyline.bbox.min.y,polyline.bbox.max.x, polyline.bbox.max.y);
        // }
        line++;

        polyline.points.clear();
        parse_str_until( file, ':', str);
        if( strcmp( str, "coords") != 0){
            log_d("coords tag. Line %i : %s", line, str);
            while(true);
        }
        parse_coords( file, polyline.points);
        line++;
        // if( line > 4050 && file_name == "/mymap/3_77/6_9"){
        //     for( Point16 p: polyline.points){
        //         log_d("p.x, p.y %i %i", p.x, p.y);
        //     }
        // }
        result->polylines.push_back(polyline);
        total_points += polyline.points.size();
        count--;
    }
    assert(count == 0);
}

void get_map_blocks(const IFileSystem* fileSystem, BBox& bbox, MemCache& memCache)
{
    log_d("get_map_blocks %i", millis());

    for (MapBlock* block: memCache.blocks){
        block->inView = false;
    }

    // loop the 4 corners of the bbox and find the files that contain them
    for (Point32 point : { bbox.min, bbox.max, Point32(bbox.min.x, bbox.max.y), Point32( bbox.max.x, bbox.min.y) })
    {
        bool found = false;
        int32_t block_min_x = point.x & ( ~MAPBLOCK_MASK);
        int32_t block_min_y = point.y & ( ~MAPBLOCK_MASK);
        
        // check if the needed block is already in memory
        for (MapBlock* memblock : memCache.blocks){
            if( block_min_x == memblock->offset.x && block_min_y == memblock->offset.y){
                memblock->inView = true;
                found = true;
                break;
            }
        }

        if (found) 
        {
            continue;
        }
        
        log_d("load from disk (%i, %i) %i", block_min_x, block_min_y, millis());
        // block is not in memory => load from disk
        int32_t block_x = (block_min_x >> MAPBLOCK_SIZE_BITS) & MAPFOLDER_MASK;
        int32_t block_y = (block_min_y >> MAPBLOCK_SIZE_BITS) & MAPFOLDER_MASK;
        int32_t folder_name_x = block_min_x >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS);
        int32_t folder_name_y = block_min_y >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS);
        char folder_name[12];
        snprintf( folder_name, 9, "%+04d%+04d", folder_name_x, folder_name_y); // force sign and 4 chars per number

        char file_name[100];
        snprintf(file_name, sizeof(file_name), "%s/%d/%d.fmp", folder_name, block_x, block_y);
        //String file_name = String("") + folder_name + "/" + block_x + "_" + block_y + ".fmp"; //  /maps/123_456/777_888

        // check if cache is full
        if (memCache.blocks.size() >= MAPBLOCKS_MAX)
        {
            // remove first one, the oldest
            log_v("Deleteing freeHeap: %i", esp_get_free_heap_size());
            MapBlock* first_block = memCache.blocks.front();
            delete first_block; // free memory
            memCache.blocks.erase( memCache.blocks.begin()); // remove pointer from the vector
            log_v("Deleted freeHeap: %i", esp_get_free_heap_size());
        }

        auto new_block = new MapBlock();
        auto stream = fileSystem->Open(file_name);

        if (!stream) {
            snprintf(file_name, sizeof(file_name), "Map file not found: %s/%d/%d.fmp", folder_name, block_x, block_y);
            tft_header_msg(file_name);
            while(true);
        }

        read_map_block(*stream, new_block);

        delete stream;

        new_block->inView = true;
        new_block->offset = Point32( block_min_x, block_min_y);
        memCache.blocks.push_back( new_block); // add the block to the memory cache

        assert(memCache.blocks.size() <= MAPBLOCKS_MAX);

        log_d("Block readed from SD card: %p", new_block);
        log_d("FreeHeap: %i", esp_get_free_heap_size());
    }   

    //log_d("memCache size: %i %i", memCache.blocks.size(), millis());
}
