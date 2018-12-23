#include <MS6205.h>             // https://github.com/holzachr/MS6205-arduino-library
    
int const shiftRegisterLatchPin  = 15; // GPIO15 = Pin D8 on NodeMCU board. Pin 12 on 74HC595.
int const shiftRegisterClockPin  = 14; // GPIO14 = Pin D5 on NodeMCU board. Pin 11 on 74HC595.
int const shiftRegisterDataPin   = 13; // GPIO13 = Pin D7 on NodeMCU board. Pin 14 on 74HC595.
int const displaySetPositionPin  = 12; // GPIO12 = Pin D6 on NodeMCU board. Pin 16A on MS6205.
int const displaySetCharacterPin = 2;  // GPIO2  = Pin D4 on NodeMCU board. Pin 16B on MS6205.
int const displayClearPin        = 5;  // GPIO5  = Pin D1 on NodeMCU board. Pin 18A on MS6205.
int const displaySelectPage0Pin  = 4;  // GPIO4  = Pin D2 on NodeMCU board. Pin  2A on MS6205.
int const displaySelectPage1Pin  = 0;  // GPIO0  = Pin D3 on NodeMCU board. Pin  2B on MS6205.
  
// Display declaration  
MS6205 display(shiftRegisterLatchPin, shiftRegisterClockPin, shiftRegisterDataPin, displaySetPositionPin, displaySetCharacterPin, displayClearPin);
  
// Scroll text declaration:
// Define area from in row 0, columns 1 to 8.
// Scroll 1 character each 300 ms
// Shown text is "Demostring"
// Will be displayed on display declared a few lines above
scrollText demoString(1, 0, 8, 0, 200, "Demostring", &display);

// Another scroll text declaration:
// Define area from column 0, row 7 to column 3, row 8.
// Scroll 1 character each 300 ms
// Shown text is "Teststring"
// Will be displayed on display declared a few lines above
scrollText testString(0, 7, 3, 8, 300, "Teststring", &display);
  
void setup() 
{
  display.clear();
}
  
void loop()
{
  testString.update();
  demoString.update();
}
