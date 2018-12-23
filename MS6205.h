/*
  MS6205.h - Library for controlling a MS6205 vintage soviet character display.
  
  Copyright 2018 Christian Holzapfel
  
  Released under the MIT License.
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  
  
  ABOUT THE DISPLAY
  ===================
   The "Elektronika MS6205" display is a multi-line cold-cathode-tube display with internal high-voltage generation.
   It has a visible area of 10 x 10 cm, and supports 160 characters in 10 rows with 16 characters each.
   Parallel data and address busses are used to set the cursor position and the character to display there.
   
   It runs of separate +5 V and +12 V supplies.
   
   This library assumes that both the address and data bus are connected in parallel, where the data is
   provided by the Arduino through a 74HC595 shift register.  
   
   
  SUPPORTED CHARACTERS
  ======================
   ASCII characters 32..127d are supported.                                                                                                     
   Character codes 96-126 are assigned to cyrillic characters instead of `,lower latin chars and {|}~.                                          
   Code 127 defines a fully black box.                                                                                                          
                                                                                                                                                
   Code       32    33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63                                
   Character  Space !  "  #  Ãƒâ€šÃ‚Â¤  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?                                 
                                                                                                                                                
   Code       64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95                                   
   Character  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _                                    
                                                                                                                                                
   Code       96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127       
   Character  Ю  А  Б  Ц  Д   Е   Ф   Г   Х   И   Й   К   Л   М   Н   О   П   Я   Р   С   Т   У   Ж   В   Ь   Ы   З   Ш   Э   Щ   Ч   █
   
   
  CURSOR (optional)
  =======================
    MS6205 can show a black box at the cursor's current position.
    If control line 8A is low, the cursor is hidden.
    If control line 8A is high, a black box is shown.  
  
  
  PAGING (optional)
  =======================
    MS6205 supports up to 4 pages.                                                                            
    Writing is always done to the current (visible) page. You cannot fill pages invisibly in background.      
    After filling, the pages remain defined (until redefined) and can be selected randomly.                   
    Pages are selected by inverted control lines 2A and 2B, which have pull-ups.                              
                                                                                                              
       2A  |  2B  | Page                                                                                      
     ------|------|------                                                                                     
      HIGH | HIGH | 0                                                                                         
      LOW  | HIGH | 1                                                                                         
      HIGH | LOW  | 2                                                                                         
      LOW  | LOW  | 3                                                                                         
                                                                                                              
    1. First select a first page to write to                                                                  
    2. Optionally clear it                                                                                    
    3. Write to page                                                                                          
    4. Select next page. Visible output changes, but page from step 1. remains in MS6205 memory               
    5. Optionally clear it                                                                                    
    6. Write to next page                                                                                     
    7. Now you can toggle between the two defined pages                                                       
    8. Optionally repeat above steps to define more pages    
    
    
  SOCKET PIN ORDER
  ===========================
  
          +-----------------------------------------------------------------------------------------------+
     B    |32|31|30|  |28|27|26|  |24|23|22|  |20|19|18|  |16|15|14|  |12|11|10|  | 8| 7| 6|  | 4| 3| 2|  |    B
       +-----------------------------------------------------------------------------------------------------+
     A |  |32|31|30|  |28|27|26|  |24|23|22|  |20|19|18|  |16|15|14|  |12|11|10|  | 8| 7| 6|  | 4| 3| 2|  |  | A
       +-----------------------------------------------------------------------------------------------------+     
                               View from the outside into the device pin header
  
      
    
  ELECTRICAL CONNECTION
  =========================== 
            
    NodeMCU v3
    module with                              74HC595                              MS6205
    ESP8266 CPU                              shift register                       display
    +-----------------+                      +------------+                       +------------------------------------+
    |                 |  Serial clock        |            |  Data 1-7, Addr 1-8   | Data Addr                          |
    |     GPIO14 / D5 +--------------------> | 11      15 +---------------------> | 22A, 8B                            |
    |                 |                      |          1 |---------------------> | 22B, 7B                            |
    |                 |  Serial data         |          2 |---------------------> | 20B, 10B                           |
    |     GPIO13 / D7 +--------------------> | 14       3 |---------------------> | 24A, 14A                           |
    |                 |                      |          4 |---------------------> | 23B, 12B                           |
    |                 |  Serial latch        |          5 |---------------------> | 18B, 11B                           |
    |     GPIO15 / D8 +--------------------> | 12       6 |---------------------> | 24B, 14B                           |
    |                 |                      |          7 +---------------------> |      20A                           | 
    |                 |                      |            |                       |                                    |
    |             +3V +--------------------> | 10, 16     |                       |                                    | Leave open                  
    |                 |                      |            |                       |                 /Busy 19B          +--------                  
    |                 |                      |            |                       |                                    |
    |             GND +---------+----------> | 8, 13      |                       |                                    |         ^ +5V
    |                 |         |            |            |                       |                 Incr. col addr 6B  |         |
    |                 |        --- GND       +------------+                       |                 Decr. col addr 6A  +---------+                   
    |                 |                                                           |                                    |
    |                 |                                                           |                                    |         ^ +5V
    |                 |                                                           |                 +5V                |         |
    |                 |  /Clear                                                   |                 26B, 27B, 28B,     +---------+
    |     GPIO 5 / D1 +---------------------------------------------------------> | 18A             26A, 28A           |
    |                 |                                                           |                                    |         ^ +12V
    |                 |  /Set position                                            |                 +12V               |         |
    |     GPIO12 / D6 +---------------------------------------------------------> | 16A             3A, 4A, 7A, 3B, 4B +---------+
    |                 |                                                           |                                    |
    |                 |  /Set character                                           |                 GND                |
    |     GPIO 2 / D4 +---------------------------------------------------------> | 16B             11A, 15A, 19A,     +---------+
    |                 |                                                           |                 23A, 27A, 30A      |         |
    |                 |  /Select page 0 (optional, leave open if unused)          |                                    |        --- GND
    |     GPIO 4 / D2 +---------------------------------------------------------> | 2A              GND+12V            |
    |                 |                                                           |                 30B, 31B, 32B,     +---------+
    |                 |  /Select page 1 (optional, leave open if unused)          |                 31A, 32A           |         |
    |     GPIO 0 / D3 +---------------------------------------------------------> | 2B                                 |        --- GND
    |                 |                                                           |                 Reset row addr 10A |
    |                 |  Show cursor (optional, conn. to GND if unused)           |                 Reset col addr 12A +---------+
    |     GPIO16 / D0 +---------------------------------------------------------> | 8A              Incr. row addr 15B |         |
    |                 |                                                           |                                    |        --- GND
    +-----------------+                                                           +------------------------------------+
        
    
  SIGNAL DESCRIPTION
  ========================
  
    A logical "0" (LOW) level is registered on address, data and other control lines at a voltage between -0.5 and 0.4 V.
    A logical "1" (HIGH) level is registered on address, data and other control lines at a voltage between 2.4 and 5.5 V.
    For this reason, control from a +3.3 V ESP8266 module works fine, even with the 74HC595 supplied with only +3.3 V.
    
    The bits on data bus are inverted. So to write an 'F', send hex 0x39 instead of usually 0x46.
    
    The lower bits 1-4 on address bus indicate the column (0-15), the upper bits 5-8 address the row (0-9. 
    
  
    +-----+------------------------------------------------------------------------------------------------------+
    | Pin | Signal               | Description                                                                   |
    +------------------------------------------------------------------------------------------------------------+
    |  2A | /Select page 0       | Bit 0 for selecting pages 0-3. Has pull-up. Leave open if paging not desired. |
    |  2B | /Select page 1       | Bit 1 for selecting pages 0-3. Has pull-up. Leave open if paging not desired. |
    |  3A | +12 V                | +12 V current is rated at 1.0 A max.                                          |
    |  3B | +12 V                | +12 V current is rated at 1.0 A max.                                          |
    |  4A | +12 V                | +12 V current is rated at 1.0 A max.                                          |
    |  4B | +12 V                | +12 V current is rated at 1.0 A max.                                          |
    |     |                      |                                                                               |
    |  6A | Decrement column addr| LOW for >= 0.1 us decrements the cursor's column address by 1.                |
    |  6B | Increment column addr| LOW for >= 0.1 us increments the cursor's column address by 1.                |
    |  7A | +12 V                | +12 V current is rated at 1.0 A max.                                          |
    |  7B | Address bit 2        | Address bit 2 of 8. +5 V = HIGH, 0 V = LOW.                                   |
    |  8A | Show cursor          | Log. '1' shows block at cursor's current position. Has pull-up.               |
    |  8B | Address bit 1        | Address bit 1 of 8. +5 V = HIGH, 0 V = LOW.                                   |
    |     |                      |                                                                               |
    | 10A | Reset row address    | HIGH for >= 0.1 us resets the cursor's row address to 0 (top).                |
    | 10B | Address bit 3        | Address bit 3 of 8. +5 V = HIGH, 0 V = LOW.                                   |
    | 11A | GND                  | Connect to GND                                                                |
    | 11B | Address bit 6        | Address bit 6 of 8. +5 V = HIGH, 0 V = LOW.                                   |
    | 12A | Reset column address | HIGH for >= 0.1 us resets the cursor's column address to 0 (left).            |
    | 12B | Address bit 5        | Address bit 5 of 8. +5 V = HIGH, 0 V = LOW.                                   |
    |     |                      |                                                                               |
    | 14A | Address bit 4        | Address bit 4 of 8. +5 V = HIGH, 0 V = LOW.                                   |
    | 14B | Address bit 7        | Address bit 7 of 8. +5 V = HIGH, 0 V = LOW.                                   |
    | 15A | GND                  | Connect to GND                                                                |
    | 15B | Increment row address| HIGH for >= 0.2 us increments the cursor to the next row.                     |
    | 16A | /Set address         | LOW for >= 0.2 us takes over the address from the address lines.              |
    | 16B | /Set character       | LOW for >= 0.1 us takes over the data lines and changes the display.          |
    |     |                      |                                                                               |
    | 18A | /Clear               | LOW for >= 20 ms clears the whole display.                                    |
    | 18B | /Data bit 6          | Data bit 6 of 7. +5 V = LOW, 0 V = HIGH (inverted!)                           |
    | 19A | GND                  | Connect to GND                                                                |
    | 19B | /Busy                | Display is busy. Don't alter data while this line is low. Output!             |
    | 20A | Address bit 8        | Address bit 8 of 8. +5 V = HIGH, 0 V = LOW.                                   |
    | 20B | Data bit 3           | Data bit 3 of 7. +5 V = LOW, 0 V = HIGH (inverted!)                           |
    |     |                      |                                                                               |
    | 22A | Data bit 1           | Data bit 1 of 7. +5 V = LOW, 0 V = HIGH (inverted!)                           |
    | 22B | Data bit 2           | Data bit 2 of 7. +5 V = LOW, 0 V = HIGH (inverted!)                           |
    | 23A | GND                  | Connect to GND                                                                |
    | 23B | Data bit 5           | Data bit 5 of 7. +5 V = LOW, 0 V = HIGH (inverted!)                           |
    | 24A | Data bit 4           | Data bit 4 of 7. +5 V = LOW, 0 V = HIGH (inverted!)                           |
    | 24B | Data bit 7           | Data bit 7 of 7. +5 V = LOW, 0 V = HIGH (inverted!)                           |
    |     |                      |                                                                               |
    | 26A | +5 V                 | +5 V current is rated 1.2 A max.                                              |
    | 26B | +5 V                 | +5 V current is rated 1.2 A max.                                              |
    | 27A | GND                  | Connect to GND                                                                |
    | 27B | +5 V                 | +5 V current is rated 1.2 A max.                                              |
    | 28A | +5 V                 | +5 V current is rated 1.2 A max.                                              |
    | 28B | +5 V                 | +5 V current is rated 1.2 A max.                                              |
    |     |                      |                                                                               |
    | 30A | GND                  | Connect to GND                                                                |
    | 30B | GND for +12 V        | Connect to GND                                                                |
    | 31A | GND for +12 V        | Connect to GND                                                                |
    | 31B | GND for +12 V        | Connect to GND                                                                |
    | 32A | GND for +12 V        | Connect to GND                                                                |
    | 32B | GND for +12 V        | Connect to GND                                                                |                 
    +-----+----------------------+-------------------------------------------------------------------------------+
      
*/

