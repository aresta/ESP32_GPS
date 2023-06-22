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


def get_coordinates( geom ):
    geom_type = geom['type']
    if geom_type == 'LineString':
        return LineString( geom['coordinates'])
    elif geom_type == 'MultiLineString':
        return LineString([ coord for linestring in geom['coordinates'] for coord in linestring  ]) # flatten the list of lists
    elif geom_type == 'Polygon':
        return LinearRing([ coord for linestring in geom['coordinates'] for coord in linestring  ]) # flatten the list of lists
    elif geom_type == 'MultiPolygon': # TODO: flatten another level
        return None 
    # elif geom_type == 'GeometryCollection': # TODO
    #     return []    
    # else: print("ERROR: unknow geometry type:", geom_type)
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
            if 'tags' in conf:
                for tag in conf['tags']:
                    if tag in tags:
                        feature_type = tag + '.' + tags[tag]
                        break
            else:
                continue
        coordinates = get_coordinates( feature['geometry'] ) 

        if feature_type and coordinates and not coordinates.is_empty:
            extracted.append({
                "type": feature_type,
                "geom_type": feature['geometry']['type'],
                "tags":  feature_type_tags,
                "z_order": z_order,
                "coordinates": coordinates
                })
            # print( feature_type, feature_type_tags)
    
    # print report
    # feat_found = set()
    # for ext in extracted:
    #     feat_found.add( ext["type"])
    # print("Feature types extracted:")
    # for ft in sorted(feat_found):
    #     print(ft)
    return extracted

def clip_features( features, bbox: Polygon):
    clipped = []
    for feat in features:
        feat_type = type( feat['coordinates'])
        if bbox.intersects( feat['coordinates']):        
            parts = clip_by_rect( feat['coordinates'], * bbox.bounds)
            new_feat = feat
            if type(parts) == LinearRing:
                pass
            elif type(parts) == LineString:
                if parts.is_valid and not parts.is_empty and not parts.touches( bbox):
                    new_feat['coordinates'] = parts
            else: 
                for l in parts.geoms:
                    if l.is_valid and not l.is_empty and not l.touches( bbox):
                        new_feat['coordinates'] = l
            if feat_type != type(new_feat['coordinates']):
                if len( new_feat['coordinates'].coords) <= 2: continue
                # if feat_type == LinearRing: new_feat['coordinates'] = LinearRing( new_feat['coordinates'].coords)
                # TODO: fix split of polygons in the border of the blocks
            clipped.append( new_feat)
    return clipped
   

def style_features( features, styles):
    styled_features = []
    for feat in features:
        feature_type = feat['type']
        feature_type_group = feat['type'].split('.')[0]
        feature_color = '0xF972' # default pink
        feature_width = None   # default
        found = False
        conf_styles = styles['lines'] if feat['geom_type'] in ('LineString','MultiLineString') else styles['polygons']
        for style_item in conf_styles:
            if feature_type in style_item['features'] or feature_type_group in style_item['features']:
                if 'color' in style_item: feature_color = styles["colors"][ style_item['color']]
                if 'width' in style_item: feature_width = style_item['width']
                found = True
                break # keep first match
        if not found: 
            print("Not mapped: ", feature_type, feature_type_group)
        styled_features.append({
            "type": feature_type, # remove
            "geom_type": feat['geom_type'],
            "color": feature_color, 
            "width": feature_width,
            "z_order": feat['z_order'],
            "coordinates": feat['coordinates'],
            })
    return styled_features

def color_to_24bits( color565):
    color565 = int( color565, 16) # convert from hex string
    r = (color565 >> 8) & 0xF8
    r |= (r >> 5)
    g = (color565 >> 3) & 0xFC
    g |= (g >> 6)
    b = (color565 << 3) & 0xF8
    b |= (b >> 5)
    return (r << 16) | (g << 8) | b

def draw_feature( draw: ImageDraw, coordinates: LineString, color, width, min_x, min_y ):
    points = [ (( x-min_x), IMG_HEIGHT-(y-min_y) ) for x,y in coordinates.coords]
    # fix this, convert from rgb565 to rgb888
    color = color_to_24bits( color)
    
    if type( coordinates) == LinearRing:
        draw.polygon( points, fill = color)
    else:
        width = max( round( width), 1) if width else 1
        draw.line( points, fill = color, width = width)


def render_map( features, file_name, min_x, min_y):
    image = Image.new("RGB", (IMG_WIDTH, IMG_HEIGHT), color=BACKGROUND_COLOR)
    draw = ImageDraw.Draw(image)
    for feat in features:
        draw_feature( draw, feat['coordinates'], feat['color'], feat['width'], min_x=min_x, min_y=min_y)
    image.save( file_name)