
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