#ifndef MS6205_H
#define MS6205_H

#include "Arduino.h"

#define NUMBER_OF_COLUMNS          16   // Number of columns in each row
#define NUMBER_OF_ROWS             10   // Number of rows in each column
#define NUMBER_OF_CHARACTERS      160   // Number of characters in all columns and rows (= columns * rows)
#define NUMBER_OF_PAGES             4   // Number of pages supported

#define BIG_DIGIT_WIDTH             3   // [columns] A "big" digit is 3 characters wide
#define BIG_DIGIT_HEIGHT            5   // [rows] A "big" digit is 5 characters tall
#define BIG_SPACE_WIDTH             1   // [columns] A "big" space between two "big" digits

class MS6205
{
  public:
  
    //--------------------------------------------------------------
    /// \brief Class constructor 
    ///
    /// Creates MS6205 object and initializes pins & variables.
    ///
    /// \param[in]  shiftRegisterLatchPin  CPU pin connected to 74HC595 shift register "latch" pin 12
    /// \param[in]  shiftRegisterClockPin  CPU pin connected to 74HC595 shift register "clock" pin 11
    /// \param[in]  shiftRegisterDataPin   CPU pin connected to 74HC595 shift register "data" pin 14
    /// \param[in]  setCursorPin           CPU pin connected to MS6205 display "set cursor" pin 16A
    /// \param[in]  setCharacterPin        CPU pin connected to MS6205 display "set character" pin 16B
    /// \param[in]  clearPin               CPU pin connected to MS6205 display "clear" pin 18A
    //--------------------------------------------------------------
    MS6205(int shiftRegisterLatchPin, int shiftRegisterClockPin, int shiftRegisterDataPin, int setCursorPin, int setCharacterPin, int clearPin);
        
