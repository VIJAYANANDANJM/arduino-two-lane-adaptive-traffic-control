# Two-Lane Adaptive Traffic Control System

A reproducible setup guide for the Arduino Uno prototype using two HC-SR04 ultrasonic sensors and four LEDs.

## 1. Project overview

**Project title:** Design and Implementation of an Arduino-Based Two-Lane Adaptive Traffic Control System Using Ultrasonic Sensors

The prototype measures the distance to objects in two lanes and selects a lane for a green signal using simple distance-based rules.

- Lane 1 has one HC-SR04 sensor, one red LED, and one green LED.
- Lane 2 has one HC-SR04 sensor, one red LED, and one green LED.
- The Arduino Uno reads both sensors, chooses a lane, and controls the LEDs.
- The Serial Monitor displays distance readings and the selected lane.

> **Safety note:** This is an educational tabletop prototype, not a real-road traffic controller. It does not implement a complete road-safety sequence, pedestrian phases, validated queue detection, or guaranteed maximum waiting times.

## 2. Hardware required

| Component | Quantity | Purpose |
|---|---:|---|
| Arduino Uno | 1 | Main controller |
| HC-SR04 ultrasonic sensor | 2 | One distance sensor per lane |
| Red LEDs | 2 | Stop indicators |
| Green LEDs | 2 | Go indicators |
| 220–330 Ω resistors | 4 | One resistor in series with each LED |
| Solderless breadboard | 1 | Circuit assembly |
| Jumper wires | As needed | Connections |
| USB cable compatible with the Uno | 1 | Power, upload, and serial communication |
| Laptop with Arduino IDE | 1 | Write and upload the program |

## 3. Pin configuration

Keep these assignments consistent with the code.

### 3.1 Ultrasonic sensors

| Sensor connection | Lane 1 | Lane 2 |
|---|---|---|
| VCC | Arduino 5V | Arduino 5V |
| GND | Arduino GND | Arduino GND |
| TRIG | D2 | D4 |
| ECHO | D3 | D5 |

Both sensors use the same Arduino 5V and GND. The breadboard power rails may be used to distribute these connections. If using rails, connect Arduino 5V to the positive rail and Arduino GND to the ground rail. Check whether the rails are split partway along the breadboard.

### 3.2 Traffic LEDs

| Signal | Arduino digital pin |
|---|---:|
| Lane 1 red | D8 |
| Lane 1 green | D9 |
| Lane 2 red | D10 |
| Lane 2 green | D11 |

For each LED:
1. Connect its assigned Arduino digital pin through a **220–330 Ω resistor** to the LED's long leg (anode, +).
2. Connect the LED's short leg (cathode, −) to Arduino GND.
3. Use a separate resistor for every LED.

The resistor can be placed before or after the LED in the same series path; it limits current. Disconnect USB power before changing wiring. Never connect an Arduino output directly to GND or 5V.

## 4. Breadboard and wiring notes

- On a typical solderless breadboard, the five holes in a numbered row on one side of the centre gap are connected internally.
- The centre gap separates the two halves.
- Power rails run along the edges, but some breadboards split a rail into separate sections.
- The red/blue markings on a rail do not provide power by themselves; connect the rails to Arduino 5V/GND if you use them.
- Keep each LED's two legs in different connected rows, so they are not shorted together.
- Confirm the HC-SR04 pin labels on the actual module; connect VCC, TRIG, ECHO, and GND according to the labels.

## 5. Control logic and current parameters

The program uses the following settings:

| Setting | Value | Meaning |
|---|---:|---|
| Detection limit | 100 cm | An object is considered detected if the valid distance is at most this value |
| Tie margin | 5 cm | Readings within 5 cm are treated as similar |
| Green phase | 5,000 ms (5 s) | Duration of the selected lane's green signal |
| All-red interval | 1,000 ms (1 s) | Both red LEDs are on before the next selection |
| Serial baud rate | 9,600 | Must match the Serial Monitor setting |
| Sensor timeout | 25,000 µs | Maximum wait for an echo in the current code |

### Lane-selection rules

