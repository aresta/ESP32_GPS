#!/usr/bin/env python
import json
import PIL.ImageDraw as ImageDraw
import PIL.Image as Image

LINES_INPUT_FILE = '/maps/lines_styled.geojson'
POLYGONS_INPUT_FILE = '/maps/polygons_styled.geojson'
# SCREEN_WIDTH, SCREEN_HEIGHT = 240, 320
SCREEN_WIDTH, SCREEN_HEIGHT = 480, 640
PIXEL_SIZE = 1 # in meters
BACKGROUND_COLOR = (220,220,220)

image = Image.new("RGB", (SCREEN_WIDTH, SCREEN_HEIGHT), color=BACKGROUND_COLOR)
draw = ImageDraw.Draw(image)


loc_x, loc_y = 225713.76, 5085162.01

min_x = loc_x - PIXEL_SIZE*SCREEN_WIDTH/2
min_y = loc_y - PIXEL_SIZE*SCREEN_HEIGHT/2
max_x = loc_x + PIXEL_SIZE*SCREEN_WIDTH/2
max_y = loc_y + PIXEL_SIZE*SCREEN_HEIGHT/2

total_points = 0

def draw_lines( coordinates, color, width ):
    global total_points
    if not color: color = 'pink'
    for coord in coordinates:
        point = ( float(coord[0]), float(coord[1]) )
        if point[0] > min_x and point[0] < max_x and point[1] > min_y and point[1] < max_y:
            points = [ ( (x-min_x)/PIXEL_SIZE, SCREEN_HEIGHT-(y-min_y)/PIXEL_SIZE ) for x,y in coordinates]
            # print(points)
            draw.line( points, fill=color, width=round(width/PIXEL_SIZE))
            total_points += len( coordinates)
            continue

def draw_polygons( coordinates, color ):
    global total_points
    if not color: color = BACKGROUND_COLOR
    if ',' in color: # tuple with color components
        color = tuple(map(int, color.split(',')))
    for coord in coordinates:
        # print(coord)
        if isinstance(coord[0], list): continue # TODO
        point = ( float(coord[0]), float(coord[1]) )
        if point[0] > min_x and point[0] < max_x and point[1] > min_y and point[1] < max_y:
            points = [ ( (x-min_x)/PIXEL_SIZE, SCREEN_HEIGHT-(y-min_y)/PIXEL_SIZE ) for x,y in coordinates]
            draw.polygon( points, fill=color)
            total_points += len( coordinates)
            continue


with open( POLYGONS_INPUT_FILE, "r") as file:
    features = json.load(file)
for feature in features:
    draw_polygons( feature['coordinates'], feature['color'] )

print("total_points",total_points)

with open( LINES_INPUT_FILE, "r") as file:
    features = json.load(file)
for feature in features:
    draw_lines( feature['coordinates'], feature['color'], feature['width'] )
    
image.save("test.png")

print("total_points",total_points)