#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include "../.pio/libdeps/uno/Servo/src/Servo.h"
#include "../lib/Functions/functions.h"
#include "../lib/config.h"
#include "../.pio/libdeps/uno/Adafruit SSD1306/Adafruit_SSD1306.h"
#include "../.pio/libdeps/uno/Adafruit SSD1306/splash.h"
#include "../.pio/libdeps/uno/Adafruit GFX Library/Adafruit_GFX.h"

// Global variables
// Variables storing EEPROM (memory) positions for variables below
const int ADDRESS_PASSWORD[3] = {0, 1, 2};
const int ADDRESS_ATTEMPTS = 3;
const int ADDRESS_COOLDOWN = 4;

int password[3]; // Initialise password

Servo myServo;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);


///Function that displays 2 lines of texts on the right side of OLED-display
///@param string1 The first string to be printed
///@param string2 The second string to be printed
void oled_two_strings(const char *string1, const char *string2, int size);


/// A function that prints the company on the left of the OLED
void oled_foofie(void);


// Initialisations of components
void setup() {
   
    pinMode(BUZZER_PIN, OUTPUT);

    pinMode(redLED_PIN, OUTPUT);
    pinMode(greenLED_PIN, OUTPUT);

    pinMode(SR_DATA_PIN, OUTPUT);
    pinMode(SR_CLK_PIN, OUTPUT);
    pinMode(SR_LATCH_PIN, OUTPUT);

    pinMode(ROT_PIN_A, INPUT);
    pinMode(ROT_PIN_B, INPUT);
    pinMode(ROT_BUTT_PIN, INPUT_PULLUP);

    pinMode(DOOR_PIN, INPUT_PULLUP);

    for (int i: BCD_PINS) {
        pinMode(i, OUTPUT);

    }
    for (int i: DOT_PINS) {
        pinMode(i, OUTPUT);
    }


    myServo.write(0);
    if(is_door_open(DOOR_PIN)){
        myServo.write(90);
    }
    myServo.attach(SERV_PIN);


    int start[] = {0, 0, 0};
    // Read password from memory
    password[0] = EEPROM.read(ADDRESS_PASSWORD[0]);
    password[1] = EEPROM.read(ADDRESS_PASSWORD[1]);
    password[2] = EEPROM.read(ADDRESS_PASSWORD[2]);
    // During the beginning phase, set initial password to 0. This is done, because EEPROM sets values to 255, and we cannot display this value.
    if (password[0] == 255) {
        password[0] = 0;
    }
    if (password[1] == 255) {
        password[1] = 0;
    }
    if (password[2] == 255) {
        password[2] = 0;
    }

    display_number(start, SR_DATA_PIN, SR_CLK_PIN, SR_LATCH_PIN, BCD_PINS);

    //Initialising the OLED-display
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display_number(start, SR_DATA_PIN, SR_CLK_PIN, SR_LATCH_PIN, BCD_PINS);
    display.clearDisplay();

    oled_foofie();
    oled_two_strings("Enter", "password:", 1);
    display.display();


}


