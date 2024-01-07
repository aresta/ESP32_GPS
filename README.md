# ESP32_GPS

The goal is to build a portable GPS device that displays a map with your position in a small display. The map data is extracted from the OpenStreetMap project data and reduced to only the basic elements for a clear navigation, e.g. while mountain biking.  The scripts extracts basically polylines and polygons to display the roads, paths, rivers, buildings, forest, etc.

To download and extract the map files of your country/area see: https://github.com/aresta/OSM_Extract. It is still work in progress, but it's already funtional. Once you extract and generate the map files copy them into the SD card and put the SD card in the device.

The elements:

- ESP32-S3 with at least 4MB of PSRAM. (with ESP32 Wrover module it works also).
- GPS module.
- Small display. IPS to get a good visibility outdoors.
- Micro-sd card with the maps data.
- Vectorial maps data from OpenStreetMap. 

First I built a prototype in the protoboard:

New version with ESP32-S3 devkit:
![GPS ESP32](/img/esp32-s3_gps.jpg)

I have created also a PCB prototype with battery, usb charger, programmer header, a joystick and two buttons for user interface and other two for reset and program mode:

The previous version. The Wrover module is under the display:
![GPS ESP32](/img/gps_pcb2.jpg)

The new version (still in design phase) with the ESP32-S3, battery charger, card slot and GPS all integrated in the board.
![GPS ESP32](/img/esp32-s3-pcb.jpg)

The schematic:
![GPS ESP32](/img/schematic_esp32-s3.jpg)

I will share the kicad project also.


The wiring for the basic prototype with the ESP32-S3 devkitc:
![GPS ESP32](/img/esp32-s3-wiring.jpg)



The MCU should be the ESP32-S3 with 4MB or 8MB of PSRAM (or Wrover) to have enough memory for the maps data to display.

I plan also a version with a small round display in a compact form. The system should work with any display size.

The prototype is already functional. It's shows your position in the map with the little triangle, it refresh as you move. With the joystick you can move around in the screen, zoom in and out. 

The map is created out of an Openstreetmap PBF extract and clipped to the desired area and put the generated map files in the SD card. Storage space is not a problem, so you can have a quite big area of hundreds of kms. The map file creation is done in the computer with some scripts and then copied to the SD card. See project: https://github.com/aresta/OSM_Extract

The map files creation, the rendering of the elements and the GPS position work fine but there is still work to do. The user interface is still very basic and the GPX track display and track recording is not yet implemented.

Any help is welcome!

Todo's:
- Optimize several aspects related to maps storage and memory.
- Test with IPS smaller displays and round (clock) displays.
- A proper plastic housing and better compact design, maybe with lateral buttons as the GPS patch can't be placed below the display to have a good reception and it takes quite some additional space.
- Show and record the GPX tracks.
- Rotate the elements based in movement direction.
- ...



---
The excellent map data is available thanks to the great OpenStreetMap project and contributors. The map data is available under the Open Database License.

© OpenStreetMap contributors