1. If only one lane has a valid reading within 100 cm, select that lane.
2. If both lanes are detected and their readings differ by more than 5 cm, select the lane with the smaller distance.
3. If both readings are within 5 cm of each other, alternate priority between lanes.
4. If neither lane is detected within 100 cm, alternate priority between lanes.
5. The selected lane's green LED turns on and its red LED turns off. The other lane stays red.
6. After 5 seconds, both lanes turn red for 1 second. The program then measures both sensors again.

**Interpretation limitation:** A single sensor measures distance to a reflecting object, not the number of vehicles or the length of the queue. The code uses distance as a simple proxy for traffic priority.

## 6. Full Arduino sketch

Copy this code into Arduino IDE and save it as `Two_Lane_Adaptive_Traffic_Control.ino`.

```cpp
const int trigPins[2] = {2, 4};
const int echoPins[2] = {3, 5};

const int redPins[2] = {8, 10};
const int greenPins[2] = {9, 11};

const float DETECTION_LIMIT = 100.0;
const float TIE_MARGIN = 5.0;

const unsigned long GREEN_TIME = 5000;
const unsigned long ALL_RED_TIME = 1000;

const float NO_OBJECT = 999.0;

int lastServedLane = 1;

float measureDistance(int lane) {
  digitalWrite(trigPins[lane], LOW);
  delayMicroseconds(2);

  digitalWrite(trigPins[lane], HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPins[lane], LOW);

  unsigned long duration =
      pulseIn(echoPins[lane], HIGH, 25000);

  if (duration == 0) {
    return NO_OBJECT;
  }

  float distance = duration * 0.0343 / 2.0;

  if (distance < 2 || distance > 400) {
    return NO_OBJECT;
  }

  return distance;
}

void allRed() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(greenPins[i], LOW);
    digitalWrite(redPins[i], HIGH);
  }
}

void setGreen(int lane) {
  allRed();
  digitalWrite(redPins[lane], LOW);
  digitalWrite(greenPins[lane], HIGH);
}

int selectLane(float distance[]) {
  bool detected[2];

  for (int i = 0; i < 2; i++) {
    detected[i] = distance[i] <= DETECTION_LIMIT;
  }

  // Neither lane has a nearby detected object
  if (!detected[0] && !detected[1]) {
    return 1 - lastServedLane;
  }

  // Only Lane 1 has a detected object
  if (detected[0] && !detected[1]) {
    return 0;
  }

  // Only Lane 2 has a detected object
  if (!detected[0] && detected[1]) {
    return 1;
  }

  // Both lanes have detected objects
  if (abs(distance[0] - distance[1]) <= TIE_MARGIN) {
    return 1 - lastServedLane;
  }

  return (distance[0] < distance[1]) ? 0 : 1;
}

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 2; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);

    pinMode(redPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);

    digitalWrite(trigPins[i], LOW);
  }

  allRed();
  Serial.println("Two-Lane Adaptive Traffic Control");
}

void loop() {
  float distance[2];

  // Read both sensors
  for (int i = 0; i < 2; i++) {
    distance[i] = measureDistance(i);
    delay(50);
  }

  // Print readings
  for (int i = 0; i < 2; i++) {
    Serial.print("Lane ");
    Serial.print(i + 1);
    Serial.print(": ");

    if (distance[i] == NO_OBJECT ||
        distance[i] > DETECTION_LIMIT) {
      Serial.println("No nearby object");
    } else {
      Serial.print(distance[i]);
      Serial.println(" cm");
    }
  }

  int selectedLane = selectLane(distance);

  Serial.print("Green signal: Lane ");
  Serial.println(selectedLane + 1);

  setGreen(selectedLane);
  lastServedLane = selectedLane;

  delay(GREEN_TIME);

  allRed();
  Serial.println("All RED: clearance interval");
  delay(ALL_RED_TIME);
}
```

## 7. Rebuild and run the project

1. **Assemble the circuit:** Wire the two HC-SR04 sensors and four LEDs using the pin tables above.
2. **Check power and polarity:** Verify sensor VCC/GND and LED orientation. Each LED must have its own resistor.
3. **Connect USB:** Connect the Arduino Uno to the laptop.
4. **Open Arduino IDE:** Create a new sketch and paste the code.
5. **Select the board:** Choose **Tools → Board → Arduino AVR Boards → Arduino Uno** (menu wording may differ by IDE version).
6. **Select the port:** Choose **Tools → Port** and select the port for the Uno.
7. **Upload:** Click Upload and wait for the IDE to report completion.
8. **Open Serial Monitor:** Choose **Tools → Serial Monitor** and set the baud rate to **9600**.
9. **Test the sensors and LEDs:** Use your hand or a cardboard object in front of each sensor, one at a time, and compare actual behaviour with the test cases below.

