#include "blynk_config.h" // BLYNK_TEMPLATE_NAME / BLYNK_DEVICE_NAME / BLYNK_PRINT
#include "config.h"       // BLYNK_TEMPLATE_ID / WIFI_SSID / WIFI_PASSWORD / BLYNK_AUTH_TOKEN — must come before BlynkSimpleEsp32.h
#include "tasks.h"
#include <WiFi.h>
// main.cpp is the one translation unit that defines the global `Blynk`
// object. BlynkSimpleEsp32.h defines it non-static on every #include, so
// including it a second time without this guard causes a linker error
// ("multiple definition of `Blynk'"). NO_GLOBAL_BLYNK makes this include
// just declare `extern BlynkWifi Blynk;` instead of defining it again.
#define NO_GLOBAL_BLYNK
#include <BlynkSimpleEsp32.h>
#include "sensor.h"
#include "motor.h"
#include "OLED_display.h"

SemaphoreHandle_t stateMutex;
SystemState systemState = {OBSTACLE_NONE, false, false};

QueueHandle_t motorCmdQueue;

static SemaphoreHandle_t speedMutex;
static int commandedSpeedLeft = 0;
static int commandedSpeedRight = 0;

// ---------------------------------------------------------------------
// Shared state accessors
// ---------------------------------------------------------------------
SystemState getSystemState() {
  SystemState copy;
  xSemaphoreTake(stateMutex, portMAX_DELAY);
  copy = systemState;
  xSemaphoreGive(stateMutex);
  return copy;
}

void setObstacleLevel(short level) {
  xSemaphoreTake(stateMutex, portMAX_DELAY);
  systemState.obstacleLevel = level;
  xSemaphoreGive(stateMutex);
}

void setWifiConnected(bool connected) {
  xSemaphoreTake(stateMutex, portMAX_DELAY);
  systemState.wifiConnected = connected;
  xSemaphoreGive(stateMutex);
}

void setBlynkConnected(bool connected) {
  xSemaphoreTake(stateMutex, portMAX_DELAY);
  systemState.blynkConnected = connected;
  xSemaphoreGive(stateMutex);
}

void setCommandedSpeed(int speedLeft, int speedRight) {
  xSemaphoreTake(speedMutex, portMAX_DELAY);
  commandedSpeedLeft = speedLeft;
  commandedSpeedRight = speedRight;
  xSemaphoreGive(speedMutex);
}

void getCommandedSpeed(int &speedLeft, int &speedRight) {
  xSemaphoreTake(speedMutex, portMAX_DELAY);
  speedLeft = commandedSpeedLeft;
  speedRight = commandedSpeedRight;
  xSemaphoreGive(speedMutex);
}

void requestMotorAction(MotorAction action, int speedLeft, int speedRight) {
  MotorCommand cmd = {action, speedLeft, speedRight};
  // Don't block Blynk's callback if the queue is briefly full — dropping
  // one button-press tick is fine, stalling Blynk.run() is not.
  xQueueSend(motorCmdQueue, &cmd, 0);
}

