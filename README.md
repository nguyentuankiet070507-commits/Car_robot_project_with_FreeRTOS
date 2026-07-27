# Car Robot Project — v2 (FreeRTOS multitasking)

This branch (`feature/freertos-v2`) rewrites the control loop from a single
`loop()` that does everything sequentially into four dedicated FreeRTOS
tasks. Same hardware, same wiring, same Blynk virtual pins — the change is
entirely in how the firmware schedules its own work.

## Why

In v1, `loop()` did WiFi/Blynk housekeeping, ran `Blynk.run()`, read the
ultrasonic sensor (which blocks on `pulseIn` for up to 30ms), and drove the
OLED animation (which blocks on `delay(40)` per frame) — all one after
another, every iteration. A slow OLED frame delayed the next sensor read;
a stalled sensor read delayed Blynk responsiveness. None of it was ever
going to race, but there was also no isolation if one part misbehaved.

## Architecture

| Task | Core | Priority | Responsibility |
|------|------|----------|-----------------|
| `motorTask` | 1 | 3 (highest) | Only task that touches the L298N. Drains `motorCmdQueue`. |
| `sensorTask` | 1 | 2 | Owns the HC-SR04 exclusively. Publishes obstacle level to `systemState`. Pushes an emergency `MOTOR_STOP` straight onto the queue if something is too close. |
| `networkTask` | 0 | 2 | WiFi reconnect logic + `Blynk.run()`. Blynk's `BLYNK_WRITE` callbacks fire from inside this task. |
| `displayTask` | 0 | 1 (lowest) | Renders loader/cool/danger OLED animation from `systemState`. Purely cosmetic — never blocks anything safety-relevant. |

### Shared state (`tasks.h` / `tasks.cpp`)

- `SystemState` (obstacle level, WiFi status, Blynk status) is protected by
  `stateMutex`. Read it with `getSystemState()`, write it with the
  `setXxx()` helpers — never touch `systemState` directly.
- `motorCmdQueue` (`QueueHandle_t`, 8 deep) carries `MotorCommand{action,
  speedLeft, speedRight}` from Blynk callbacks and the sensor task to
  `motorTask`.
- Commanded speed (from the `V0` slider) lives behind its own
  `speedMutex`, separate from `stateMutex`, since it's updated far more
  often than obstacle/connection status.

### Why motor priority is highest

`motorTask` blocks on `xQueueReceive(..., portMAX_DELAY)` — it does
nothing until a command arrives, so giving it top priority costs nothing
in idle CPU time, but guarantees an emergency stop from `sensorTask`
preempts a slow-rendering OLED frame or a WiFi reconnect loop instead of
queuing up behind them.

### Safety notes carried over from v1, now enforced in two places

- `sensorTask` still enqueues a direct `MOTOR_STOP` the instant distance
  drops to "danger" range (mirrors v1's `stopMotor()` call in `loop()`).
- `motorTask` *also* refuses to execute a `MOTOR_FORWARD` command if the
  last known obstacle level is "danger" — so even a `V1` button press that
  was already in flight when the obstacle appeared can't override the
  stop.

## What's unchanged

- Pin map, PWM setup, HC-SR04 wiring/voltage divider, OLED wiring.
- `include/config.h` (WiFi/Blynk secrets) — still gitignored, still
  required before building. See the main README's Step 2.
- Blynk virtual pin assignments (`V0`–`V4`).
- `platformio.ini` dependencies.

## What changed

- `motor.h`/`motor.cpp`: functions now take `(speedLeft, speedRight)`
  explicitly instead of reading global `speed1`/`speed2`, so they're safe
  to call from a task without a mutex.
- `sensor.h`: obstacle levels are now a named `enum` (`OBSTACLE_NONE`,
  `OBSTACLE_NEAR`, `OBSTACLE_DANGER`, `OBSTACLE_SENSOR_ERROR`) instead of
  magic numbers `0`/`1`/`2`/`-1`.
- New `tasks.h`/`tasks.cpp`: the task bodies, shared state, and queue.
- `main.cpp`: now just the Blynk callbacks (which enqueue commands) and
  `startTasks()` in `setup()`. `loop()` is left idling.
- New `blynk_config.h`: centralizes the `BLYNK_TEMPLATE_NAME` etc. macros
  that must precede `BlynkSimpleEsp32.h`, since both `main.cpp` and
  `tasks.cpp` now include it.

## Building

Identical to v1 — same `platformio.ini`, same `include/config.h` you
already have from setting up v1. `Ctrl+Alt+B` / `Ctrl+Alt+U` in
PlatformIO.
