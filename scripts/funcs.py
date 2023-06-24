from shapely import LineString, LinearRing, Polygon, Point, clip_by_rect
import PIL.ImageDraw as ImageDraw
import PIL.Image as Image
import math

IMG_WIDTH, IMG_HEIGHT = pow( 2, 12), pow( 2, 12) # 4096 x 4096
BACKGROUND_COLOR = 0xDDDDDD

PI = 3.14159265358979323846264338327950288
def DEG2RAD(a): return ((a) / (180 / PI))
def RAD2DEG(a): return ((a) * (180 / PI))
EARTH_RADIUS = 6378137
def lat2y( lat): return round( math.log( math.tan( DEG2RAD(lat) / 2 + PI/4 )) * EARTH_RADIUS)
def lon2x( lon): return round( DEG2RAD(lon) * EARTH_RADIUS)

def parse_tags(tags_str):
    res = dict()
    tags = tags_str.split('","')
    for tag in tags:
        tag = tag.replace('"','')
        parts = tag.split('=>')
        res[parts[0]] = parts[1]
    return res

def flatten_list( multipol):
    res = []
    for p in multipol:
        if type( p[0]) in (int, float):
            res.append( p)
        else:
            res += flatten_list( p)
    return res

def get_coordinates( geom ):
    geom_type = geom['type']
    if geom_type == 'LineString':
        return LineString( geom['coordinates'])
    elif geom_type == 'MultiLineString':
        return LineString( flatten_list( geom['coordinates']))
    elif geom_type == 'Polygon':
        return LineString( flatten_list( geom['coordinates']))
    elif geom_type == 'MultiPolygon': # TODO: flatten another level
        return LineString( flatten_list( geom['coordinates']))
    
    # elif geom_type == 'GeometryCollection': # TODO
    #     return []    
    # else: print("ERROR: unknow geometry type:", geom_type)
    return None

def process_features( features, conf ):
    extracted = []
    for feature in features:
        properties = feature['properties']
        tags = parse_tags( feature['properties']['other_tags'] ) if 'other_tags' in feature['properties'] else dict()
        
        # some features are defined just by a tag, like railway
        # we add them to the properties
        if 'tags' in conf: 
            for tag in conf['tags']:
                if tag in tags: 
                    properties[tag] = tags[tag]

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

        if not feature_type: continue
        coordinates = get_coordinates( feature['geometry'] ) 
        if feature_type and coordinates and not coordinates.is_empty:
            extracted.append({
                'id': properties['osm_id'] if 'osm_id' in properties else '', # for testing/debugging
                'type': feature_type,
                'geom_type': 'line' if feature['geometry']['type'] in ('LineString','MultiLineString') else 'polygon',
                'tags':  feature_type_tags,
                'z_order': z_order,
                'coordinates': coordinates
                })
    
    # print report
    feat_found = set()
    for ext in extracted:
        feat_found.add( ext["type"])
    print("Feature types extracted:")
    for ft in sorted(feat_found):
        print(ft)
    return extracted


def style_features( features, styles):
    styled_features = []
    for feat in features:
        feature_type = feat['type']
        feature_type_group = feat['type'].split('.')[0]
        feature_color = '0xF972' # default pink
        feature_width = None   # default
        found = False
        conf_styles = styles['lines'] if feat['geom_type'] == 'line' else styles['polygons']
        for style_item in conf_styles:
            if feature_type in style_item['features'] or feature_type_group in style_item['features']:
                if 'color' in style_item: feature_color = styles["colors"][ style_item['color']]
                if 'width' in style_item: feature_width = style_item['width']
                found = True
                break # keep first match
        if not found: 
            print("Not mapped: ", feature_type, feature_type_group)
        styled_features.append({
            'id': feat['id'],  # for debugging
            'type': feature_type, # remove
            'geom_type': feat['geom_type'],
            'color': feature_color, 
            'width': feature_width,
            'z_order': feat['z_order'],
            'coordinates': feat['coordinates'],
            })
    return styled_features

def clip_features( features, bbox: Polygon):
    clipped = []
    for feat in features:
        feat_type = type( feat['coordinates'])
        if bbox.intersects( feat['coordinates']):        
            parts = clip_by_rect( feat['coordinates'], * bbox.bounds)
            new_feat = {
                'id': feat['id'],
                'type': feat['type'],
                'geom_type': feat['geom_type'],
                'color': feat['color'],
                'width': feat['width'],
                'z_order': feat['z_order']
            }
            if type(parts) == LinearRing:
                print("**** Parts LinearRing")
                pass
            elif type(parts) == LineString:
                if parts.is_valid and not parts.is_empty and not parts.touches( bbox):
                    new_feat['coordinates'] = parts
                else:
                    print(" *** discarded", feat['id'], parts.is_valid, not parts.is_empty, not parts.touches( bbox))
            else: 
                for l in parts.geoms:
                    if l.is_valid and not l.is_empty and not l.touches( bbox):
                        new_feat['coordinates'] = l
            if feat_type != type(new_feat['coordinates']): # TODO: check
                if len( new_feat['coordinates'].coords) <= 2: continue
                # if feat_type == LinearRing: new_feat['coordinates'] = LinearRing( new_feat['coordinates'].coords)
                # TODO: fix split of polygons in the border of the blocks
            clipped.append( new_feat)
    return clipped


def color_to_24bits( color565):
    color565 = int( color565, 16) # convert from hex string
    r = (color565 >> 8) & 0xF8
    r |= (r >> 5)
    g = (color565 >> 3) & 0xFC
    g |= (g >> 6)
    b = (color565 << 3) & 0xF8
    b |= (b >> 5)
    return (b << 16) | (g << 8) | r  # for some reason it expects the channels in reverse order (bgr)

def draw_feature( draw: ImageDraw, feat, min_x, min_y ):
    points = [ (( x-min_x), IMG_HEIGHT-(y-min_y) ) for x,y in feat['coordinates'].coords]
    color = color_to_24bits( feat['color'])    
    if feat['geom_type'] == 'polygon':
        draw.polygon( points, fill = color)
    else:
        width = max( round( feat['width']), 1) if feat['width'] else 1
        draw.line( points, fill = color, width = width)


def render_map( features, file_name, min_x, min_y):
    image = Image.new("RGB", (IMG_WIDTH, IMG_HEIGHT), color=BACKGROUND_COLOR)
    draw = ImageDraw.Draw(image)
    for feat in features:
        draw_feature( draw, feat, min_x=min_x, min_y=min_y)
    image.save( file_name)