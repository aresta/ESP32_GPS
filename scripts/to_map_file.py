#!/usr/bin/env python
import json

LINES_INPUT_FILE = '../maps/lines_styled.geojson'
LINES_MAP_FILE = '../maps/lines.map'


with open( LINES_INPUT_FILE, "r") as file:
    features = json.load(file)

# find the minimum x and y, to substract from all coords
min_x = features[0]['coordinates'][0][0]
min_y = features[0]['coordinates'][0][1]
for feat in features:
    for coord in feat['coordinates']:
        min_x = min( min_x, coord[0])
        min_y = min( min_y, coord[1])


with open( LINES_MAP_FILE, "w", encoding='ascii') as file:
    file.write( f"features: {len(features)}\n")
    file.write( f"offset_x: {min_x}\n")
    file.write( f"offset_y: {min_y}\n")
    file.write( f"\n")
    for feat in features:
        file.write( f"{feat['color']}\n")
        file.write( f"{feat['width']}\n")
        file.write( f"{feat['z_order']}\n")
        file.write( f"{len(feat['coordinates'])}\n")
        for coord in feat['coordinates']:
            file.write( f"{round(coord[0] - min_x,1)},{round(coord[1] - min_y,1)};")
        file.write('\n')





