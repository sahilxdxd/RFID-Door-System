
# Hardware Components

## Components Used

| No. | Component | Quantity | Purpose |
|---|---|---:|---|
| 1 | ESP32 DevKit | 1 | Main microcontroller |
| 2 | RC522 RFID Reader | 1 | Reads RFID card UID |
| 3 | RFID Card | 1 | Provides access identification |
| 4 | SG90 Servo Motor | 1 | Controls door movement |
| 5 | LED | 1 | Indicates access status |
| 6 | 220 Ohm Resistor | 1 | Limits LED current |
| 7 | Jumper Wires | As required | Electrical connections |
| 8 | USB Cable | 1 | ESP32 programming and power |
| 9 | External 5V Supply | As required | Servo power, if needed |

## Component Description

### ESP32
The ESP32 acts as the central controller. It processes RFID data, controls the servo and LED, and hosts the local Wi-Fi dashboard.

### RC522 RFID Reader
The RC522 reads the unique identifier (UID) of an RFID card and sends it to the ESP32 through SPI communication.

### SG90 Servo Motor
The servo motor controls the opening and closing movement of the miniature door.

### LED
The LED provides a visual indication when access is granted.

### RFID Card
The RFID card is scanned by the RC522 reader to request access.

## Power Considerations

- Connect the RC522 to 3.3V, not 5V.
- Power the servo using a suitable regulated supply.
- Connect the external supply ground to ESP32 GND.
- Ensure all components share a common ground.