    //--------------------------------------------------------------
    /// \brief Set cursor
    ///
    /// Sets the position of the display's cursor. Set the location at which           \
    /// subsequent text written to the MS6205 will be displayed. 
    ///
    /// \param[in]  column  Display column to write to, 0 (left) to 15 (right)
    /// \param[in]  row     Display row to write to, 0 (upper) to 9 (lower)
    //--------------------------------------------------------------
    void setCursor(int column, int row);
    
    //--------------------------------------------------------------
    /// \brief Increment cursor
    ///
    /// Adds n positions to the display's cursor.                                     \ 
    /// Wraps around to the next line and to the very beginning (upper-left corner) if text is too long.
    ///
    /// \param[in]  n  Positions to add to the current cursor position
    //--------------------------------------------------------------
    void addCursor(int n);
    
    //--------------------------------------------------------------
    /// \brief Write text to display
    ///
    /// This method writes a string to the display at the current cursor position.     \
    /// Position is automatically incremented. Wraps around to the next line and       \
    /// to the very beginning (upper-left corner) if text is too long.                 \
    /// This method is the easiest way to write to the display.
    ///
    /// \param[in]  String  String to display
    //--------------------------------------------------------------
    void write(String string);    
    
    //--------------------------------------------------------------
    /// \brief Write single character to the display
    ///
    /// This method writes a single character to the display at the current cursor position.                                                          \
    /// Position is *not* incremented or wrapped!                                                                                                     \
    /// This method gives a more low-level access with more flexibility.                                                                              \
    ///
    /// \param[in]  character  Character to display
    //--------------------------------------------------------------
    void writeCharacter(char character);
    
