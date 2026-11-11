
# Installation and Setup

## Requirements

- Arduino IDE 2.x
- ESP32 board package
- MFRC522 library
- ESP32Servo library
- ESP32 DevKit
- RC522 RFID reader
- SG90 servo motor

## Step 1: Install Arduino IDE

Download Arduino IDE from:

https://www.arduino.cc/en/software

## Step 2: Install ESP32 Board Support

1. Open Arduino IDE.
2. Open Boards Manager.
3. Search for ESP32.
4. Install the Espressif ESP32 package.
5. Select ESP32 Dev Module.

## Step 3: Install Required Libraries

Open Library Manager and install:

- MFRC522
- ESP32Servo

WiFi, WebServer, and SPI are included with the ESP32 board package.

## Step 4: Configure the Firmware

1. Open firmware/RFID_Door_System.ino.
2. Verify the pin definitions.
3. Set your RFID card UID.
4. Change the Wi-Fi access point password.
5. Check the servo power connections.

## Step 5: Upload the Code

1. Connect ESP32 through USB.
2. Select the correct COM port.
3. Select ESP32 Dev Module.
4. Upload the firmware.
5. Open Serial Monitor at 115200 baud.

## Step 6: Open the Dashboard

1. Connect your phone or laptop to the ESP32 Wi-Fi network.
2. Enter the configured Wi-Fi password.
3. Open the browser.
4. Visit http://192.168.4.1.

The dashboard should display the door status and system information.