/*
 Refer to high-level flowchart following this format:
 *state name*
 ... code ...
*/
void loop() {
    static int counter = 0; // Initialise the counter variable for rotary encoder
    rot_counter(&counter,ROT_PIN_A, ROT_PIN_B); // Update the variable the with the digit function returns
    static int lastCounter = 0; // Initialise the last counter variable to remember

    static int currPassword[3] = {0, 0, 0};

    static int activeDisplay = 0; // Active 7-segment display

    static bool doorOpened = false;

    static bool locked = true;
    //Check if the door is open
    if(is_door_open(DOOR_PIN)){
        digitalWrite(greenLED_PIN, HIGH);
        digitalWrite(redLED_PIN, LOW);
        locked = false;
    }

    // In case of power outage, we read the memory for how many attempts are left
    static int unlockAttempts = EEPROM.read(ADDRESS_ATTEMPTS);
    if (unlockAttempts == 255) {
        unlockAttempts = 0;
    }

    // In case of power outage, we read the memory if the program has to enter lockdown
    int activeCooldown = EEPROM.read(ADDRESS_COOLDOWN);
    if (activeCooldown == 255) {
        activeCooldown = 0;
    }
    if (activeCooldown != 0) {
        //Setting OLED text
        display.clearDisplay();
        oled_foofie();
        oled_two_strings("Lockdown\n         mode", "Time left:", 1);
        display.display();

        enterLockdown(ADDRESS_COOLDOWN, EEPROM.read(ADDRESS_COOLDOWN), SR_DATA_PIN, SR_CLK_PIN, SR_LATCH_PIN, BCD_PINS);
        EEPROM.update(ADDRESS_COOLDOWN, 0);


        //Setting OLED text
        display.clearDisplay();
        oled_foofie();
        oled_two_strings("Enter", "password:", 1);
        display.display();
    }
    EEPROM.update(ADDRESS_ATTEMPTS, unlockAttempts);



    // State locked of high-level flowchart
    if (locked) {
        light_decimal_point(DOT_PINS, activeDisplay); // The decimal point on the active 7-segment display lights up
        digitalWrite(redLED_PIN, HIGH);
        digitalWrite(greenLED_PIN, LOW);

        // State enter code
        // Display the digit user inputted
        if (counter != lastCounter) {
            currPassword[activeDisplay] = counter;
            display_number(currPassword, SR_DATA_PIN, SR_CLK_PIN, SR_LATCH_PIN, BCD_PINS);
            lastCounter = counter;
        }

        // Change the active 7-segment display to the next display
        if (rot_button_pressed(ROT_BUTT_PIN)) {
            counter = 0;
            activeDisplay++;
        }

        // activeDisplay variable start at 0
        if (activeDisplay > 2) {

            // State correct == YES
            if (validate_password(currPassword, password)) {
                counter = 0;
                digitalWrite(greenLED_PIN, HIGH);
                digitalWrite(redLED_PIN, LOW);
                myServo.write(90); // Turn the servo motor to open the lock
                locked = false;
                unlockAttempts = 0; // Reset the password attempts
                display.clearDisplay();

                //Setting OLED text
                oled_foofie();
                oled_two_strings("Welcome!", "You can now", 1);
                display.setCursor(55, 43);
                display.print(" open");
                display.setCursor(55, 53);
                display.print(" the door!");
                display.display();

                // State correct == NO
            } else {
                //Setting OLED text
                display.clearDisplay();
                oled_foofie();
                oled_two_strings("Wrong\n         password!", "Attempts", 1);
                display.setCursor(55, 43);
                display.print("left:");
                display.display();

                unlockAttempts++;
                // Array to reset the digits to 0 on 7-segment display
                int unlockAttemptsArr[3] = {0, 0, 3 - unlockAttempts};
                for (int i = 0; i < 5; i++) {
                    // Displays how many attempts are left
                    display_number(unlockAttemptsArr, SR_DATA_PIN, SR_CLK_PIN, SR_LATCH_PIN, BCD_PINS);
                    // LEDs blink and buzzer produces sound
                    digitalWrite(redLED_PIN, HIGH);
                    tone(BUZZER_PIN, 30);
                    delay(200);
                    noTone(BUZZER_PIN);
                    digitalWrite(redLED_PIN, LOW);
                    delay(200);
                }
                //Setting OLED text
                display.clearDisplay();
                oled_foofie();
                oled_two_strings("Enter", "password:", 1);
                display.display();
            }
            // Reset the active 7-segment display and current password
            activeDisplay = 0;
            currPassword[0] = 0;
            currPassword[1] = 0;
            currPassword[2] = 0;
            display_number(currPassword, SR_DATA_PIN, SR_CLK_PIN, SR_LATCH_PIN, BCD_PINS);
        }
        // State failed attempts > 3 ?
        if (unlockAttempts == 3) {
            //Setting OLED text
            display.clearDisplay();
            oled_foofie();
            oled_two_strings("Lockdown\n         mode", "Time left:", 1);
            display.display();

            unlockAttempts = 0; // Reset the unlock attempts to 0
            EEPROM.update(ADDRESS_ATTEMPTS, unlockAttempts); // Update the memory
            // State security measure
            enterLockdown(ADDRESS_COOLDOWN, 15, SR_DATA_PIN, SR_CLK_PIN, SR_LATCH_PIN, BCD_PINS);
            //Setting OLED text
            display.clearDisplay();
            oled_foofie();
            oled_two_strings("Enter", "password:", 1);
            display.display();
        }

        // State unlocked
    } else {
        // Check if the door is open
        if (is_door_open(DOOR_PIN)) {
            doorOpened = true;
            //Setting OLED text
            display.clearDisplay();
            oled_foofie();
            oled_two_strings("Door  open!", "Hold 3 sec", 1);
            display.setCursor(55, 43);
            display.print("to change");
            display.setCursor(55, 53);
            display.print(" password");
            display.display();
        }

        // The program detected that the door is closed, which in turn, will lock the vault
        if (doorOpened && !is_door_open(DOOR_PIN)) {
            //Setting OLED text
            display.clearDisplay();
            oled_foofie();
            oled_two_strings("Enter", "password:", 1);
            display.display();

            doorOpened = false;
            myServo.write(0);
            locked = true;
            display_number(currPassword, SR_DATA_PIN, SR_CLK_PIN, SR_LATCH_PIN, BCD_PINS);
        }

        // TODO: combine these 2 if statements into one
        if (doorOpened) {

            // State Button pressed >3 seconds
            if (rot_button_time_pressed(ROT_BUTT_PIN) > 2000) {
                //Setting OLED text
                display.clearDisplay();
                oled_foofie();
                oled_two_strings("Choose new", "password:", 1);
                display.display();

                // State Enter new code
                while (activeDisplay < 3) {
                    rot_counter(&counter,ROT_PIN_A, ROT_PIN_B);
                    light_decimal_point(DOT_PINS, activeDisplay);

                    if (counter != lastCounter) {
                        currPassword[activeDisplay] = counter;
                        display_number(currPassword, SR_DATA_PIN, SR_CLK_PIN, SR_LATCH_PIN, BCD_PINS);
                        lastCounter = counter;
                    }

                    if (rot_button_pressed(ROT_BUTT_PIN)) {
                        counter = 0;
                        activeDisplay++;
                    }

                }

                // Updates the old password with a new one
                // TODO: test this loop
                for (int i = 0; i < 3; ++i) {
                    password[i] = currPassword[i];
                    EEPROM.update(ADDRESS_PASSWORD[i], password[i]);
                    currPassword[i] = 0;
                }
                activeDisplay = 0;
                for (int i = 0; i < 2; i++) {
                    // LEDs blink and buzzer produces sound
                    digitalWrite(greenLED_PIN, HIGH);
                    tone(BUZZER_PIN, (i+20)*200);
                    delay(100);
                    noTone(BUZZER_PIN);
                    digitalWrite(greenLED_PIN, LOW);
                    delay(100);
                }
            }
        }

    }

}

