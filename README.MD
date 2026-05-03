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
The code is based on the arduino software platform. In order to compile and upload the code, it is recommended to install the [ArduinoIDE](https://www.arduino.cc/en/software/) and the [ESP8266 board package](https://arduino-esp8266.readthedocs.io/en/latest/installing.html).

### **Basic example**
Simple code example that just rotates the turntable.

### **WebTurntable**
The full version of the code. The turntable acts as a Wi-Fi access point, allowing you to contoll the turntable through a webinterface.

The last direction, speed, and stepping mode are stored in [LittleFS](https://github.com/earlephilhower/arduino-littlefs-upload), and loaded from flash after reboot/power cycle. To prevent excessive wear on the flash memory, settings are written after a short idle delay (750 ms). After changing controls, wait about 1 second before unplugging power so the latest state is saved.

The following additional libraries are used:
- [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer)
- [ESPAsyncTCP](https://github.com/ESP32Async/ESPAsyncTCP)
- [Arduino_JSON](https://github.com/arduino-libraries/Arduino_JSON)


You can upload the code directly from the ArduinoIDE. To upload the webpage data you will need to install the [LittleFS tool](https://github.com/earlephilhower/arduino-littlefs-upload).

You can open the web interface to control the turntable as follows:
1. Connect to the Wi-Fi hotspot with name `TURNTABLE`
2. Open your browser and enter the following address `http://192.168.4.1`
3. You should now be in the webinterface to control the turntable

**Make sure the flash size is set to: "4MB (FS:3MB OTA: ~512KB)"**