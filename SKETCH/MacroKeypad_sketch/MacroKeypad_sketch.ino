/*******************************************************************
v 3.4.3 - 26.05.2021
change log:
refactored the color based filling patterns function by adding params - [ setColorsModeHUE( long, int) ]
added a 5th MODE - AUTOMATICALLY MOUSE MOVEMENT - YOU KNOW WHAT FOR !!!
v 3.4.2 - 25.05.2021
change log:
added color based filling patterns for Photoshop and Excel mode
added a rainbow filling effect for keypad mode
v 3.3 - 22.05.2021
change log:
update the display order of function's numbers to match the keypad numbers.
[in the switch function keys are numbered from top left (1) to bottom right (9),
but in order to match the classc keypad i changed the way they appear n the screen from bottom left (1) to upper right (9)]
in switch 1 2 3 on display 7 8 9
4 5 6 4 5 6
M 6 7 9 M 1 2 3
Pro Tips:
=============== Keyboard Control================================================================================
Keyboard.write(); Sends a keystroke to a connected computer.
This is similar to pressing and releasing a key on your keyboard.
Will send a shift command if applicable. Example: Keyboard.write('K') will
automatically do SHIFT + k.
Can also accept ASCII code like this:
//Keyboard.write(32); // This is space bar (in decimal)
Helpful list of ASCII + decimal keys http://www.asciitable.com/
Keyboard.press(); Best for holding down a key with multi-key commands; like copy/ paste
This example is [ctrl] + [shift] + [e]
//Keyboard.press(KEY_LEFT_CTRL);
//Keyboard.press(KEY_LEFT_SHIFT);
//Keyboard.press('e');
//delay(100); Keyboard.releaseAll();
Keyboard.print(); Sends a keystroke(s)
Keyboard.print("stop using aim bot"); // types this in as a char or int! (not a string)!
Keyboard.println(); Sends a keystroke followed by a newline (carriage return)
Very practical if you want to type a password and login in one button press!
SOMETIMES, applications are coded to recognize Keyboard.press() and not Keyboard.write() and vice versa.
You might have to experiment.
=============== Mouse Control================================================================================
Mouse.move(x, y, wheel); Moves the mouse and or scroll wheel up/down left/right.
Range is -128 to +127. units are pixels
-number = left or down
+number = right or up
Mouse.press(b); Presses the mouse button (still need to call release). Replace "b" with:
MOUSE_LEFT //Left Mouse button
MOUSE_RIGHT //Right Mouse button
MOUSE_MIDDLE //Middle mouse button
MOUSE_ALL //All three mouse buttons
Mouse.release(b); Releases the mouse button.
Mouse.click(b); A quick press and release.
**********************************************************************************************************/
// --------------------------------------------------------------
// Standard Libraries
// --------------------------------------------------------------
#include "Keyboard.h"
// Library with a lot of the HID definitions and methods
// Can be useful to take a look at it see whats available
// https://github.com/arduino-libraries/Keyboard/blob/master/src/Keyboard.h
#include <Mouse.h> //there are some mouse move functions for encoder_Mode 2 and 3
#include <Keypad.h>
// This library is for interfacing with the 3x3 Matrix
// Can be installed from the library manager, search for "keypad"
// and install the one by Mark Stanley and Alexander Brevig
// https://playground.arduino.cc/Code/Keypad/
const byte ROWS = 3; //three rows
const byte COLS = 3; //three columns
char previousPressedKey;
boolean hasReleasedKey = false; //use for button Hold mode. Only works with one button at a time for now...
#include <Encoder.h>
//Library for simple interfacing with encoders (up to two)
//low performance ender response, pins do not have interrupts
Encoder RotaryEncoderA(10, 16); //the LEFT encoder (encoder A)
// --------------------------------------------------------------
// Additional Libraries - each one of these will need to be installed to use the special features like i2c LCD and RGB LEDs.
// --------------------------------------------------------------
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address for a 40 chars and 4 line display
// Your LCD hardware address or type might be different... This LCD library might not work for your application
const int LCD_NB_ROWS = 4 ; //for the 4x20 LCD lcd.begin(), but i think this is kinda redundant
const int LCD_NB_COLUMNS = 20 ;
//----------------------------------------
//uncomment below if you plan to use 0.91' 128x32 OLED display and then comment the above section
/*
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2 (U8G2_R0, U8X8_PIN_NONE, 5, 4); // 2, 0 for ESP-01 : GPIO 2 -> SCL | GIPO 0 -> SDA
*/
/*
//rotate display at 180° (for U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2 (U8G2_R0, U8X8_PIN_NONE, 5, 4); with U8G2 Lib)
u8g2.clearDisplay();
u8g2.setDisplayRotation(U8G2_R0);
u8g2.setFlipMode(1);
*/
//----------------------------------------------------------------
unsigned long previousMillis = 0; // values to compare last time interval was checked (For LCD refreshing)
unsigned long currentMillis = millis(); // values to compare last time interval was checked (For LCD refreshing) and DemoTimer
int check_State = 0; // state to check trigger the demo interrupt
int updateLCD_flag = 0; // LCD updater, this flag is used to only update the screen once between mode changes
// and once every 3 second while in a mode. Saves cycles / resources
// --------------------------------------------------------------
#include <Adafruit_NeoPixel.h> //inclusion of Adafruit's NeoPixel (RBG addressable LED) library
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN A2 // Which pin on the Arduino is connected to the NeoPixels?
#define NUMPIXELS 10 // How many NeoPixels are attached to the Arduino? 10 total, but they are address from 0,1,2,...9.
#define BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int colorUpdate = 0; //setting a flag to only update colors once when the mode is switched.
long firstPixelHue = 0;
// --------------------------------------------------------------
char keys[ROWS][COLS] = {
{'1', '2', '3'}, // the keyboard hardware is a 3x3 grid
{'4', '5', '6'},
{'7', '8', '9'},
};
// The library will return the character inside this array when the appropriate
// button is pressed then look for that case statement. This is the key assignment lookup table.
// Layout(key/button order) looks like this
// |----------------------|
// | [ 1] [ 2] [ 3] | * Encoder A location = key[1]
// | [ 4] [ 5] [ 6] |
// | [ 7] [ 8] [ 9] | NOTE: The mode button is not row/column key, it's directly wired to A0!!
// |----------------------|
// --------------------------------------------------------------
// Variables that will change:
int modePushCounter = 0; // counter for the number of button presses
int buttonState = 0; // current state of the button
int lastButtonState = 0; // previous state of the button
int mouseMove; // variable that holds how many pixels to move the mouse cursor
//String password; // string for rando password generator, its a global variable because i might do something fancy with it?
long positionEncoderA = -999; //encoderA LEFT position variable
//long positionEncoderB = -999; //encoderB RIGHT position variable
const int ModeButton = A0; // the pin that the Modebutton is attached to
const int pot = A1; // pot for adjusting attract mode demoTime or mouseMouse pixel value
//const int Mode1= A2;
//const int Mode2= A3; //Mode status LEDs
byte rowPins[ROWS] = {4, 5, A3 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8 }; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
// --------------------------------------------------------------
void setup() {
lcd.init(); //initialize the 4x20 lcd
lcd.backlight(); //turn on the backlight
lcd.begin(LCD_NB_COLUMNS , LCD_NB_ROWS);
randomSeed(analogRead(0)); // is this a bad idea? it's called before assigning the button as pull up... the switch is open at rest..
//pinMode(ModeButton, INPUT_PULLUP); // initialize the button pin as a input:
pinMode(ModeButton, INPUT); // initialize the button pin as a input, must connect a 10k resistor to GND [ https://www.arduino.cc/en/Tutorial/BuiltInExamples/Button ]
Serial.begin(9600); // initialize serial communication:
lcd.setCursor(0, 0); lcd.print(" Macro KeyPad");
lcd.setCursor(0, 1); lcd.print(" (c) by marius_mym");
lcd.setCursor(6, 3); lcd.print("v 3.4.3");
delay(1000);
lcd.clear();
Keyboard.begin();
pixels.begin(); // This initializes the NeoPixel library.
/*
u8g2.begin();
u8g2.setContrast(0); // set contrast -> 0 lowest ; 255 highest
//rotate display at 180°
u8g2.clearDisplay();
u8g2.setDisplayRotation(U8G2_R0);
u8g2.setFlipMode(1);
*/
}
void loop() {
char key = keypad.getKey();
mouseMove = (analogRead(pot)); //reading the analog input, pot = pin A1
mouseMove = map(mouseMove, 0,1023, 1,124); //remap the analog pot values fron 1 to 124
checkModeButton();
switch (modePushCounter) { // switch between keyboard configurations:
//====================================================================================================================
//---------------------------------------MODE 1. Office and websurfing mode-------------------------------------------
//====================================================================================================================
case 0: // is case 0 so it should be mode 0, but for the final product is easier to name it as MODE 1
LCD_update_0(); //Mode 0 text for LCD,
encoderA_Mode0(); // turn VOLUME UP / DOWN
//setColorsMode0(); //indicate what mode is loaded by changing the key colors
// - rainbowMode
if (key) {
//Serial.println(key);
switch (key) {
case '1': Keyboard.pressRaw(0x7F); //Keyboard MUTE Volume
delay(50);
Keyboard.releaseRaw(0x7F);
pixels.setPixelColor(1, pixels.Color( 0,150,128));
break;
case '2': Keyboard.press(KEY_LEFT_GUI); //open Chrome [ KEY_LEFT_GUI = WIN key ]
Keyboard.press('r');
Keyboard.release(KEY_LEFT_GUI);
Keyboard.release('r');
delay(150); //give your system time to catch up with these android-speed keyboard presses
Keyboard.println("chrome");
pixels.setPixelColor(2, pixels.Color( 0,150,128));
break; //don't forget the break statement!
//RandoPasswordGenerator(); break; // generate a not-so-sophisticated password
case '3': Keyboard.press(KEY_LEFT_GUI); //open Photoshop
Keyboard.press('r');
Keyboard.release(KEY_LEFT_GUI);
Keyboard.release('r');
delay(150);
Keyboard.println("photoshop");
pixels.setPixelColor(3, pixels.Color( 0,150,128));
break;
case '4': Keyboard.press(KEY_LEFT_GUI); //open Gmail
Keyboard.press('r');
Keyboard.release(KEY_LEFT_GUI);
Keyboard.release('r');
delay(150);
Keyboard.println("chrome");
delay(2000);
Keyboard.println("gmail.com");
pixels.setPixelColor(4, pixels.Color( 0,150,128));
break;
case '5': Keyboard.press(KEY_LEFT_GUI); //open MS Word
Keyboard.press('r');
Keyboard.release(KEY_LEFT_GUI);
Keyboard.release('r');
delay(150);
Keyboard.println("winword");
pixels.setPixelColor(5, pixels.Color( 0,150,128));
break;
case '6': Keyboard.press(KEY_LEFT_GUI); //open MS Excel
Keyboard.press('r');
Keyboard.release(KEY_LEFT_GUI);
Keyboard.release('r');
delay(150);
Keyboard.println("excel.exe");
pixels.setPixelColor(6, pixels.Color( 0,150,128));
break;
case '7': Keyboard.press(KEY_LEFT_GUI); // SHOW DESKTOP (minimize all apps and show desktop)
Keyboard.press('d');
Keyboard.release(KEY_LEFT_GUI);
Keyboard.release('d');
pixels.setPixelColor(7, pixels.Color( 0,150,128));
break;
case '8': Keyboard.press(KEY_LEFT_GUI); // Snipping Tool
Keyboard.press(KEY_LEFT_SHIFT);
Keyboard.press('s');
Keyboard.release(KEY_LEFT_GUI);
Keyboard.release(KEY_LEFT_SHIFT);
Keyboard.release('s');
//Keyboard.press(KEY_DELETE);
pixels.setPixelColor(8, pixels.Color( 0,150,128));
break;
case '9': Keyboard.press(KEY_LEFT_GUI); // lock sesion
Keyboard.press('l');
Keyboard.release(KEY_LEFT_GUI);
Keyboard.release('l');
pixels.setPixelColor(9, pixels.Color( 0,150,128));
break;
}
pixels.show(); //update the color after the button press
delay(100);
Keyboard.releaseAll(); // this releases the buttons
delay(100); //delay a bit to hold the color (optional)
colorUpdate = 0; //call the color update to change the color back to Mode settings
}
break;
//====================================================================================================================
//------------------------------------------MODE 2. Photoshop macro keys mode-----------------------------------------
//====================================================================================================================
case 1:
encoderA_Mode0(); // turn VOLUME UP / DOWN
//encoderA_Mode2(); //move mouse
//setColorsModeRainbow();
setColorsModeHUE(32000, 1200); //32000 = first HUE val from Blue spectre ; 1200 - incrementing value // 63000 = first HUE val from Red spectre ; 1000 - incrementing value
LCD_update_1(); //Mode 1 text for LCD
if (key) {
//Serial.println(key);
switch (key) {
case '1': Keyboard.pressRaw(0x7F); //Keyboard MUTE Volume
delay(50);
Keyboard.releaseRaw(0x7F);
pixels.setPixelColor(1, pixels.Color(150,0,0));
break;
case '2': Keyboard.press('z'); // zoom in
Keyboard.release('z');
Mouse.click();
pixels.setPixelColor(2, pixels.Color(150,0,0));
break;
case '3': Keyboard.press('z'); // zoom out
Keyboard.release('z');
Keyboard.press(KEY_LEFT_ALT);
delay(100);
Mouse.click();
delay(100);
Keyboard.release(KEY_LEFT_ALT);
pixels.setPixelColor(3, pixels.Color(150,0,0));
break;
case '4': Keyboard.press(KEY_LEFT_CTRL); //new layer
Keyboard.press(KEY_LEFT_SHIFT);
Keyboard.press('n');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release(KEY_LEFT_SHIFT);
Keyboard.release('n');
pixels.setPixelColor(4, pixels.Color(150,0,0));
break;
case '5': Keyboard.press(KEY_LEFT_CTRL); //make a copy of present layer
Keyboard.press('j');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release('j');
pixels.setPixelColor(5, pixels.Color(150,0,0));
break;
case '6': Keyboard.press(KEY_LEFT_CTRL); // invert selection
Keyboard.press('i');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release('i');
pixels.setPixelColor(6, pixels.Color(150,0,0));
break;
case '7': Keyboard.press(KEY_LEFT_CTRL); // merge down
Keyboard.press('e');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release('e');
pixels.setPixelColor(7, pixels.Color(150,0,0));
break;
case '8': Keyboard.press(KEY_LEFT_CTRL); // merge visible
Keyboard.press(KEY_LEFT_SHIFT);
Keyboard.press('e');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release(KEY_LEFT_SHIFT);
Keyboard.release('e');
pixels.setPixelColor(8, pixels.Color(150,0,0));
break;
case '9': Keyboard.press(KEY_LEFT_CTRL); // show/hide grid
Keyboard.pressRaw(0x27); //0x27 = ' character
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.releaseRaw(0x27);
pixels.setPixelColor(9, pixels.Color(150,0,0));
break;
}
pixels.show(); //update the color after the button press
delay(100);
Keyboard.releaseAll(); // this releases the buttons
delay(100); //delay a bit to hold the color (optional)
colorUpdate = 0; //call the color update to change the color back to Mode settings
}
break;
//====================================================================================================================
//------------------------------------------------- MODE 3. Excel ----------------------------------------------------
//====================================================================================================================
case 2:
encoderA_Mode2(); //moves mouse (sensitivity) based on the analog pot setting
LCD_update_2(); // Mode 2 text for LCD
//setColorsMode2(); // set color layout for this mode
setColorsModeHUE(12922, 2000); // 12922 = first HUE val from Green spectre ; 2000 - incrementing value
if (key) {
switch (key) {
case '1':
Keyboard.pressRaw(0x7F); //Keyboard MUTE Volume
delay(50);
Keyboard.releaseRaw(0x7F);
break;
case '2':
Keyboard.press(KEY_LEFT_CTRL); //open the Paste Special dialog box
Keyboard.press(KEY_LEFT_ALT);
Keyboard.press('v');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release(KEY_LEFT_ALT);
Keyboard.release('v');
break;
case '3':
Keyboard.press(KEY_LEFT_ALT); // add border around selected cells
Keyboard.press('h');
delay(50);
Keyboard.release(KEY_LEFT_ALT);
Keyboard.release('h');
delay(50);
Keyboard.write('b');
delay(50);
Keyboard.write('a');
break;
case '4':
Keyboard.press(KEY_LEFT_CTRL); // format cell value as GENERAL
Keyboard.press(KEY_LEFT_SHIFT);
Keyboard.press('`');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release(KEY_LEFT_SHIFT);
Keyboard.release('`');
break;
case '5':
Keyboard.press(KEY_LEFT_CTRL); // format cell value as NUMBER
Keyboard.press(KEY_LEFT_SHIFT);
Keyboard.press('1');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release(KEY_LEFT_SHIFT);
Keyboard.release('1');
break;
case '6':
Keyboard.press(KEY_LEFT_CTRL); // open format window
Keyboard.press('1');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release('1');
break;
case '7':
Keyboard.press(KEY_LEFT_ALT); // Sum selected cells
Keyboard.press('=');
Keyboard.release(KEY_LEFT_ALT);
Keyboard.release('=');
break;
case '8':
Keyboard.press(KEY_LEFT_CTRL); // open insert dialog winow
Keyboard.press(KEY_LEFT_SHIFT);
Keyboard.press('5');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release(KEY_LEFT_SHIFT);
Keyboard.release('5');
break;
case '9':
Keyboard.press(KEY_LEFT_CTRL); // insert function Arguments [For example, if you type "=DATE(" and then use Control + Shift + A, Excel give you "=DATE(year,month,day)".
Keyboard.press(KEY_LEFT_SHIFT); // You can then double-click each argument and change it to the address or value you need.]
Keyboard.press('5');
Keyboard.release(KEY_LEFT_CTRL);
Keyboard.release(KEY_LEFT_SHIFT);
Keyboard.release('5');
break;
}
delay(50);
Keyboard.releaseAll(); // this releases the buttons
}
break;
//====================================================================================================================
//------------------------------------------------- MODE 4.Classic Keypad --------------------------------------------
//====================================================================================================================
case 3:
LCD_update_3();
//setColorsModeRainbow(); // this function is called from checkModeButton() function because it needs to be called on every loop.
//setColorsModeBlue();
//setColorsMode3();
encoderA_Mode4();
if (key) {
switch (key) {
case '1':
Keyboard.write(0x37); //hex value of 7
break;
case '2':
Keyboard.write(0x38);//hex value of 8
break;
case '3':
Keyboard.write(0x39);
break;
case '4':
Keyboard.write(0x34);
break;
case '5':
Keyboard.write(0x35);
break;
case '6':
Keyboard.write(0x36);
break;
case '7':
Keyboard.write(0x31);
break;
case '8':
Keyboard.write(0x32);
break;
case '9':
Keyboard.write(0x33);
break;
}
delay(50);
Keyboard.releaseAll(); // this releases the buttons
}
break;
//====================================================================================================================
//--------------------------------------- MODE 5.Automatically moving mouse ------------------------------------------
//====================================================================================================================
case 4: // this mode helps you when you "work" from home by mimic the mouse movement
//setColorsModeRainbow();
setColorsModeHUE(62000, 1200); // 63000 = first HUE val from Green spectre ; 1000 - incrementing value
LCD_update_4();
Mouse.move(-mouseMove,0,0);
delay(30);
Mouse.move(mouseMove,0,0);
delay(20);
break;
}
delay(1); // delay in between reads for stability
}
//====================================================================================================================
//====================================================================================================================
//------------------------------------------Sub Routine Section-------------------------------------------------------
//====================================================================================================================
//====================================================================================================================
//=======================================================================================
//====================== Neopixels Update Routines ===============================
//=======================================================================================
void setColorsModeRainbow(){
for(int i=0; i<pixels.numPixels(); i++) { // this function creates a rainbow pattern
int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue, 255, 170)));
//firstPixelHue += 256; // update outside this fucntion to avoid using delay.
}
pixels.show();
}
void setColorsModeHUE(long firstHueColorValue, int incrementValue){ // please see https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use#hsv-hue-saturation-value-colors-dot-dot-dot-3024464-41
if (colorUpdate == 0){
for(int i=0; i<pixels.numPixels(); i++) {
pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(firstHueColorValue, 255, 170)));
firstHueColorValue += incrementValue; // increment hue
}
pixels.show();
colorUpdate=1; // neoPixels have been updated.
}
}
void setColorsMode0(){
if (colorUpdate == 0){ // have the neopixels been updated?
/*
for(int i = 0; i<NUMPIXELS; i++){
pixels.setPixelColor(i, pixels.Color( 0,145, 12));
}
*/
pixels.setBrightness(170);
pixels.setPixelColor(0, pixels.Color(150, 0,214)); // purple[for OFFICE MODE]
pixels.setPixelColor(1, pixels.Color(150, 0, 0)); // red [for mute on click ]
pixels.setPixelColor(2, pixels.Color(150, 0, 0)); // red-ish [for chrome]
pixels.setPixelColor(3, pixels.Color( 0, 64,115)); // dark blue [for Photoshop]
pixels.setPixelColor(4, pixels.Color(176, 30, 26)); // dark red [for Gmail]
pixels.setPixelColor(5, pixels.Color( 24, 87,166)); // blue [for Word]
pixels.setPixelColor(6, pixels.Color( 0 ,150, 18)); // green [for Excel]
pixels.setPixelColor(7, pixels.Color( 0, 0,140)); // [for Show Desktop]
pixels.setPixelColor(8, pixels.Color(227,227, 2)); // yellow-ish [Snipping Tool]
pixels.setPixelColor(9, pixels.Color(153, 0, 0)); // dark red [for Lock Sesion]
pixels.show();
colorUpdate=1; // neoPixels have been updated.
}
// Set the flag to 1; so they are not updated until a Mode change
}
/*
void setColorsMode1(){
if (colorUpdate == 0){ // have the neopixels been updated?
pixels.setPixelColor(0, pixels.Color( 0, 64,115)); // dark blue [for PHOTOSHOP MODE]
pixels.setPixelColor(1, pixels.Color( 0, 81,148)); // slightly lighter blue
pixels.setPixelColor(2, pixels.Color( 0, 81,148));
pixels.setPixelColor(3, pixels.Color( 0, 81,148));
pixels.setPixelColor(4, pixels.Color( 0, 81,148));
pixels.setPixelColor(5, pixels.Color( 0, 81,148));
pixels.setPixelColor(6, pixels.Color( 0, 81,148));
pixels.setPixelColor(7, pixels.Color( 0, 81,148));
pixels.setPixelColor(8, pixels.Color( 0, 81,148));
pixels.setPixelColor(9, pixels.Color( 0, 81,148));
pixels.show(); // neoPixels have been updated.
colorUpdate=1; // Set the flag to 1; so they are not updated until a Mode change
}
}
void setColorsMode2(){
if (colorUpdate == 0){ // have the neopixels been updated?
pixels.setPixelColor(0, pixels.Color( 51,102, 0));
pixels.setPixelColor(1, pixels.Color( 0, 0,150));
pixels.setPixelColor(2, pixels.Color( 0,150, 0));
pixels.setPixelColor(3, pixels.Color(150, 0, 0));
pixels.setPixelColor(4, pixels.Color(166,110, 27));
pixels.setPixelColor(5, pixels.Color(150, 0,150));
pixels.setPixelColor(6, pixels.Color( 0, 80,150));
pixels.setPixelColor(7, pixels.Color( 0, 80,150));
pixels.setPixelColor(8, pixels.Color( 80,102, 0));
pixels.show(); // neoPixels have been updated.
colorUpdate=1; // Set the flag to 1; so they are not updated until a Mode change
}
}
void setColorsMode3(){
if (colorUpdate == 0){ // have the neopixels been updated?
pixels.setPixelColor(0, pixels.Color( 51, 95, 158)); // red
pixels.setPixelColor(1, pixels.Color( 0, 0, 150)); // green
pixels.setPixelColor(2, pixels.Color( 0,100, 0)); // blue
pixels.setPixelColor(3, pixels.Color( 80,102, 0)); // yellow
pixels.setPixelColor(4, pixels.Color(120, 0,100)); // magenta
pixels.setPixelColor(5, pixels.Color(150, 0, 10)); // red
pixels.setPixelColor(6, pixels.Color(165, 75, 0)); // orange
pixels.setPixelColor(7, pixels.Color( 80,102, 0)); // yellow
pixels.setPixelColor(8, pixels.Color(120, 0,100)); // magenta
pixels.setPixelColor(9, pixels.Color( 0,150,128)); // cyan-blue
pixels.show();
colorUpdate=1; } // neoPixels have been updated.
// Set the flag to 1; so they are not updated until a Mode change
}
*/
//=======================================================================================
//========================== CHECK MODE BUTTON ===================================
//=======================================================================================
void checkModeButton(){
buttonState = digitalRead(ModeButton);
if (buttonState != lastButtonState) { // compare the buttonState to its previous state
if (buttonState == LOW) { // if the state has changed, increment the counter
// if the current state is LOW then the button cycled:
modePushCounter++;
Serial.println("pressed");
Serial.print("number of button pushes: ");
Serial.println(modePushCounter);
updateLCD_flag = 0; // forces a screen refresh
colorUpdate = 0; // set the color change flag ONLY when we know the mode button has been pressed.
// Saves processor resources from updating the neoPixel colors all the time
}
delay(50); // Delay a little bit to avoid bouncing
}
lastButtonState = buttonState; // save the current state as the last state, for next time through the loop
if (modePushCounter >4){ //reset the counter after 5 presses (count from 0) CHANGE THIS FOR MORE MODES
modePushCounter = 0;}
if(modePushCounter == 0 || modePushCounter == 3){ // I assigned setColorsModeRainbow()(rainbow pattern mode) to Mode 4 - Keypad Mode. These lines checks if that mode is the current mode
firstPixelHue += 128; // increment led Hue
setColorsModeRainbow(); // call rainbow function - this particular function needs to be clled every single time in order to constantly update neopixels.
if (firstPixelHue >= 3*65536){ //prevent overfall
firstPixelHue = 0;
}
delay(10); // this small delay control the rainbow circle speed
}
}
//=======================================================================================
//======================== Encoder Definitions Assignments ==============================
//=======================================================================================
//this section allows a unique encoder function for each mode (profile)
//=============Encoder A Function ====== Mode 0 =========================================================
void encoderA_Mode0(){
long newPos = RotaryEncoderA.read()/4; //When the encoder lands on a valley, this is an increment of 4.
// your encoder might be different (divide by 2) i dunno.
if (newPos != positionEncoderA && newPos > positionEncoderA) {
positionEncoderA = newPos;
Keyboard.pressRaw(0x80); //Keyboard Volume Up
Keyboard.releaseRaw(0x80);
pixels.setPixelColor(1, pixels.Color(140,0,140));
pixels.show();
delay(100); //delay a bit to hold the color (optional)
colorUpdate = 0;
// Keyboard.press(KEY_RIGHT_ARROW);
//Keyboard.release(KEY_RIGHT_ARROW);
}
if (newPos != positionEncoderA && newPos < positionEncoderA) {
positionEncoderA = newPos;
Keyboard.pressRaw(0x81); //Keyboard Volume Down
Keyboard.releaseRaw(0x81);
pixels.setPixelColor(1, pixels.Color(140,0,140));
pixels.show();
delay(100); //delay a bit to hold the color (optional)
colorUpdate = 0;
//Keyboard.press(KEY_LEFT_ARROW);
//Keyboard.release(KEY_LEFT_ARROW);
}
}
//=============Encoder A Function ====== Mode 1 =========================================================
void encoderA_Mode1(){
long newPos = RotaryEncoderA.read()/2;
if (newPos != positionEncoderA && newPos < positionEncoderA) {
positionEncoderA = newPos;
//tab increase
Keyboard.write(9); //tab key
}
if (newPos != positionEncoderA && newPos > positionEncoderA) {
positionEncoderA = newPos;
//tab decrease
Keyboard.press(KEY_LEFT_SHIFT);
Keyboard.write(9); //tab key
Keyboard.release(KEY_LEFT_SHIFT); }
}
//=============Encoder A Function ====== Mode 2 =========================================================
void encoderA_Mode2(){ //testing some encoder wheel play control for arcade games; centede, tempest...
long newPos = RotaryEncoderA.read()/2;
if (newPos != positionEncoderA && newPos > positionEncoderA) {
positionEncoderA = newPos;
//Serial.println(mouseMove);
Mouse.move(-mouseMove,0,0); //moves mouse right... Mouse.move(x, y, wheel) range is -128 to +127
}
if (newPos != positionEncoderA && newPos < positionEncoderA) {
positionEncoderA = newPos;
Mouse.move(mouseMove,0,0); //moves mouse left... Mouse.move(x, y, wheel) range is -128 to +127
}
}
//=============Encoder A Function ====== Mode 3 =========================================================
void encoderA_Mode3(){
long newPos = RotaryEncoderA.read()/2;
if (newPos != positionEncoderA && newPos > positionEncoderA) {
positionEncoderA = newPos;
Mouse.press(MOUSE_LEFT); //holds down the mouse left click
Mouse.move(-4,0,0); //moves mouse left... Mouse.move(x, y, wheel) range is -128 to +127
Mouse.release(MOUSE_LEFT); //releases mouse left click
}
if (newPos != positionEncoderA && newPos < positionEncoderA) {
positionEncoderA = newPos;
Mouse.press(MOUSE_LEFT); //holds down the mouse left click
Mouse.move(4,0,0); //moves mouse right... Mouse.move(x, y, wheel) range is -128 to +127
Mouse.release(MOUSE_LEFT); //releases mouse left click
}
}
//=============Encoder A Function ====== Mode 4 =========================================================
void encoderA_Mode4(){
long newPos = RotaryEncoderA.read()/4; //When the encoder lands on a valley, this is an increment of 4. your encoder might be different (divide by 2)
if (newPos != positionEncoderA && newPos > positionEncoderA) {
positionEncoderA = newPos;
Keyboard.write(0x30); //hex value of 0
// Keyboard.press(KEY_RIGHT_ARROW);
//Keyboard.release(KEY_RIGHT_ARROW);
}
if (newPos != positionEncoderA && newPos < positionEncoderA) {
positionEncoderA = newPos;
Keyboard.write(0x30); //enter 3 zeros [ 000 ]
delay(5);
Keyboard.write(0x30);
delay(5);
Keyboard.write(0x30);
//Keyboard.press(KEY_LEFT_ARROW);
//Keyboard.release(KEY_LEFT_ARROW);
}
}
//=======================================================================================
//======================== LCD Update Routines ===================================
//=======================================================================================
//------------------------------------------MODE 1. Office and websurfing
void LCD_update_0() { //This method is less heavy on tying up the arduino cycles to update the LCD; instead
//this updates the LCD every 3 seconds. Putting the LCD.write commands
//in the key function loops breaks the 'feel' and responsiveness of the keys.
unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= 3000) { // if the elasped time greater than 3 seconds
previousMillis = currentMillis; // save the last time you checked the interval
switch (updateLCD_flag) {
case 0:
lcd.clear();
lcd.setCursor(0, 0); lcd.print("MODE 1 -GENERAL MODE");
lcd.setCursor(0, 1); lcd.print("7.Mute|8.Chorme|9.Ph");
lcd.setCursor(0, 2); lcd.print("4.MAIL|5.Word|6.Excl");
lcd.setCursor(0, 3); lcd.print("1.DESK|2.Snip|3.Lock");
/*
u8g2.clearBuffer();
writeInCells("MUTE","Chrome", "PhotoSh",
"gMail","Word","Excel",
"ShowDSK","SnipTl","LockDSK");
*/
updateLCD_flag = 1; // stops the LCD from updating every x seconds.
// will not run again until the Mode key is pressed.
break;
}
}
}
//------------------------------------------MODE 2. Photoshop macro keys mode
void LCD_update_1() {
unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= 3000) { // if the elasped time greater than 3 seconds
previousMillis = currentMillis; // save the last time you checked the interval
switch (updateLCD_flag) {
case 0:
lcd.clear();
lcd.setCursor(0, 0); lcd.print("MODE 2 - Photoshop ");
lcd.setCursor(0, 1); lcd.print("7.Mute | 8. + | 9. -");
lcd.setCursor(0, 2); lcd.print("4.nLyrn|5.cLyr|6.inv");
lcd.setCursor(0, 3); lcd.print("1.mrgDN|2.mrgV|3.Grd");
/*
u8g2.clearBuffer();
writeInCells("MUTE All","Zoom In", "Zoom Out",
"NewLyr","CopyLyr","Inverse",
"MrgDown","MrgVisib","S/H Grid");
*/
updateLCD_flag = 1;
break;
}
}
}
//------------------------------------------MODE 3 - Excel macro
void LCD_update_2() {
unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= 3000) { //if the elasped time greater than 3 seconds
previousMillis = currentMillis; // save the last time you checked the interval
switch (updateLCD_flag) {
case 0:
lcd.clear();
lcd.setCursor(0, 0); lcd.print("MODE 3 - Excel macro");
lcd.setCursor(0, 1); lcd.print("7: Mute "); // Mute
lcd.setCursor(0, 2); lcd.print("8: Paste special box"); // open the Paste Special dialog box
lcd.setCursor(0, 3); lcd.print("9: Brdr around cells"); // add border around selected cells
/*
u8g2.clearBuffer();
writeInCells("MUTE All","PasteBox", "BorderCl",
"CellAsGen","CellAsNo","FrmatWd",
"SumCells","InsrtWnd","FuncArgs");
*/
updateLCD_flag = 1;
break;
case 1:
lcd.clear();
lcd.setCursor(0, 0); lcd.print("MODE 3 - Excel macro");
lcd.setCursor(0, 1); lcd.print("4: Cell val= GENERAL"); // format cell value as GENERAL
lcd.setCursor(0, 2); lcd.print("5: Cell val = NUMBER"); // format cell value as NUMBER
lcd.setCursor(0, 3); lcd.print("6: Open format windw"); // open format window
updateLCD_flag = 2;
break;
case 2:
lcd.clear();
lcd.setCursor(0, 0); lcd.print("MODE 3 - Excel macro");
lcd.setCursor(0, 1); lcd.print("1:Sum selected cells"); // Sum selected cells
lcd.setCursor(0, 2); lcd.print("2:Open insert winow "); // open insert dialog winow
lcd.setCursor(0, 3); lcd.print("3:Ins func arguments"); // insert function Arguments
updateLCD_flag = 0;
break;
}
}
}
//------------------------------------------MODE 4 - Keypad Mode
void LCD_update_3() { // not used in the auto-attract mode
unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= 3000) { //if the elasped time greater than 3 seconds
previousMillis = currentMillis;
switch (updateLCD_flag) {
case 0:
lcd.clear();
lcd.setCursor(0, 0); lcd.print("MODE 4 - Keypad Mode");
lcd.setCursor(0, 1); lcd.print("Encoder| 7 | 8 | 9 |"); // Mute
lcd.setCursor(0, 2); lcd.print("> [0] | 4 | 5 | 6 |"); // open the Paste Special dialog box
lcd.setCursor(0, 3); lcd.print("< [000]| 1 | 2 | 3 |"); // add border around selected cells
updateLCD_flag = 1;
/*
u8g2.clearBuffer();
keypadModeOLED();
*/
updateLCD_flag = 1;
break;
/* case 1:
lcd.clear();
updateLCD_flag = 2;
break;
case 2:
lcd.clear();
updateLCD_flag = 0;
break;
*/
}
}
}
void LCD_update_4(){
unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= 3000) { //if the elasped time greater than 3 seconds
previousMillis = currentMillis;
switch (updateLCD_flag) {
case 0:
lcd.clear();
lcd.setCursor(0, 0); lcd.print("MODE 5- Mouse moving");
lcd.setCursor(0, 1); lcd.print(" Mouse is moving ");
lcd.setCursor(0, 2); lcd.print("automatically- press");
lcd.setCursor(0, 3); lcd.print("'MODE' button to end");
updateLCD_flag = 1;
break;
}
}
}