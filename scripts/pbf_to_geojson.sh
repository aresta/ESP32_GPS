#!/bin/bash

# OpenStreetMap uses the WGS84 spatial reference system
# Most tiled web maps (such as the standard OSM maps and Google Maps) use this Mercator projection.
# WGS84 (EPSG 4326) => Mercator (EPSG 3857)


# /maps/osmium extract -p ../conf/clip_bcn.geojson spain-latest.osm.pbf -o /pbf/bcn.pbf

# ogr2ogr -t_srs EPSG:3857 -clipdst 218500 5075000 235000 5095000 -simplify 0.000005 \
#     /maps/points.geojson /pbf/bcn.pbf \
#     points

ogr2ogr -t_srs EPSG:3857 \
    -clipdst 218500 5075000 235000 5095000 \
    /maps/lines.geojson /pbf/bcn.pbf \
    lines
    # -simplify 0.000005 \

# ogr2ogr -t_srs EPSG:3857 -clipdst 218500 5075000 235000 5095000 -simplify 0.000005 \
#     /maps/relations.geojson /pbf/bcn.pbf \
#     multilinestrings # relations...

ogr2ogr -t_srs EPSG:3857 \
    -clipdst 218500 5075000 235000 5095000 \
    /maps/polygons.geojson /pbf/bcn.pbf \
    multipolygons
    # -simplify 0.000005 \



# ogr2ogr -t_srs EPSG:3857 -clipdst 218000 5070000 231000 5100000 -simplify 0.000005  /maps/lines.geojson /pbf/bcn.pbf lines