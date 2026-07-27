#include <sensor.h>

const int trigPin = 5;
const int echoPin = 18;

long duration;
float distanceCm;

void initSensor() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

short readSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 30ms timeout so a disconnected sensor can't block this task forever
  duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) {
    return OBSTACLE_SENSOR_ERROR;
  }

  distanceCm = duration * SOUND_SPEED / 2;

  if (distanceCm < 30 && distanceCm > 10) {
    return OBSTACLE_NEAR;
  } else if (distanceCm <= 10) {
    return OBSTACLE_DANGER;
  }
  return OBSTACLE_NONE;
}
