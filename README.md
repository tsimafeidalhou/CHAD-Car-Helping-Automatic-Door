# CHAD (Car Helping Automatic Door)

An IoT garage automation system that uses an ultrasonic distance sensor to track a vehicle's position and automatically close the garage door when the car finishes parking or leaves. A color-coded LED and web dashboard provide real-time parking guidance, and a servo motor physically presses the garage door opener button.

---

## Files

| File | Description |
|------|-------------|
| `Garage_Opener.cpp` | Particle Photon 2 firmware |
| `CHAD_website.html` | Web dashboard for remote monitoring and manual override |
| `JustBox.stl` | 3D printable housing body |
| `Lid.stl` | 3D printable housing lid |

---

## Hardware Required

- Particle Photon 2
- HC-SR04 Ultrasonic Distance Sensor
- SG90 Servo Motor
- WS2812 NeoPixel RGB LED
- Garage door opener PCB (salvaged from a wireless opener)
- 6V battery pack (4 D-cell batteries) for the servo
- 3x 1kΩ resistors (voltage divider for echo pin)
- Breadboard and jumper wires

---

## Wiring

| Component | Photon 2 Pin |
|-----------|-------------|
| HC-SR04 TRIG | D2 |
| HC-SR04 ECHO | D3 (through voltage divider: 5V → 3.3V using three 1kΩ resistors) |
| SG90 Servo | D1 (powered externally from 6V battery pack) |
| NeoPixel LED | SPI |

See the schematic in the report for the full wiring diagram.

---

## Setup

### Firmware

1. Open `Garage_Opener.cpp` in the [Particle Web IDE](https://build.particle.io) or Particle Workbench.
2. Flash to your Particle Photon 2.
3. The device will expose three cloud variables (`distance`, `ledColor`, `carPresent`) and one cloud function (`closeDoor`).

### Web Dashboard

1. Open `CHAD_website.html` in a text editor.
2. Replace the `deviceID` and `accessToken` values at the top of the script with your own Particle credentials:
```javascript
var deviceID = "YOUR_DEVICE_ID";
var accessToken = "YOUR_ACCESS_TOKEN";
```
3. Open the file in any web browser. It will poll your device every 1.5 seconds.

### 3D Printed Housing

1. Print `JustBox.stl` and `Lid.stl` with any standard FDM printer.
2. Mount the breadboard, Photon, and battery pack inside the box.
3. Mount the servo on the lid so it aligns with the button on your garage door opener PCB.
4. Mount the ultrasonic sensor on the outside, aimed toward where the car parks.

---

## How It Works

The system divides the garage into three distance zones:

| Color | Distance | Meaning |
|-------|----------|---------|
| Green | > 100cm | Safe, keep moving |
| Yellow | 20-100cm | Slow down |
| Red | < 20cm | Stop |

When the LED transitions from green to yellow to red (approaching) or red to yellow to green (departing), a 30-second timer starts. If the zone doesn't change during that time, the servo presses the garage door button.