    //--------------------------------------------------------------
    /// \brief Write single character to the display at a given position
    ///
    /// This method writes a single character to the display at a given position.                                                                     \
    /// Position is *not* incremented or wrapped!                                                                                                     \
    /// This method gives a more low-level access with more flexibility.                                                                              \
    ///
    /// \param[in]  column     Display column to write to, 0 (left) to 15 (right)
    /// \param[in]  row        Display row to write to, 0 (upper) to 9 (lower)
    /// \param[in]  character  Character to display
    //--------------------------------------------------------------
    void writeCharacter(int column, int row, char character);
    
    //--------------------------------------------------------------
    /// \brief Write single "block" character to the display
    ///
    /// This method writes a single "block" character to the display at the given position.                                                           \
    /// Position is *not* incremented or wrapped!                                                                                                     \
    /// This method gives a more low-level access with more flexibility.                                                                              \
    /// The "block" character is mostly useful for doing ASCII art.
    ///
    /// \param[in]  column  Display column to write to, 0 (left) to 15 (right)
    /// \param[in]  row     Display row to write to, 0 (upper) to 9 (lower)
    //--------------------------------------------------------------
    void writeBlock(int column, int row);
    
    //--------------------------------------------------------------
   /// \brief Write a "big" number to the display
   ///
   /// Each "big" digit is shown by a 3x5 normal character matrix of blocks and spaces.               \
   /// Wrapping around lines is not supported.
   ///
   /// \param[in]  column  Display column to start writing to, leftmost edge of a total of 3
   /// \param[in]  row     Display row to start writing to, upper row of a total of 5
   /// \param[in]  number  Number to draw in 3x5 character matrix
   //--------------------------------------------------------------
   void writeBigNumber(int column, int row, int number);
    
