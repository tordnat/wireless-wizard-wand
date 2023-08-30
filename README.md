# Wireless Wizard Wand 🪄✨
![workflow](https://github.com/tordnat/wireless-wizard-wand/actions/workflows/build.yml/badge.svg)

Bring magic to your home with Nordic Semiconductor Wireless Wizard Wand (www). The wand utilizes a powerful nRF52840 chip with [Edge Impulse ML motion recognition](https://studio.edgeimpulse.com/public/181284/latest) to communicate with all your smart home appliances over Zigbee. 

![demo-gif](documentation/www-loop.gif)

## Build & deployment

There are currently two options to deploy the project:
1. Use the prebuilt release
2. Build the project using west and nRF Connect SDK v2.3

To flash the hex-file to the Adafruit Feather nRF52840 you can use an external debugger (Segger J-Link or nRF52dk).
The blue indication LED (which indicates Zigbee stack initialization) on the wand should give you a sign whether you have flashed the MCU successfully.
Once the program is running, you can connect to the wand with a micro-USB cable. If the wand detects a serial connection over USB on boot, logging should automatically start. 
The sample application is a simple program that looks for a Zigbee light bulb and sends commands over Zigbee based on ML motion recognition. Once a light bulb is detected, the
red indication LED should light up.

***Conditions for a working sample***:

✅ Correctly flashed MCU

✅ MPU6050 IMU detected over I2C (see Hardware)

✅ Joined Zigbee network (see [Zigbee Light Bulb Sample](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/samples/zigbee/network_coordinator/README.html))

✅ Light bulb detected

## About

During my internship at Nordic Semiconductor ASA I was tasked with designing and building a demo that demonstrates the nRF52840's AI capabilities, and thus the Wireless Wizard Wand was born.
The wand was designed and built during the course of my summer internship, and tested during Trondheim Skaperfest (≈maker faire). The test included controlling a light bulb ([IKEA Trådfri](https://www.ikea.com/no/no/p/tradfri-led-paere-e27-470-lumen-smart-kan-dimmes-tradlost-varmhvit-til-kaldhvit-tubeformet-90461916/)) and unlocking a treasure chest
with magic spells. Although not all features were implemented, the test was a success🥳. 


<img src="https://github.com/tordnat/wireless-wizard-wand/blob/documentation/documentation/www-closeup.png" width="300" /> 

## Hardware & Features



<img src="https://github.com/tordnat/wireless-wizard-wand/blob/documentation/documentation/www-3d-pcb.png" width="300" /> 

***Main Components***
- [Adafruit Feather nRF52840 Express](https://www.adafruit.com/product/4062#description)
- [Generic MPU6050 Module](https://components101.com/sensors/mpu6050-module)

***PCB***
- Haptic Motor Driver DA7280-00FVC (QFN Package)
- Vybronics ERM Vibration Motor VC1030B028F
- Panasonic SMD Button EVQ-P7J01P
- 0805 Packages for passive components

***Casing***
- ICR18650-22F Battery
- Connection Pins from MPD BH-18650-W
- Smartfil Wood Filament

## How I made it

***Programming***

***PCB Design***

***Hardware Design***

## Further Development

- Implement Haptics
- LED indication
- Zigbee pairing logic
- Power optimization
- Separate charging circuit to improve charge time

