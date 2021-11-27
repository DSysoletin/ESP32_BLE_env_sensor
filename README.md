# ESP32_BLE_env_sensor
Simple ESP32 firmware for reading DHT11 values and sending them as a Eddystone BLE beacon. Includes long deep sleep between scans, can work with battery power.

Reference schematics for the hardware can be found in BLE_sensor.png file.

To use this firmware with USB power of ESP32, just connect the DHT11 sensor and upload the firmware to the ESP32. 

To use this firmware with the battery power, please take a look to the schematics provided. Because ESP32 draws a really high current when active, please make sure that you have low-ESR capacitor with adequate capacity placed on ESP32's module power input pins.

You can use nRF Connect software for Android mobile phone to watch the advetisements.