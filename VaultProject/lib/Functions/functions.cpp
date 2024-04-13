#include <Arduino.h>

#include "../../.pio/libdeps/uno/Adafruit SSD1306/Adafruit_SSD1306.h"
#include "../../.pio/libdeps/uno/Adafruit GFX Library/Adafruit_GFX.h"
#include "../../.pio/libdeps/uno/Servo/src/Servo.h"
#include <SPI.h>
#include <Wire.h>
#include "functions.h"
#include "../config.h"
#include <EEPROM.h>


byte SEV_SEG_DIGITS[] = { // Array to represent digits from 0-9 in 7-segment display (1 is off, 0 is on)
        // ABCDEFG
        0b00000011, // 0
        0b10011111, // 1
        0b00100101, // 2
        0b00001101, // 3
        0b10011001, // 4
        0b01001001, // 5
        0b11000001, // 6
        0b00011111, // 7
        0b00000001, // 8
        0b00011001  // 9
};


void to_binary(int decimal, int binary[4]) {
    for (int i = 3; i >= 0; i--) {
        binary[i] = decimal % 2;
        decimal /= 2;
    }
}

void rot_counter(int *counter, int pin_a, int pin_b) {
    unsigned long time = millis();
    static unsigned long prevTime = 0;

    static int lastState_A = 1;
    int currState_A;
    // Read the current state of CLK
    currState_A = digitalRead(pin_a);

    // If last and current state of CLK are different, then pulse occurred
    // React to only 1 state change to avoid double count
    if (currState_A != lastState_A && currState_A == 1 && time - prevTime > DEBOUNCE) {
        // If the DT state is different from the CLK state then
        // the encoder is rotating CCW so decrement
        if (digitalRead(pin_b) != currState_A) {
            *counter = *counter +1;
        } else {
            // Encoder is rotating CW so increment
            *counter = *counter -1;
        }
    }

    // Remember last CLK state
    lastState_A = currState_A;


    if (*counter > 9) {
        *counter = 0;
    } else if (*counter < 0) {
        *counter = 9;
    }
}
//int rot_counter(int pin_a, int pin_b) {
//    static int counter = 0;
//
//    unsigned long time = millis();
//    static unsigned long prevTime = 0;
//
//    static int lastState_A = 1;
//    int currState_A;
//    // Read the current state of CLK
//    currState_A = digitalRead(pin_a);
//
//    // If last and current state of CLK are different, then pulse occurred
//    // React to only 1 state change to avoid double count
//    if (currState_A != lastState_A && currState_A == 1 && time - prevTime > DEBOUNCE) {
//        // If the DT state is different from the CLK state then
//        // the encoder is rotating CCW so decrement
//        if (digitalRead(pin_b) != currState_A) {
//            counter++;
//        } else {
//            // Encoder is rotating CW so increment
//            counter--;
//        }
//        Serial.println(counter);
//    }
//
//    // Remember last CLK state
//    lastState_A = currState_A;
//
//
//    if (counter > 9) {
//        counter = 0;
//    } else if (counter < 0) {
//        counter = 9;
//    }
//    return counter;
//}


bool rot_button_pressed(int rot_butt_pin) {
    unsigned long time = millis();
    static unsigned long prevTime = 0;

    int new_state = digitalRead(rot_butt_pin);
    static int old_state = 1;

    if (new_state != old_state && time - prevTime > DEBOUNCE) {
        old_state = new_state;
        prevTime = time;
        if (new_state == 0) {
            Serial.println("Pressed");
            return true;
        }
    }
    return false;
}

void display_number(const int *digits, const int data_pin, const int clk_pin, const int latch_pin, const int *bcd_pin) {
    shiftOut(data_pin, clk_pin, LSBFIRST,
             SEV_SEG_DIGITS[digits[1]]); // Outputs the number to display for 2nd second shift register
    shiftOut(data_pin, clk_pin, LSBFIRST,
             SEV_SEG_DIGITS[digits[0]]); // Outputs the number to display for 1st second shift register

    digitalWrite(latch_pin, HIGH);
    digitalWrite(latch_pin, LOW);

    // Outputs the number to display for BCD decoder
    int Bin_arr[4];
    to_binary(digits[2], Bin_arr);

    for (int i = 0; i < 4; i++) {
        if (Bin_arr[i] == 1) {
            digitalWrite(bcd_pin[i], HIGH);
        } else {
            digitalWrite(bcd_pin[i], LOW);
        }
    }
}

bool validate_password(const int currPassword[3], const int password[3]) {
    if (currPassword[0] == password[0] && currPassword[1] == password[1] && currPassword[2] == password[2]) {
        return true;
    }
    return false;
}


bool is_door_open(int door_sensor_pin) {
    if(digitalRead(door_sensor_pin) == HIGH){
        return true;
    }
    return false;
}


void light_decimal_point(const int *indicators_pins, int indicator) {
    for (int i = 0; i < 3; i++) {
        digitalWrite(indicators_pins[i], HIGH);
    }
    digitalWrite(indicators_pins[indicator], LOW);
}

void enterLockdown(const int ADDRESS_COOLDOWN, const int time_o, const int sr_data_pin, const int sr_clk_pin,
                   const int sr_latch_pin, const int *bcd_pin) {
    digitalWrite(greenLED_PIN, LOW);
    digitalWrite(redLED_PIN, HIGH);
    int secondsLeft = time_o;
    unsigned long time = 0;
    unsigned long prevTime = millis();

    while (secondsLeft > 0) {
        time = millis();
        if (time - prevTime > 1000) {
            EEPROM.update(ADDRESS_COOLDOWN, secondsLeft);
            prevTime = millis();
            secondsLeft--;
            int secondsLeftArr[3];
            secondsLeftArr[0] = secondsLeft / 100; // 1st digit
            secondsLeftArr[1] = (secondsLeft / 10) % 10; //2nd digit
            secondsLeftArr[2] = secondsLeft % 10; // 3rd digit
            display_number(secondsLeftArr, sr_data_pin, sr_clk_pin, sr_latch_pin, bcd_pin);
        }
    }
}


int rot_button_time_pressed(int rot_button_pin) {
    unsigned long time = millis();
    static unsigned long prevTime = 0;
    static unsigned long pressTime = 0;

    static int old_state = 1;
    int new_state = digitalRead(rot_button_pin);

    if (new_state != old_state && time - prevTime > DEBOUNCE) {
        old_state = new_state;
        prevTime = time;
        if (new_state == 0) {
            pressTime = time;
        } else if (new_state == 1) {
            return time - pressTime;
        }
    }
    return -1;
}

