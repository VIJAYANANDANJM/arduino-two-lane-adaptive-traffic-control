# Arduino-Based Two-Lane Adaptive Traffic Control System Using Ultrasonic Sensors

An intelligent, responsive two-lane traffic light management system built on Arduino Uno using dual HC-SR04 ultrasonic distance sensors. The system dynamically prioritizes lanes based on detected vehicle proximity, reducing idle waiting time compared to conventional fixed-timer traffic lights.

---

## 🚦 System Architecture & Concept

Traditional fixed-time traffic signals cycle at pre-programmed intervals regardless of actual traffic density. This prototype implements an **adaptive control algorithm**:
- **Proximity-Based Detection:** Continuous distance measurement across both lanes via HC-SR04 ultrasonic sensors.
- **Dynamic Green Assignment:** Grants the green phase (5s) to the lane with oncoming vehicles or closer vehicle queues.
- **Fairness & Anti-Starvation:** Alternates priority when vehicle distances are tied within a configurable threshold ($\le 5\text{ cm}$) or when both lanes are empty.
- **Clearance Interval:** Enforces a safe 1-second all-red clearance phase between signal switches to prevent junction collisions.

```
       +---------------------------------------------+
       |                 Arduino Uno                 |
       +---------------------------------------------+
         |        |         |        |       |     |
      Trig/Echo Trig/Echo  Red/Grn  Red/Grn  5V   GND
         |        |         |        |       |     |
      [HC-SR04] [HC-SR04] [Lane 1] [Lane 2]  Power Rail
       Lane 1    Lane 2     LEDs     LEDs
```

---

## 🔌 Hardware Pin Mapping

### 1. Ultrasonic Distance Sensors (HC-SR04)
| Sensor | VCC | GND | TRIG Pin | ECHO Pin |
|---|---|---|:---:|:---:|
| **Lane 1** | 5V | GND | **D2** | **D3** |
| **Lane 2** | 5V | GND | **D4** | **D5** |

### 2. Traffic Light Signals (LEDs with 220–330 Ω Resistors)
| Signal Indicator | Arduino Digital Pin | Series Resistor |
|---|:---:|:---:|
| **Lane 1 Red (Stop)** | **D8** | 220–330 Ω |
| **Lane 1 Green (Go)** | **D9** | 220–330 Ω |
| **Lane 2 Red (Stop)** | **D10** | 220–330 Ω |
| **Lane 2 Green (Go)** | **D11** | 220–330 Ω |

---

## ⚙️ Control Parameters & Algorithm Logic

| Parameter | Configured Value | Description |
|---|:---:|---|
| `DETECTION_LIMIT` | `100.0 cm` | Maximum range to consider an approaching vehicle |
| `TIE_MARGIN` | `5.0 cm` | Distance variance treated as equal traffic load |
| `GREEN_TIME` | `5000 ms` | Active green light duration for the served lane |
| `ALL_RED_TIME` | `1000 ms` | Inter-phase safety clearance duration |
| `BAUD_RATE` | `9600` | Serial diagnostic monitoring output rate |

### Decision Rules
1. **Single Lane Active:** If only one lane has an obstacle within $\le 100\text{ cm}$, that lane gets immediate green.
2. **Both Lanes Active (Unequal):** If both lanes have vehicles and $|D_1 - D_2| > 5\text{ cm}$, the closer lane receives green priority.
3. **Tie / Both Equal:** If $|D_1 - D_2| \le 5\text{ cm}$, the system alternates to the lane that was not served last (`1 - lastServedLane`).
4. **No Traffic:** If neither lane detects an obstacle within $100\text{ cm}$, the system round-robins between lanes.

---

## 📁 Repository Structure

```
├── Two_Lane_code/
│   └── Two_Lane_code.ino               # Main Arduino firmware source code
├── Two_Lane_Adaptive_Traffic_Control_Setup.md # Detailed hardware wiring, breadboard & test suite guide
├── .gitignore                          # Build artifacts & editor ignores
└── README.md                           # Project documentation
```

---

## 🚀 Getting Started

### Prerequisites
- [Arduino IDE](https://www.arduino.cc/en/software) (version 1.8.x or 2.x)
- Arduino Uno R3 (or compatible board) with USB cable

### Setup Instructions
1. Clone or download this repository.
2. Open [`Two_Lane_code/Two_Lane_code.ino`](Two_Lane_code/Two_Lane_code.ino) in the Arduino IDE.
3. Wire the breadboard following the [Hardware Pin Mapping](#-hardware-pin-mapping) table and the comprehensive [Setup Guide](Two_Lane_Adaptive_Traffic_Control_Setup.md).
4. In Arduino IDE:
   - Select Board: **Tools → Board → Arduino AVR Boards → Arduino Uno**
   - Select Port: **Tools → Port → [Your Arduino COM Port]**
5. Click **Upload** (Ctrl + U).
6. Open the **Serial Monitor** at **9600 baud** to view real-time distance telemetry and lane transitions.

---

## 🧪 Verification & Test Scenarios

| Test Case | Lane 1 Distance | Lane 2 Distance | Expected Controller Action |
|:---:|:---:|:---:|---|
| **TC01** | 20 cm | > 100 cm | Lane 1 Green (5s) → All Red (1s) |
| **TC02** | > 100 cm | 20 cm | Lane 2 Green (5s) → All Red (1s) |
| **TC03** | 20 cm | 60 cm | Lane 1 Green (Closer traffic priority) |
| **TC04** | 60 cm | 20 cm | Lane 2 Green (Closer traffic priority) |
| **TC05** | 30 cm | 33 cm | Alternates between Lane 1 & Lane 2 |
| **TC06** | No vehicle | No vehicle | Round-robin alternation |

---

## 🔮 Future Enhancements
- Integration of yellow/amber transition warning lights.
- Dedicated pedestrian push-button crossing interrupt.
- Vehicle count queue estimation using multi-sensor arrays or camera-based computer vision.
- IoT cloud dashboard reporting traffic volume and congestion logs (e.g., via ESP8266/ESP32 or MQTT).