    //--------------------------------------------------------------
    /// \brief Write a single "big" character to the display. Only digits are currently supported.
    ///
    /// A "big" digit is shown by a 3x5 normal character matrix of blocks and spaces.                   \
    /// Wrapping around lines is not supported.
    ///
    /// \param[in]  column  Display column to start writing to, leftmost edge of a total of 3
    /// \param[in]  row     Display row to start writing to, upper row of a total of 5
    /// \param[in]  digit   Digit to draw in 3x5 character matrix
    //--------------------------------------------------------------
    void writeBigDigit(int column, int row, int digit);
    
    //--------------------------------------------------------------
    /// \brief Clear the display
    ///
    /// Clears all text from the display.
    //--------------------------------------------------------------
    void clear(void);
    
    //--------------------------------------------------------------
    /// \brief Optional: Initialize cursor functionality 
    ///
    /// The MS6205 can show a black box at the cursor's current position.
    /// Initialize pins & variables needed for showing and hiding the cursor.                                   
    ///                                                                                                           
    /// \param[in]  showCursorPin  CPU pin connected to MS6205 display "show cursor" pin 8A                    
    //--------------------------------------------------------------
    void beginCursor(int showCursorPin);
    
    //--------------------------------------------------------------
    /// \brief Optional: Show cursor on display 
    ///
    /// Shows a black box at the cursor position.
    /// Call beginCursor() before use.
    //--------------------------------------------------------------
    void showCursor(void);
    
    //--------------------------------------------------------------
    /// \brief Optional: Hidecursor on display 
    ///
    /// Hides cursor position.
    /// Call beginCursor() before use.
    //--------------------------------------------------------------
    void hideCursor(void);
    
    //--------------------------------------------------------------
    /// \brief Optional: Initialize paging functionality 
    ///
    /// Initialize pins & variables needed for paging and select page 0 (first).                                   
    ///                                                                                                           
    /// \param[in]  selectPage0Pin  CPU pin connected to MS6205 display "select page 0" pin 2A                    
    /// \param[in]  selectPage1Pin  CPU pin connected to MS6205 display "select page 1" pin 2B
    //--------------------------------------------------------------
    void beginPaging(int selectPage0Pin, int selectPage1Pin);
    
    //--------------------------------------------------------------
    /// \brief Optional: Set visible page
    ///
    /// Call beginPaging() before use.
    ///
    /// \param[in]  page  [0-3] Page to display 
    //--------------------------------------------------------------
    void showPage(int page);
    
    
  private:
    int _shiftRegisterLatchPin;     // CPU pin connected to 74HC595 pin 12
    int _shiftRegisterClockPin;     // CPU pin connected to 74HC595 pin 11
    int _shiftRegisterDataPin;      // CPU pin connected to 74HC595 pin 14
    int _setCursorPin;              // CPU pin connected to MS6205 pin 16A
    int _setCharacterPin;           // CPU pin connected to MS6205 pin 16B
    int _clearPin;                  // CPU pin connected to MS6205 pin 18A
    int _selectPage0Pin;            // CPU pin connected to MS6205 pin 2A
    int _selectPage1Pin;            // CPU pin connected to MS6205 pin 2B
    int _showCursorPin;             // CPU pin connected to MS6205 pin 8A
    bool _pagingEnabled;
    bool _cursorEnabled;
    int _address;
    
    void writeToShiftRegister(char data);
    void writeAddress(int address);
};

#include <MS6205_scroll.h>

#endif // MS6205_H