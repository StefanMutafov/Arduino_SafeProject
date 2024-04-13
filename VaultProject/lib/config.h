#include <Arduino.h>
#include "../.pio/libdeps/uno/Servo/src/Servo.h"
#include "../.pio/libdeps/uno/Adafruit SSD1306/Adafruit_SSD1306.h"
#include "../.pio/libdeps/uno/Adafruit GFX Library/Adafruit_GFX.h"

#ifndef VAULTPROJECT_CONFIG_H
#define VAULTPROJECT_CONFIG_H


// Pin connections for Arduino
const int DOT_PINS[3] = {A1, A2, A3};
const int BCD_PINS[4] = {4, 5, 6, 7};
#define BUZZER_PIN 12
#define redLED_PIN 1
#define greenLED_PIN 0
#define SERV_PIN A0
#define DOOR_PIN 10
#define ROT_PIN_A 3
#define ROT_PIN_B 9
#define ROT_BUTT_PIN 2
#define SR_DATA_PIN 8
#define SR_CLK_PIN 11
#define SR_LATCH_PIN 13
#define DEBOUNCE 100 // Debounce time of 100 ms for rotary encoder
#define OLED_RESET (-1)
#define OLED_WIDTH 128
#define OLED_HEIGHT 64


#endif //VAULTPROJECT_CONFIG_H