#ifndef MOTOR_H
#define MOTOR_H

// ---- Pin map (unchanged from v1) ----
extern const int IN1;
extern const int IN2;
extern const int IN3;
extern const int IN4;
extern const int ENA;
extern const int ENB;

extern const int ch_ENA;
extern const int ch_ENB;

extern const int freq;
extern const int resolution;
extern const int MAX_SPEED;

// v2: motor functions take explicit speeds instead of reading shared
// globals. This makes them safe to call from any task without a mutex,
// since the caller owns the values it passes in.
void motorInit();
void motorForward(int speedLeft, int speedRight);
void motorBackward(int speedLeft, int speedRight);
void motorTurnLeft(int speedLeft, int speedRight);
void motorTurnRight(int speedLeft, int speedRight);
void motorStop();

#endif // MOTOR_H
