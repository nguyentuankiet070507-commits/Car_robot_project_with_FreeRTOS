#ifndef BLYNK_CONFIG_H
#define BLYNK_CONFIG_H

// These BLYNK_* macros must be defined *before* BlynkSimpleEsp32.h is
// included, in every .cpp file that includes it. Centralizing them here
// avoids the two definitions drifting apart between main.cpp and tasks.cpp.
#define BLYNK_TEMPLATE_NAME "CAR ROBOT CONTROLLER"
#define BLYNK_DEVICE_NAME "CAR ROBOT CONTROLLER"
#define BLYNK_PRINT Serial

#endif // BLYNK_CONFIG_H
