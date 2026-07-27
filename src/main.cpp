#include <Arduino.h>
#include "blynk_config.h"
#include "config.h"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "tasks.h"
#include "motor.h"

// BLYNK_WRITE handlers run inside networkTask's call to Blynk.run(), so
// they must never block. They just read/write the commanded speed and
// push a MotorCommand onto the queue for motorTask to execute.

BLYNK_WRITE(V0) {
  int speed2 = param.asInt();
  // Left motor (128) runs faster than right (89) at the same PWM value;
  // this ratio keeps them rotating at matching speeds.
  int speed1 = (int) constrain(speed2 * 128 / 89, 0, MAX_SPEED);
  setCommandedSpeed(speed1, speed2);
}

BLYNK_WRITE(V1) { // Forward
  int sL, sR;
  getCommandedSpeed(sL, sR);
  requestMotorAction(param.asInt() ? MOTOR_FORWARD : MOTOR_STOP, sL, sR);
}

BLYNK_WRITE(V2) { // Backward
  int sL, sR;
  getCommandedSpeed(sL, sR);
  requestMotorAction(param.asInt() ? MOTOR_BACKWARD : MOTOR_STOP, sL, sR);
}

BLYNK_WRITE(V3) { // Left
  int sL, sR;
  getCommandedSpeed(sL, sR);
  requestMotorAction(param.asInt() ? MOTOR_LEFT : MOTOR_STOP, sL, sR);
}

BLYNK_WRITE(V4) { // Right
  int sL, sR;
  getCommandedSpeed(sL, sR);
  requestMotorAction(param.asInt() ? MOTOR_RIGHT : MOTOR_STOP, sL, sR);
}

void setup() {
  Serial.begin(115200);
  startTasks(); // spins up sensor/motor/display/network tasks (tasks.cpp)
}

void loop() {
  // All real work happens in the FreeRTOS tasks created by startTasks().
  // Arduino's own loopTask still exists on core 1 at low priority, so we
  // just let it idle rather than deleting it.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