## 8. Test cases

The distances below are target values; actual readings may vary. Keep objects facing the sensor and within its useful range.

| Test ID | Lane 1 target | Lane 2 target | Expected selection |
|---|---|---|---|
| TC01 | 20 cm | 150 cm or farther | Lane 1 green |
| TC02 | 150 cm or farther | 20 cm | Lane 2 green |
| TC03 | 20 cm | 60 cm | Lane 1 green |
| TC04 | 60 cm | 20 cm | Lane 2 green |
| TC05 | 30 cm | 33 cm | Alternates between lanes over successive cycles |
| TC06 | Both over 100 cm / no echo | Both over 100 cm / no echo | Alternates between lanes |
| TC07 | 120 cm | 150 cm | Alternates between lanes |
| TC08 | 25 cm | 25 cm | Alternates between lanes |

For each test, record:
- Actual Lane 1 distance from the Serial Monitor
- Actual Lane 2 distance from the Serial Monitor
- Lane reported as green
- Whether the LEDs matched the expected selection

## 9. Example Serial Monitor output

Your values will differ based on object placement.

```text
Two-Lane Adaptive Traffic Control

Lane 1: 20.15 cm
Lane 2: 60.32 cm
Green signal: Lane 1
All RED: clearance interval
```

Set the Serial Monitor to **9600 baud**. If the output is blank or unreadable, verify that the correct port is selected and that the baud rate matches `Serial.begin(9600)`.

## 10. Troubleshooting

| Problem | Checks |
|---|---|
| No Serial Monitor output | Confirm the correct port, 9600 baud, and that the sketch uploaded successfully. |
| Sensor always reports no nearby object | Check VCC, GND, TRIG, and ECHO wiring; test the sensor separately. |
| One sensor works but the other does not | Recheck the second sensor's D4/D5 connections and its power. |
| Wrong LED lights up | Check the LED pin table, resistor placement, and code pin arrays. |
| Both green LEDs light up | Check wiring for shorts and confirm the correct sketch is running. |
| Distance readings fluctuate | Aim the sensors at flat objects, avoid angled surfaces, and test one sensor at a time. |
| LED does not light | Check polarity, the series resistor, breadboard row connections, and GND. |

## 11. Known limitations and possible improvements

- Distance is only a proxy for traffic demand; this prototype does not count vehicles or measure queue length.
- If one lane consistently has the closer reading, the current code can keep selecting it. It does **not** guarantee a maximum waiting time for the other lane.
- The project uses red and green LEDs and an all-red interval, but no yellow/amber signal or pedestrian phase.
- Sensor faults are treated like no nearby object; there is no dedicated fault indicator.
- The 5-second green and 1-second all-red timings are demonstration settings, not validated real-road timings.

Possible future improvements include a maximum green duration, a minimum service guarantee for each lane, better queue estimation, an amber LED per lane, a manual override, and fault detection.

## 12. Quick recovery checklist

If the project needs to be assembled again, use this order:

- [ ] Gather the Arduino Uno, two HC-SR04 sensors, four LEDs, four resistors, breadboard, jumper wires, and USB cable.
- [ ] Wire Lane 1 sensor: TRIG D2, ECHO D3, VCC 5V, GND GND.
- [ ] Wire Lane 2 sensor: TRIG D4, ECHO D5, VCC 5V, GND GND.
- [ ] Wire Lane 1 red/green LEDs to D8/D9.
- [ ] Wire Lane 2 red/green LEDs to D10/D11.
- [ ] Put a 220–330 Ω resistor in series with every LED.
- [ ] Upload the sketch from Section 6.
- [ ] Open Serial Monitor at 9600 baud.
- [ ] Run the test cases in Section 8.

---

**Project status:** Prototype tested by the user with the stated hardware configuration. Re-run the test cases after any rewiring or code changes.
