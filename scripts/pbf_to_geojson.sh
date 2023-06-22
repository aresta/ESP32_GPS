#!/bin/bash

# OpenStreetMap uses the WGS84 spatial reference system
# Most tiled web maps (such as the standard OSM maps and Google Maps) use this Mercator projection.
# WGS84 (EPSG 4326) => Mercator (EPSG 3857)


# /maps/osmium extract -p ../conf/clip_bcn.geojson spain-latest.osm.pbf -o /pbf/bcn.pbf

rm /maps/lines.geojson
rm /maps/polygons.geojson

ogr2ogr -t_srs EPSG:3857 \
    -clipsrc 1.83 41.31 2.28 41.70 \
    /maps/lines.geojson /pbf/bcn.pbf \
    lines

ogr2ogr -t_srs EPSG:3857 \
    -clipsrc 1.83 41.31 2.28 41.70 \
    /maps/polygons.geojson /pbf/bcn.pbf \
    multipolygons
