#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// ---------------------------------------------------------------------
// Shared state
// ---------------------------------------------------------------------
// Anything read by more than one task lives here, guarded by stateMutex.
// Rule: never touch these fields directly — always go through
// getSystemState() / the setters below, which take the mutex for you.
struct SystemState {
  short obstacleLevel;   // ObstacleLevel enum from sensor.h
  bool  wifiConnected;
  bool  blynkConnected;
};

extern SemaphoreHandle_t stateMutex;
extern SystemState systemState; // never touch directly outside tasks.cpp

SystemState getSystemState();
void setObstacleLevel(short level);
void setWifiConnected(bool connected);
void setBlynkConnected(bool connected);

// ---------------------------------------------------------------------
// Motor command queue
// ---------------------------------------------------------------------
enum MotorAction { MOTOR_STOP, MOTOR_FORWARD, MOTOR_BACKWARD, MOTOR_LEFT, MOTOR_RIGHT };

struct MotorCommand {
  MotorAction action;
  int speedLeft;
  int speedRight;
};

extern QueueHandle_t motorCmdQueue;

// Called from Blynk callbacks (which run inside networkTask's context via
// Blynk.run()) to request a motor action. Non-blocking: if the queue is
// briefly full the command is dropped rather than stalling Blynk.
void requestMotorAction(MotorAction action, int speedLeft, int speedRight);

// ---------------------------------------------------------------------
// Commanded speed (set by the V0 slider, read by direction buttons)
// ---------------------------------------------------------------------
void setCommandedSpeed(int speedLeft, int speedRight);
void getCommandedSpeed(int &speedLeft, int &speedRight);

// ---------------------------------------------------------------------
// Task entry points
// ---------------------------------------------------------------------
void sensorTask(void *pvParameters);
void motorTask(void *pvParameters);
void displayTask(void *pvParameters);
void networkTask(void *pvParameters);

// Creates the mutex/queue and spins up all 4 tasks. Call once from setup().
void startTasks();

#endif // TASKS_H
