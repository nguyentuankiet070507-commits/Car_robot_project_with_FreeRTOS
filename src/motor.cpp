#include "motor.h"
#include <Arduino.h>

// Pin definitions (unchanged from v1)
const int IN1 = 12;
const int IN2 = 14;
const int IN3 = 27;
const int IN4 = 26;

const int ENA = 13;
const int ENB = 25;

const int ch_ENA = 0;
const int ch_ENB = 1;

const int freq = 2000;
const int resolution = 8;
const int MAX_SPEED = 200;

void motorInit() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(ch_ENA, freq, resolution);
  ledcSetup(ch_ENB, freq, resolution);

  ledcAttachPin(ENA, ch_ENA);
  ledcAttachPin(ENB, ch_ENB);
}

void motorForward(int speedLeft, int speedRight) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(ch_ENA, speedLeft);
  ledcWrite(ch_ENB, speedRight);
}

void motorBackward(int speedLeft, int speedRight) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(ch_ENA, speedLeft);
  ledcWrite(ch_ENB, speedRight);
}

void motorTurnLeft(int speedLeft, int speedRight) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(ch_ENA, speedLeft);
  ledcWrite(ch_ENB, speedRight);
}

void motorTurnRight(int speedLeft, int speedRight) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(ch_ENA, speedLeft);
  ledcWrite(ch_ENB, speedRight);
}

void motorStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  ledcWrite(ch_ENA, 0);
  ledcWrite(ch_ENB, 0);
}
