#ifndef SENSOR_H
#define SENSOR_H
#include <Arduino.h>

extern const int trigPin;
extern const int echoPin;

#define SOUND_SPEED 0.034

extern long duration;
extern float distanceCm;

// Obstacle levels, shared with the display/motor tasks.
enum ObstacleLevel {
  OBSTACLE_NONE = 0,
  OBSTACLE_NEAR = 1,   // 10cm < distance < 30cm
  OBSTACLE_DANGER = 2, // distance <= 10cm
  OBSTACLE_SENSOR_ERROR = -1
};

void initSensor();
short readSensor();

#endif // SENSOR_H
