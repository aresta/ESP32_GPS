#!/usr/bin/env python
import json
from shapely import Point,  box
from funcs import process_features, clip_features, style_features, render_map


LINES_INPUT_FILE = '/maps/lines.geojson'
POLYGONS_INPUT_FILE = '/maps/polygons.geojson'
CONF_FEATURES = '/conf/conf_extract.json'
CONF_STYLES = '/conf/conf_styles.json'
OUPUT_FILE = '/maps/polygons_styled.geojson'

MAPBLOCK_SIZE_BITS = 12 
MAPFILE_SIZE_BITS = 16 

init_x, init_y = 226502.0, 5084319.0

initial_point = Point( init_x, init_y)

mapblock_mask = pow( 2, MAPBLOCK_SIZE_BITS) - 1     # ...0000111111111111
mapfile_mask = pow( 2, MAPFILE_SIZE_BITS) - 1       # ...000011111111111111
mapblock_offset = Point( round( init_x) & (~mapblock_mask), round( init_y) & (~mapblock_mask)) # clean the last 12 bits
mapfile_offset = Point(  round( init_x) & (~mapfile_mask), round( init_y) & (~mapfile_mask))  # clean the last 16 bits

mapblock_bbox = box( mapblock_offset.x, mapblock_offset.y, mapblock_offset.x + mapblock_mask, mapblock_offset.y + mapblock_mask)
print("mapblock_bbox", mapblock_bbox)

conf = json.load( open( CONF_FEATURES, "r"))
styles = json.load( open(CONF_STYLES, "r"))

# extract
lines = json.load( open( LINES_INPUT_FILE, "r"))
polygons = json.load( open( POLYGONS_INPUT_FILE, "r"))
extracted_polygons = process_features( polygons['features'], conf["polygons"])
extracted_lines = process_features( lines['features'], conf["lines"])
clipped = clip_features( extracted_polygons + extracted_lines, mapblock_bbox)

# apply styles
styled_features = style_features( clipped, styles)

render_map( styled_features, initial_point)



# json.dump( styled_features, open( OUPUT_FILE, "w"))