// ---------------------------------------------------------------------
// Sensor task — core 1, owns the ultrasonic sensor exclusively
// ---------------------------------------------------------------------
void sensorTask(void *pvParameters) {
  initSensor();
  for (;;) {
    short level = readSensor();
    if (level != OBSTACLE_SENSOR_ERROR) {
      setObstacleLevel(level);
    } else {
      Serial.println("WARNING: Sensor disconnected!");
    }

    // Safety-critical: don't wait for the next Blynk button release,
    // cut the motors the instant something is too close.
    if (level == OBSTACLE_DANGER) {
      MotorCommand stopCmd = {MOTOR_STOP, 0, 0};
      xQueueSend(motorCmdQueue, &stopCmd, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ---------------------------------------------------------------------
// Motor task — core 1, the only task that touches the motor driver
// ---------------------------------------------------------------------
void motorTask(void *pvParameters) {
  motorInit();
  MotorCommand cmd;
  for (;;) {
    if (xQueueReceive(motorCmdQueue, &cmd, portMAX_DELAY) == pdTRUE) {
      // Safety override: refuse to drive forward into a confirmed
      // close-range obstacle, no matter what the app requested.
      if (cmd.action == MOTOR_FORWARD &&
          getSystemState().obstacleLevel == OBSTACLE_DANGER) {
        motorStop();
        continue;
      }

      switch (cmd.action) {
        case MOTOR_FORWARD:  motorForward(cmd.speedLeft, cmd.speedRight); break;
        case MOTOR_BACKWARD: motorBackward(cmd.speedLeft, cmd.speedRight); break;
        case MOTOR_LEFT:     motorTurnLeft(cmd.speedLeft, cmd.speedRight); break;
        case MOTOR_RIGHT:    motorTurnRight(cmd.speedLeft, cmd.speedRight); break;
        case MOTOR_STOP:
        default:              motorStop(); break;
      }
    }
  }
}

// ---------------------------------------------------------------------
// Display task — core 0, lowest priority, purely cosmetic
// ---------------------------------------------------------------------
void displayTask(void *pvParameters) {
  init_OLED();
  for (;;) {
    SystemState state = getSystemState();
    if (!state.wifiConnected || !state.blynkConnected) {
      loader_display();
    } else if (state.obstacleLevel == OBSTACLE_NEAR ||
               state.obstacleLevel == OBSTACLE_DANGER) {
      danger_display();
    } else {
      cool_display();
    }
    // FRAME_DELAY (OLED_display.h) already paces the animation via delay(),
    // which yields on ESP32 Arduino, so no extra vTaskDelay needed here.
  }
}

// ---------------------------------------------------------------------
// Network task — core 0, owns WiFi + Blynk connection and Blynk.run()
// ---------------------------------------------------------------------
void networkTask(void *pvParameters) {
  const TickType_t checkInterval = pdMS_TO_TICKS(10000);
  TickType_t lastCheck = xTaskGetTickCount();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  for (;;) {
    if (xTaskGetTickCount() - lastCheck > checkInterval) {
      if (WiFi.status() != WL_CONNECTED) {
        setWifiConnected(false);
        Serial.println("Lose connection!");
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        TickType_t start = xTaskGetTickCount();
        while (WiFi.status() != WL_CONNECTED &&
               xTaskGetTickCount() - start < pdMS_TO_TICKS(10000)) {
          vTaskDelay(pdMS_TO_TICKS(50));
        }
      }

      if (WiFi.status() == WL_CONNECTED) {
        setWifiConnected(true);
        if (!Blynk.connected()) {
          Serial.println("Try to connect Blynk again!");
          Blynk.connect();
        }
      }
      lastCheck = xTaskGetTickCount();
    }

    if (Blynk.connected()) {
      setBlynkConnected(true);
      Blynk.run(); // BLYNK_WRITE callbacks fire synchronously from here
    } else {
      setBlynkConnected(false);
      Blynk.connect(100);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ---------------------------------------------------------------------
// Task creation
// ---------------------------------------------------------------------
void startTasks() {
  stateMutex = xSemaphoreCreateMutex();
  speedMutex = xSemaphoreCreateMutex();
  motorCmdQueue = xQueueCreate(8, sizeof(MotorCommand));

  // Priorities: motor > sensor/network > display. Motor must preempt
  // everything else so an emergency stop is never queued up behind a
  // slow OLED frame.
  xTaskCreatePinnedToCore(motorTask,   "motorTask",   2048, nullptr, 3, nullptr, 1);
  xTaskCreatePinnedToCore(sensorTask,  "sensorTask",  2048, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(networkTask, "networkTask", 6144, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(displayTask, "displayTask", 3072, nullptr, 1, nullptr, 0);
}