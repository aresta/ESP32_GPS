#!/usr/bin/env python
import json

LINES_INPUT_FILE = '/maps/lines_extracted.geojson'
LINES_OUPUT_FILE = '/maps/lines_styled.geojson'
POLYGONS_INPUT_FILE = '/maps/polygons_extracted.geojson'
POLYGONS_OUPUT_FILE = '/maps/polygons_styled.geojson'
CONF_STYLES = '/conf/conf_styles.json'

with open( CONF_STYLES, "r") as file:
    styles = json.load(file)

def process_features( features, styles ):
    styled_features = []
    for feature in features:
        feature_type = feature['type']
        feature_type_group = feature['type'].split('.')[0]
        feature_color = 'pink' # default
        feature_width = None   # default
        for style_item in styles:
            if feature_type in style_item['features'] or feature_type_group in style_item['features']:
                if 'color' in style_item: feature_color = style_item['color']
                if 'width' in style_item: feature_width = style_item['width']
        # print(feature['type'], feature_color, feature_width)
        styled_features.append({
            "type": feature_type, # remove
            "color": feature_color, 
            "width": feature_width,
            "z_order": feature['z_order'],
            "coordinates": feature['coordinates'],
            })
    return styled_features

with open( LINES_INPUT_FILE, "r") as file:
    features = json.load(file)
with open( LINES_OUPUT_FILE, "w") as file:
    json.dump( 
        process_features( features, styles["lines"] ), 
        file)

with open( POLYGONS_INPUT_FILE, "r") as file:
    features = json.load(file)
with open( POLYGONS_OUPUT_FILE, "w") as file:
    json.dump( 
        process_features( features, styles["polygons"] ), 
        file)



