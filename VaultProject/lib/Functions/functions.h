//
// Created by Stefan on 15/12/2023.
//

#ifndef VAULTPROJECT_FUNCTIONS_H
#define VAULTPROJECT_FUNCTIONS_H


/// Converts the binary value of a decimal number
/// @paramm decimal The decimal number to be transformed
/// @param binary The array that will be set to the binary number
void to_binary(int decimal, int binary[4]);

//
///// Returns a value of a counter when the rotary encoder is rotated
///// @param PIN_A First pin of the rotary encoder
///// @param PIN_B Second pin of the rotary encoder
///// @return returns the current number of rotary encoder position
//int rot_counter(int pin_a, int pin_b);


/// Returns a value of a counter when the rotary encoder is rotated
/// @param PIN_A First pin of the rotary encoder
/// @param PIN_B Second pin of the rotary encoder
/// @return returns the current number of rotary encoder position
void rot_counter(int *counter,int pin_a, int pin_b);


/// Detects when the rotary encoder is pressed
/// @param rot_button_pin pin of the button
/// @return True if pressed, False if not pressed
bool rot_button_pressed(int rot_button_pin);

/// Compare two password
/// @param currPassword The password you want checked
/// @param password The master password
/// @return True if the passwords are the same, false otherwise
bool validate_password(const int currPassword[3], const int password[3]);

/// Returns the time a buttons has been held for
/// @param rot_button_pin pin of the button
/// @return The time the button has been pressed for or -1 if it hasn't
int rot_button_time_pressed(int rot_button_pin);

/// Detects when the door is open
/// @param door_sensor_pin pin of the button
/// @return True if the door is open, false otherwise
bool is_door_open(int door_sensor_pin);


/// Lights a decimal point for the active 7-segment displays
/// @param indicators_pins pointer to an array with the indicator pins
/// @param indicator The indicator to be lit
void light_decimal_point(const int indicators_pins[], int indicator);


/// Displays a given number on the 7-segment displays
/// @param digits Array with the digits to be displayed
/// @param DATA_PIN the DATA pin for the Shift Registers
/// @param CLK_PIN The Clock pin for the Sift Registers
/// @param LATCH_PIN The Latch pin for the Shift Registers
/// @param BCD_PINS The pins for the BCD
void display_number(const int *digits, int data_pin, int clk_pin, int latch_pin, const int *bcd_pins);


/// Enters lockdown after failing to input correct password 3 times
/// @param eeAddress_Time Position in the EEPROM "array" where the int variable storing the remaining lockdown time is located.
void enterLockdown(const int ADDRESS_COOLDOWN, const int time_o, const int sr_data_pin, const int sr_clk_pin,
                   const int sr_latch_pin, const int *bcd_pin);

void oled_two_strings(const char *string1, const char *string2, int size);

void oled_foofie(void);

#endif //VAULTPROJECT_FUNCTIONS_H
