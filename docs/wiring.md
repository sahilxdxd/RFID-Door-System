
# Circuit Connections

## ESP32 to RC522 RFID Reader

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

## ESP32 to Servo Motor

| Servo Wire | Connection |
|---|---|
| Orange / Yellow | GPIO 13 |
| Red | Regulated 5V Supply |
| Brown / Black | GND |

## ESP32 to LED

| LED Connection | ESP32 |
|---|---|
| Positive leg (Anode) | GPIO 4 through 220Ω resistor |
| Negative leg (Cathode) | GND |

## Important Notes

1. The RC522 must use 3.3V power.
2. Do not connect the servo's 5V wire to the ESP32 3.3V pin.
3. Use a suitable regulated power supply for the servo.
4. Connect the servo supply ground and ESP32 ground together.
5. Verify all connections before powering the system.
