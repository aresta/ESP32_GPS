#!/usr/bin/env python
import json
import os
from shapely import Point,  box, LinearRing
from funcs import process_features, clip_features, style_features, render_map


LINES_INPUT_FILE = '/maps/lines.geojson'
POLYGONS_INPUT_FILE = '/maps/polygons.geojson'
CONF_FEATURES = '/conf/conf_extract.json'
CONF_STYLES = '/conf/conf_styles.json'

MAPBLOCK_SIZE_BITS = 12     # 4096 x 4096 coords (~meters) per block  
MAPFOLDER_SIZE_BITS = 4     # 16 x 16 map blocks per folder
mapblock_mask  = pow( 2, MAPBLOCK_SIZE_BITS) - 1     # ...00000000111111111111
mapfolder_mask = pow( 2, MAPFOLDER_SIZE_BITS) - 1    # ...00001111

conf = json.load( open( CONF_FEATURES, "r"))
styles = json.load( open(CONF_STYLES, "r"))

init_x, init_y = 226502, 5084319

for (init_x, init_y) in [ 
        (226502,        5084319),
        (226502 + 4096, 5084319),
        (226502,        5084319 + 4096),
        (226502 - 4096, 5084319),
        (226502,        5084319 - 4096),
        (226502 + 4096, 5084319 + 4096),
        (226502 - 4096, 5084319 - 4096),
        (226502 + 4096, 5084319 - 4096),
        (226502 - 4096, 5084319 + 4096),
        ]:
    min_x = init_x & (~mapblock_mask)
    min_y = init_y & (~mapblock_mask)
    # mapblock_offset = Point( round( init_x) & (~mapblock_mask), round( init_y) & (~mapblock_mask)) # clean the last 12 bits
    mapblock_bbox = box( min_x, min_y, min_x + mapblock_mask, min_y + mapblock_mask)

    # extract
    lines = json.load( open( LINES_INPUT_FILE, "r"))
    polygons = json.load( open( POLYGONS_INPUT_FILE, "r"))
    extracted_polygons = process_features( polygons['features'], conf["polygons"])
    extracted_lines = process_features( lines['features'], conf["lines"])
    clipped = clip_features( extracted_polygons + extracted_lines, mapblock_bbox)

    # apply styles
    styled_features = style_features( clipped, styles)


    # export map files
    features, points = 0,0

    block_x = (min_x >> MAPBLOCK_SIZE_BITS) & mapfolder_mask
    block_y = (min_y >> MAPBLOCK_SIZE_BITS) & mapfolder_mask
    folder_name_x = min_x >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS)
    folder_name_y = min_y >> (MAPFOLDER_SIZE_BITS + MAPBLOCK_SIZE_BITS)
    file_name = f"/maps/{folder_name_x}_{folder_name_y}/{block_x}_{block_y}"
    os.makedirs(f"/maps/{folder_name_x}_{folder_name_y}", exist_ok=True)
    print(f"Files: {file_name}.lmf and .pmf")

    render_map( features = styled_features, file_name=f"test_{folder_name_x}_{folder_name_y}-{block_x}_{block_y}.png", screen_center = Point( init_x, init_y))

    with open( f"{file_name}.lmf", "w", encoding='ascii') as file:
        file.write( f"offset_x: {min_x}\n")
        file.write( f"offset_y: {min_y}\n")
        file.write( f"\n")
        for feat in styled_features:
            if type( feat['coordinates']) == LinearRing: continue
            file.write( f"{feat['color']}\n")
            file.write( f"{feat['width']}\n")
            file.write( f"{feat['z_order']}\n")
            file.write( f"{len(feat['coordinates'].coords)}\n")
            for coord in feat['coordinates'].coords:
                file.write( f"{int(round(coord[0] - min_x))},{int(round(coord[1] - min_y))};")
                points += 1
            file.write('\n')
            features += 1
    print("Lines, points: ", features, points)

    features, points = 0,0
    with open( f"{file_name}.pmf", "w", encoding='ascii') as file:
        file.write( f"offset_x: {min_x}\n")
        file.write( f"offset_y: {min_y}\n")
        file.write( f"\n")
        for feat in styled_features:
            if type( feat['coordinates']) != LinearRing: continue
            file.write( f"{feat['color']}\n")
            file.write( f"{feat['z_order']}\n")
            file.write( f"{len(feat['coordinates'].coords)}\n")
            for coord in feat['coordinates'].coords:
                file.write( f"{int(round(coord[0] - min_x))},{int(round(coord[1] - min_y))};")
                points += 1
            file.write('\n')
            features += 1
    print("Polygons, points: ", features, points)




