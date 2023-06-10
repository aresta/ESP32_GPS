from shapely import LineString, LinearRing, Polygon, Point, clip_by_rect
import PIL.ImageDraw as ImageDraw
import PIL.Image as Image

# SCREEN_WIDTH, SCREEN_HEIGHT = 480, 640
SCREEN_WIDTH, SCREEN_HEIGHT = 4000, 6000
PIXEL_SIZE = 2 # in meters
BACKGROUND_COLOR = 0xDDDDDD


def parse_tags(tags_str):
    res = dict()
    tags = tags_str.split('","')
    for tag in tags:
        tag = tag.replace('"','')
        parts = tag.split('=>')
        res[parts[0]] = parts[1]
    return res

def round_coord( coord):
    return ( round( coord[0]), round( coord[1]))

def get_coordinates( geom ):
    geom_type = geom['type']
    if geom_type == 'LineString':
        return LineString( geom['coordinates'])
    elif geom_type == 'MultiLineString':
        return LineString([ coord for linestring in geom['coordinates'] for coord in linestring  ]) # flatten the list of lists
    elif geom_type == 'Polygon':
        return LinearRing([ coord for linestring in geom['coordinates'] for coord in linestring  ]) # flatten the list of lists
    elif geom_type == 'MultiPolygon': # TODO: flatten another level
        # print("MultiPolygon len", len(geom['coordinates']))
        # for p in geom['coordinates']: print("    *** len", len(p), p)
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
            if 'tags' in conf and conf['tags'] in tags:
                feature_type = conf['tags'] + '.' + tags[conf['tags']]
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
    print("------------------------")
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
        feature_color = 'pink' # default
        feature_width = None   # default
        found = False
        conf_styles = styles['lines'] if feat['geom_type'] in ('LineString','MultiLineString') else styles['polygons']
        for style_item in conf_styles:
            if feature_type in style_item['features'] or feature_type_group in style_item['features']:
                if 'color' in style_item: feature_color = style_item['color']
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


def draw_feature( draw: ImageDraw, coordinates: LineString, color, width, screen_center: Point ):
    min_x = screen_center.x - PIXEL_SIZE*SCREEN_WIDTH/2
    min_y = screen_center.y - PIXEL_SIZE*SCREEN_HEIGHT/2
    points = [ ( (x-min_x)/PIXEL_SIZE, SCREEN_HEIGHT-(y-min_y)/PIXEL_SIZE ) for x,y in coordinates.coords]

    if   color == 'green':       color = 0xAADDAA
    elif color == 'greenclear':  color = 0xBAEEBA
    elif color == 'greenclear2': color = 0xBCFFBC
    elif color == 'grayclear':   color = 0xBFBFBF
    elif color == 'grayclear2':  color = 0xCFCFCF
    elif color == 'blueclear':   color = 0xBBBBFF
    
    if type( coordinates) == LinearRing:
        draw.polygon( points, fill = color)
        # draw.line( points, fill=0x55BB55)
        # for p in points: draw.point( p, "red")
    else:
        if not width: width = 1
        else:
            width =  round(width/PIXEL_SIZE)
            if width == 0: width = 1
        draw.line( points, fill = color, width = width)
        # draw.point( points[0], "red")
        # draw.point( points[-1], "red")


def render_map( features, screen_center: Point):
    image = Image.new("RGB", (SCREEN_WIDTH, SCREEN_HEIGHT), color=BACKGROUND_COLOR)
    draw = ImageDraw.Draw(image)
    for feat in features:
        draw_feature( draw, feat['coordinates'], feat['color'], feat['width'], screen_center)
    image.save("test.png")