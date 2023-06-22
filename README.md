# ESP32_GPS

The goal is to build a portable GPS device that displays a map with your position in a small display.

You just need to download the map file of your country/area, copy it into the SD card, put the SD card in the device and power on.

The elements:

- ESP32 Wrover micro controller.
- GPS module.
- Small display. IPS to get a good visibility outdoors.
- Micro-sd card with the maps data.
- Vectorial maps data from OpenStreetMap. Basically polylines and polygons to display the roads, paths, rivers, buildings, forest, etc.

I have already built a functional prototype that works.

Here in the protoboard:

![GPS ESP32](/img/esp32_gps_d.jpg)

The wiring:

![GPS ESP32](/img/esp32_gps_wiring.jpg)

The MCU should be the ESP32 Wrover to have enough memory, but I haven't found the Fritzing image.  And the pins could be in different positions, just pay attention to the GPIO numbers.

In the final version the plan is to put just the ESP32 without the board, and the rest in a custom PCB behind the display.
I will create the PCB design in KiCad.

I plan also a version with a small round display in a compact form. The system works with any display size.

The prototype is already functional. It's shows your position in the map with the little triangle, and it refresh as you move. 

The map is created out of an Openstreetmap PBF extract and clipped to a few square kms for testing. The map file creation is done in the computer with some scripts and then copied to the SD card.

There is still work to do in several different areas.  Any help is welcome!

- Optimize several aspects related to maps storage and memory.
- A PCB, once the design is completed.
- Some buttons and user interface.
- Test with IPS smaller displays and round (clock) displays.
- A proper plastic housing.
- Battery power. AAA or small Li-XX batteries.
- Show and record the GPX tracks.
- ...



---
The excellent map data is available thanks to the great OpenStreetMap project and it is available under the Open Database License.

© OpenStreetMap contributors


