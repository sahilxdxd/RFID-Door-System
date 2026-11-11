
# RFID-Based Automatic Door System

An IoT-based automatic door prototype developed using ESP32, RC522 RFID reader, SG90 servo motor, and an LED indicator.

The system provides RFID-based access authentication, automatic door operation, and a Wi-Fi dashboard for monitoring and adjusting door settings.

## Project Overview

The RFID-Based Automatic Door System is designed to demonstrate an automated access-control mechanism.

When an RFID card is scanned, the RC522 reader reads its unique identification number (UID). The ESP32 checks the UID against the authorized card stored in the program.

- If the card is authorized, access is granted, the LED turns ON, and the servo opens the door.
- If the card is unauthorized, access is denied and the door remains closed.
- After the configured time, the door closes automatically.

An ESP32-hosted web dashboard allows users to monitor the system and adjust selected operating settings.

## Features

- RFID-based authentication using RC522.
- Single authorized RFID card.
- Automatic door opening and closing.
- Adjustable door-open duration.
- Adjustable servo movement speed.
- LED indication for authorized access.
- ESP32 Wi-Fi access point.
- Web-based monitoring dashboard.
- Live door and LED status.
- Authorized and denied scan counters.
- Recent RFID scan history.
- Repeated RFID card scanning.

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32 DevKit | Main microcontroller |
| RC522 RFID Reader | Reads RFID card UID |
| RFID Card | Provides card identification |
| SG90 Servo Motor | Controls door movement |
| LED | Indicates authorized access |
| 220Ω Resistor | Limits LED current |
| Jumper Wires | Electrical connections |
| Miniature Door Structure | Physical demonstration |

## Software and Libraries

- Arduino IDE
- ESP32 Arduino Core
- MFRC522 Library
- ESP32Servo Library
- HTML
- CSS
- JavaScript
- ESP32 WebServer Library
- ESP32 Wi-Fi Library

## Pin Configuration

### RC522 RFID Reader

| RC522 Pin | ESP32 Pin |
|---|---|
| SDA / SS | GPIO 21 |
| SCK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| RST | GPIO 22 |
| VCC | 3.3V |
| GND | GND |
| IRQ | Not Connected |

### Servo Motor

| Servo Wire | ESP32 / Supply |
|---|---|
| Orange / Yellow | GPIO 13 |
| Red | Regulated 5V |
| Brown / Black | GND |

### LED

| LED Connection | ESP32 |
|---|---|
| Positive leg | GPIO 4 through 220Ω resistor |
| Negative leg | GND |

**Important:** All grounds must be electrically common. The RC522 must be powered from 3.3V, never 5V. Use a suitable regulated 5V supply for the servo and connect its ground to ESP32 GND.

## System Architecture

```text
        RFID CARD
            |
            v
      RC522 READER
            |
       SPI COMMUNICATION
            |
            v
          ESP32
            |
     UID AUTHENTICATION
            |
       +----+----+
       |         |
   AUTHORIZED  UNAUTHORIZED
       |         |
       v         v
   SERVO OPEN  ACCESS DENIED
       |
       v
    LED ON
       |
       v
 AUTOMATIC TIMER
       |
       v
   SERVO CLOSE
       |
       v
     LED OFF


          ESP32
            |
        Wi-Fi AP
            |
            v
      WEB DASHBOARD
            |
     +------+------+
     |             |
  MONITORING    SETTINGS
                  |
          +-------+-------+
          |               |
      OPEN TIME       SERVO SPEED
```

## Working Principle

### 1. RFID Detection

The RC522 reader detects a compatible RFID card and reads its UID.

### 2. Authentication

The ESP32 compares the scanned UID with the authorized UID stored in the firmware.

### 3. Access Granted

If the UID matches:

- Access is granted.
- LED turns ON.
- Servo receives an opening command.
- Door opens.

### 4. Access Denied

If the UID does not match:

- Access is denied.
- Denied scan counter increases.
- Door does not receive an opening command.

### 5. Automatic Closing

After the configured open duration, the ESP32 sends a closing command to the servo.

### 6. Wi-Fi Dashboard

The ESP32 hosts a local website that displays system status and allows users to change door-open duration and servo movement settings.

## Wi-Fi Dashboard

The ESP32 creates its own Wi-Fi access point.

Example configuration:

| Setting | Value |
|---|---|
| Wi-Fi Name | RFID-Door-System |
| Password | ChangeThisPassword |
| Dashboard Address | http://192.168.4.1 |

Connect a phone or laptop to the ESP32 Wi-Fi network and open the dashboard address in a browser.

The dashboard includes:

- Door status
- LED status
- Authorized scan count
- Denied scan count
- Latest scanned UID
- Latest access result
- Recent RFID activity
- Door-open duration setting
- Servo movement speed setting

**Note:** This is a local Wi-Fi dashboard. It is not automatically accessible over the internet. Remote access requires an internet connection at the ESP32 and a cloud service or secured backend.

## Servo Speed Control

The dashboard provides a speed setting from 1% to 100%.

| Setting | Movement |
|---|---|
| 1% | Slowest gradual movement |
| 25% | Slow |
| 50% | Medium |
| 75% | Fast |
| 99% | Fastest gradual movement |
| 100% | Direct servo target command |

At 100%, the ESP32 sends the target angle directly. The actual motor speed is limited by the servo's physical specifications, power supply, mechanical load, and friction.

## Door Timing

The door-open duration can be adjusted through the dashboard from 1 to 20 seconds.

The default duration is 5 seconds.

## Installation

1. Install Arduino IDE.
2. Install ESP32 board support.
3. Install the MFRC522 library.
4. Install the ESP32Servo library.
5. Open the firmware `.ino` file.
6. Select ESP32 Dev Module.
7. Select the correct COM port.
8. Check the wiring.
9. Upload the program.
10. Open Serial Monitor at 115200 baud.

## Repository Structure

```text
RFID-Based-Automatic-Door-System/
│
├── README.md
│
├── firmware/
│   └── RFID_Door_System.ino
│
├── docs/
│   ├── COMPONENTS.md
│   ├── WIRING.md
│   ├── INSTALLATION.md
│   ├── WORKING.md
│   └── TROUBLESHOOTING.md
│
├── images/
│   ├── circuit-diagram.png
│   ├── hardware-setup.jpg
│   └── dashboard.png
│
└── LICENSE
```

## Limitations

- The current implementation uses a single authorized RFID UID.
- RFID UID comparison alone is not suitable for high-security access control.
- Door position is estimated from software timing; no position sensor is installed.
- Wi-Fi dashboard settings are stored in RAM and reset after restarting the ESP32.
- Local Wi-Fi operation does not provide remote internet access automatically.
- Servo speed cannot exceed the physical motor's rated speed.

## Future Improvements

- Cloud-based remote monitoring.
- Mobile application integration.
- Secure RFID authentication.
- Persistent configuration storage.
- Physical door-position sensor.
- Access event logging in a cloud database.
- Multiple authorized users, if required in a future version.

## Applications

- Smart home demonstrations.
- Miniature access-control systems.
- Educational IoT projects.
- RFID-based entry prototypes.
- Embedded systems learning.

## License

This project is distributed under the MIT License. See the LICENSE file.

## Author

Developed as an educational IoT project.

**Project Title:** RFID-Based Automatic Door System
