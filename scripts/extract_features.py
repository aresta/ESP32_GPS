#!/usr/bin/env python
import json

LINES_INPUT_FILE = '/maps/lines.geojson'
LINES_OUPUT_FILE = '/maps/lines_extracted.geojson'
POLYGONS_INPUT_FILE = '/maps/polygons.geojson'
POLYGONS_OUPUT_FILE = '/maps/polygons_extracted.geojson'
CONF_FEATURES = '/conf/conf_extract.json'

def parse_tags(tags_str):
    res = dict()
    tags = tags_str.split('","')
    for tag in tags:
        tag = tag.replace('"','')
        parts = tag.split('=>')
        res[parts[0]] = parts[1]
    return res

def get_coordinates( geom ):
    geom_type = geom['type']
    if geom_type == 'LineString':
        return geom['coordinates']
    elif geom_type in ('MultiLineString','Polygon'):
        return [ coord for linestring in geom['coordinates'] for coord in linestring  ] # flatten the list of lists
    elif geom_type == 'MultiPolygon': # TODO: flatten another level
        return [] 
    elif geom_type == 'GeometryCollection': # TODO
        return []    
    else: print("ERROR: unknow geometry type:", geom_type)
    return None

def process_features( features, conf ):
    extracted = []
    for feature in features:
        properties = feature['properties']
        tags = parse_tags( feature['properties']['other_tags'] ) if 'other_tags' in feature['properties'] else dict()
        feature_type = None
        feature_type_tags = []
        z_order = properties['z_order'] if 'z_order' in properties else None
        for conf_feat_type in conf['feature_types']:
            if conf_feat_type in properties:
                feat_subtype = properties[ conf_feat_type ]
                filter_by_subtype = (len( conf['feature_types'][conf_feat_type]) > 0) 
                if filter_by_subtype and not feat_subtype in conf['feature_types'][conf_feat_type]: continue
                feature_type = conf_feat_type + '.' + feat_subtype
                if isinstance( conf['feature_types'][conf_feat_type], list): break # no tags to check, we are done
                conf_feature_tags = conf['feature_types'][conf_feat_type][feat_subtype]
                for feat_subtype_tag in conf_feature_tags:
                    if feat_subtype_tag in tags:
                        feature_type_tags.append( feat_subtype_tag + '.' + tags[feat_subtype_tag])
                break
        if not feature_type:
            if 'tags' in conf and conf['tags'] in tags:
                feature_type = conf['tags'] + '.' + tags[conf['tags']]
        coordinates = [[round(c[0],1),round(c[1],1)] for c in get_coordinates( feature['geometry'] )] # round to 1 decimal

        if feature_type:
            extracted.append({
                "type": feature_type,
                "tags":  feature_type_tags,
                "z_order": z_order,
                "geom_type": feature['geometry']['type'],
                "coordinates": coordinates
                })
            # print( feature_type, feature_type_tags)
    return extracted


with open( CONF_FEATURES, "r") as file:
    conf = json.load(file)

with open( LINES_INPUT_FILE, "r") as file:
    lines = json.load(file)
extracted = process_features( lines['features'], conf["lines"])
with open( LINES_OUPUT_FILE, "w") as file:
    json.dump( extracted, file)

with open( POLYGONS_INPUT_FILE, "r") as file:
    polygons = json.load(file)
extracted = process_features( polygons['features'], conf["polygons"])    
with open( POLYGONS_OUPUT_FILE, "w") as file:
    json.dump( extracted, file)



