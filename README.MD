# **WebTurntable**
A webpage controllable turntable project that I made during a free weekend. This repository contains the project code. Visit the [MakerWorld](https://makerworld.com/en/models/572900) page to get the 3D files.

## **Necessary components**
### **Electronics**
- ULN2003 stepper driver
- 28byj-48 (5V) motor
- Wemos D1 mini (USB-C)
- 3mm LED
- LED current limiting resistor (220 ohm)

### **Fasteners**
- 3x M3 threaded inserts
- 5x M3 bolts
- 2x M3 nuts
- 4x M2 bolts
- 4x M2 nuts

### **3D-printed parts**
- Disk
- Body
- Bottom plate

## **Compile and upload the code**
The code is based on the arduino software platform. In order to compile and upload this code, it is recommended to install the VSCode plugin for arduino. Also make sure the ESP8266 board package is installed.

### **Basic example**
Simple code (example) that just rotates the turntable.

### **WebTurntable**
The full version of the code. The turntable acts as a Wi-Fi access point, allowing access to its control webpage.

Libraries you'll need are:
- Arduino_JSON
- ESPAsyncTCP
- ESP AsyncWebServer

You can upload the code directly from VSCode, although the webpage data has to be written to flash. I did this using the old arduino IDE with the SPIFFS tool.

**Make sure the flash size is set to: "4MB (FS:3MB OTA: ~512KB)"**