#!/usr/bin/env python
import json
import os
from shapely import Point,  box, LinearRing
from funcs import process_features, clip_features, style_features, render_map, lat2y, lon2x


LINES_INPUT_FILE = '/maps/lines.geojson'
POLYGONS_INPUT_FILE = '/maps/polygons.geojson'
CONF_FEATURES = '/conf/conf_extract.json'
CONF_STYLES = '/conf/conf_styles.json'
MAP_FOLDER = '/maps/mymap'

MAPBLOCK_SIZE_BITS = 12     # 4096 x 4096 coords (~meters) per block  
MAPFOLDER_SIZE_BITS = 4     # 16 x 16 map blocks per folder
mapblock_mask  = pow( 2, MAPBLOCK_SIZE_BITS) - 1     # ...00000000111111111111
mapfolder_mask = pow( 2, MAPFOLDER_SIZE_BITS) - 1    # ...00001111

conf = json.load( open( CONF_FEATURES, "r"))
styles = json.load( open(CONF_STYLES, "r"))

# min_lat, min_lon = 41.41, 1.97
# max_lat, max_lon = 41.54, 2.11
min_lat, min_lon = 41.31, 1.83
max_lat, max_lon = 41.70, 2.28
area_min_x, area_min_y = lon2x( min_lon), lat2y( min_lat)
area_max_x, area_max_y = lon2x( max_lon), lat2y( max_lat)

lines = json.load( open( LINES_INPUT_FILE, "r"))
polygons = json.load( open( POLYGONS_INPUT_FILE, "r"))

# extract relevant features
print("Extracting features")
extracted_lines = process_features( lines['features'], conf["lines"])
extracted_polygons = process_features( polygons['features'], conf["polygons"])
print("Applying styles")
# apply styles
styled_lines = style_features( extracted_lines, styles)
styled_polygons = style_features( extracted_polygons, styles)

print("Building map")
for init_x in range(area_min_x, area_max_x, 4096):
    for init_y in range(area_min_y, area_max_y, 4096):
        print("--------------------")
        print("init_x, init_y", init_x, init_y)
        min_x = init_x & (~mapblock_mask)
        min_y = init_y & (~mapblock_mask)
        # mapblock_offset = Point( round( init_x) & (~mapblock_mask), round( init_y) & (~mapblock_mask)) # clean the last 12 bits
        mapblock_bbox = box( min_x, min_y, min_x + mapblock_mask, min_y + mapblock_mask)

        # clip
        clipped_lines = clip_features( styled_lines, mapblock_bbox)
        clipped_polygons = clip_features( styled_polygons, mapblock_bbox)
        if len(clipped_lines) == 0 and len( clipped_polygons) == 0:
            continue

        # export map files
        features, points = 0,0

        block_x = (min_x >> MAPBLOCK_SIZE_BITS) & mapfolder_mask
        block_y = (min_y >> MAPBLOCK_SIZE_BITS) & mapfolder_mask
        folder_name_x = min_x >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS)
        folder_name_y = min_y >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS)
        file_name = f"{MAP_FOLDER}/{folder_name_x}_{folder_name_y}/{block_x}_{block_y}"
        os.makedirs(f"{MAP_FOLDER}/{folder_name_x}_{folder_name_y}", exist_ok=True)
        print(f"File: {file_name}.fmp")

        os.makedirs(f"./test_imgs", exist_ok=True)
        render_map( features = clipped_polygons + clipped_lines, 
                file_name=f"./test_imgs/test_{folder_name_x}_{folder_name_y}-{block_x}_{block_y}.png", 
                min_x=min_x, min_y=min_y)

        # TODO: order features by z_order, first the ones to be drawn below the others
        with open( f"{file_name}.fmp", "w", encoding='ascii') as file:
            file.write( f"Polygons:{len(clipped_polygons)}\n")
            for feat in clipped_polygons:
                # file.write( f"id:{feat['id']}\n")  # TODO: remove!
                file.write( f"{feat['color']}\n")
                for coord in feat['coordinates'].coords:
                    file.write( f"{int(round(coord[0] - min_x))},{int(round(coord[1] - min_y))};")
                    points += 1
                file.write('\n')
                features += 1
            print("Lines, points: ", features, points)

            features, points = 0,0
            file.write( f"Polylines:{len(clipped_lines)}\n")
            for feat in clipped_lines:
                # file.write( f"id:{feat['id']}\n")  # TODO: remove!
                file.write( f"{feat['color']}\n")
                file.write( f"{feat['width']}\n")
                for coord in feat['coordinates'].coords:
                    file.write( f"{int(round(coord[0] - min_x))},{int(round(coord[1] - min_y))};")
                    points += 1
                file.write('\n')
                features += 1
            print("Polygons, points: ", features, points)




