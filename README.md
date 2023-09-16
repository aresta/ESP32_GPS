# ESP32_GPS

The goal is to build a portable GPS device that displays a map with your position in a small display.

You just need to download the map files of your country/area (see: https://github.com/aresta/OSM_Extract), copy it into the SD card, put the SD card in the device and power on.

The elements:

- ESP32 Wrover micro controller.
- GPS module.
- Small display. IPS to get a good visibility outdoors.
- Micro-sd card with the maps data.
- Vectorial maps data from OpenStreetMap. Basically polylines and polygons to display the roads, paths, rivers, buildings, forest, etc.

First I built a prototype in the protoboard:

![GPS ESP32](/img/esp32_gps_d.jpg)

And then I created also a PCB prototype with battery, usb charger, programmer header, a joystick and two buttons for user interface and other two for reset and program mode:

![GPS ESP32](/img/gps_pcb3.jpg)

![GPS ESP32](/img/gps_pcb2.jpg)

The usb charger and the micro SD-card reader are soldered below. In a final version I could integrate them also in the PCB.

The schematic:
![GPS ESP32](/img/schematic.jpg)

I will share the kicad project also.


The wiring for the basic prototype:
![GPS ESP32](/img/esp32_gps_wiring.jpg)



The MCU should be the ESP32 Wrover to have enough memory, but I haven't found the Fritzing image.  And the pins could be in different positions, just pay attention to the GPIO numbers.

I plan also a version with a small round display in a compact form. The system works with any display size.

The prototype is already functional. It's shows your position in the map with the little triangle, it refresh as you move and with the joystick you can move around in the screen, zoom in and out. 

The map is created out of an Openstreetmap PBF extract and clipped to the desired area and put the generated map files in the SD card. Storage space is not a problem, so you can have a quite big area of hundreds of kms. The map file creation is done in the computer with some scripts and then copied to the SD card. See project: https://github.com/aresta/OSM_Extract

The map files creation and displaying the elements is already working but there is still work to do. Also in the user interface and the rest of the software.

Any help is welcome!

Todo's:
- Optimize several aspects related to maps storage and memory.
- Test with IPS smaller displays and round (clock) displays.
- A proper plastic housing and better connections design.
- Show and record the GPX tracks.
- ...



---
The excellent map data is available thanks to the great OpenStreetMap project and it is available under the Open Database License.

© OpenStreetMap contributors


