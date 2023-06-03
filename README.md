# ESP32_GPS

The goal is to build a portable GPS device that displays a map with your position in a small display.

The device elements:

- ESP32 micro controller.
- GPS module.
- Small display.
- Micro-sd card with the maps data.
- Vectorial maps data from OpenStreetMap. Basically polylines and polygons to display the roads, paths, rivers, buildings, forest, etc.

I have already built a functional prototype that works.

Here in the protoboard:

![GPS ESP32](/img/esp32_gps_a.jpg)

And a more compact and robust prototype to test outside:

![GPS ESP32](/img/esp32_gps_b.jpg)
![GPS ESP32](/img/esp32_gps_c.jpg)

I'll post the wiring.

The prototype is already functional. It's shows your position in the map with the little triangle, and it refresh as you move. 

It loads a map in a custom file stored in the SD card. The map is created out of an Openstreetmap PBF extract and clipped to a few square kms for testing. The map file creation is done in the computer with some scripts and then copied to the SD card.

There is still work to do in several different areas.  Any help is welcome!

- Optimize several aspects related to maps storage and memory.
- A PCB, once the design is completed.
- Some buttons and user interface.
- Test with IPS smaller displays and round (clock) displays.
- A proper plastic housing.
- Battery power. AAA or small Li-XX batteries.
- Replace the ESP32 devkit by an ESP32 wroom or wrover.
- ...



---
The excellent map data is available thanks to the great OpenStreetMap project and it is available under the Open Database License.

© OpenStreetMap contributors