void oled_two_strings(const char *string1, const char *string2, int size) {
    display.setTextColor(WHITE);
    display.setTextWrap(false);
    display.setTextSize(size);

    // TODO: make this an enum
    if (size == 2) {
        // Big string
        display.setCursor(62, 20);
        display.print(string1);
        display.setCursor(62, 31);
        display.print(string2);
    } else {
        // Small string
        display.setCursor(55, 14);
        display.print(string1);
        display.setCursor(55, 33);
        display.print(string2);
        // display.setCursor(57, 42); // Third small string
    }
}

void oled_foofie(void) {
    static const unsigned char PROGMEM doofen[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x0c, 0xe1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff,
                                                   0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xe0, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x3f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xdf, 0xc0,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc7, 0xe0, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x1f, 0x8e, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
                                                   0x42, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x41, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x21, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21,
                                                   0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x71, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x24, 0x3b, 0xc0, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x28, 0x1e, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xa8, 0x71,
                                                   0x80, 0x00, 0x00, 0x00, 0x00, 0x02, 0x68, 0xeb, 0x40, 0x00, 0x00,
                                                   0x00, 0x00, 0x02, 0xa4, 0x69, 0x40, 0x00, 0x00, 0x00, 0x00, 0x02,
                                                   0x64, 0x14, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x43, 0xe3, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x01, 0xe5, 0x07, 0xe0, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x2c, 0xc0, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c,
                                                   0xa7, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x98, 0xe2, 0x00,
                                                   0x00, 0x00, 0x00, 0x18, 0x47, 0x84, 0x99, 0x00, 0x00, 0x00, 0x00,
                                                   0x27, 0xc4, 0xe7, 0x87, 0x80, 0x00, 0x00, 0x03, 0xe8, 0x02, 0x1c,
                                                   0x00, 0x00, 0x00, 0x00, 0x0c, 0xa8, 0x01, 0x8a, 0x00, 0x00, 0x00,
                                                   0x00, 0x19, 0x6b, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x00, 0x23, 0x66,
                                                   0x78, 0x00, 0x60, 0x00, 0x00, 0x00, 0x62, 0x7a, 0x07, 0x00, 0x60,
                                                   0x00, 0x00, 0x00, 0xc2, 0xfa, 0x00, 0xc0, 0x40, 0x00, 0x00, 0x00,
                                                   0x84, 0xf9, 0x00, 0x30, 0x20, 0x00, 0x00, 0x01, 0x04, 0xf9, 0x00,
                                                   0x0c, 0x20, 0x00, 0x00, 0x01, 0x06, 0xfa, 0x00, 0x03, 0x20, 0x00,
                                                   0x00, 0x02, 0x03, 0xf9, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x02, 0x0c,
                                                   0xf8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x02, 0x08, 0xf8, 0x80, 0x00,
                                                   0x00, 0x00, 0x00, 0x04, 0x88, 0xfc, 0xc0, 0x00, 0x7e, 0x00, 0x00,
                                                   0x04, 0x88, 0xfd, 0x20, 0x00, 0x81, 0x00, 0x00, 0x04, 0x88, 0xfd,
                                                   0x20, 0x01, 0x3e, 0x00, 0x00, 0x04, 0x88, 0xfd, 0x10, 0x02, 0x42,
                                                   0x00, 0x00, 0x0c, 0x88, 0xfd, 0x10, 0x02, 0x47, 0xf0, 0x00, 0x08,
                                                   0x88, 0xfd, 0x90, 0x02, 0x88, 0x18, 0x00, 0x08, 0x8c, 0xff, 0x48,
                                                   0x02, 0x8b, 0xf4, 0x00, 0x08, 0x84, 0xff, 0x44, 0x02, 0x52, 0x0a,
                                                   0x00, 0x08, 0x84, 0xff, 0x24, 0x01, 0x24, 0xfa, 0x00, 0x08, 0x84,
                                                   0xff, 0x92, 0x01, 0x29, 0x0e, 0x00, 0x08, 0x86, 0xff, 0x4b, 0x81,
                                                   0x06, 0x1b, 0x00, 0x18, 0x82, 0xff, 0x48, 0x40, 0x80, 0xe5, 0x00,
                                                   0x10, 0x82, 0xff, 0xa4, 0x20, 0x87, 0x0c, 0x80, 0x10, 0x83, 0xff,
                                                   0xa2, 0x20, 0x84, 0x02, 0x80, 0x10, 0x81, 0xff, 0xf2, 0x10, 0x84,
                                                   0x04, 0x80, 0x10, 0x80, 0x00, 0x0b, 0x88, 0x84, 0x01, 0x00, 0x10,
                                                   0x80, 0x00, 0x08, 0x87, 0x04, 0xc2, 0x00, 0x10, 0x40, 0x00, 0x07,
                                                   0x40, 0x41, 0x04, 0x00, 0x10, 0x38, 0x00, 0x01, 0xf0, 0x81, 0x08,
                                                   0x00, 0x10, 0x07, 0x80, 0x01, 0x1f, 0x00, 0x10, 0x00, 0x10, 0x00,
                                                   0xf0, 0x01, 0x00, 0xee, 0x20, 0x00};
    display.drawBitmap(-2, 0, doofen, 64, 64, WHITE);
}