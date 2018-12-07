/*
  MS6205.h - Library for controlling a MS6205 vintage soviet character display.
  
  Copyright by Christian Holzapfel, December 2, 2018.
  
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
*/

#include "Arduino.h"
#include "MS6205.h"

#define CONTROL_LINE_HOLD_TIME_US   1    // [us] Hold time of control lines between level changes. 0.2 us according to MS6205 datasheet, but we play safe here
#define CLEAR_ALL_HOLD_TIME_US     20    // [ms] Time to hold "Clear All" control line to clear the display, according to MS6205 datasheet

char const firstValidChar       =  32;   // Decimal code of first available ASCII character (32d = space in this case)
char const lastValidChar        = 127;   // Decimal code of last available ASCII character (127d = a fully black box in case of MS6205)
char const blackBoxChar         = 0x00;  // Decimal code of a fully black box in case of MS6205 (not ASCII compliant)

char const bigDigits[10][BIG_DIGIT_HEIGHT][BIG_DIGIT_WIDTH] =   // Matrix definitions for "big" numbers. A 1 indicates a drawn block, a 0 will result in a space (empty field).
                {{{1, 1, 1},    // 0
                  {1, 0, 1},
                  {1, 0, 1},
                  {1, 0, 1},
                  {1, 1, 1}},

                 {{0, 1, 0},    // 1
                  {0, 1, 0},
                  {0, 1, 0},
                  {0, 1, 0},
                  {0, 1, 0}},

                 {{1, 1, 1},    // 2
                  {0, 0, 1},
                  {1, 1, 1},
                  {1, 0, 0},
                  {1, 1, 1}},
                  
                 {{1, 1, 1},    // 3
                  {0, 0, 1},
                  {0, 1, 1},
                  {0, 0, 1},
                  {1, 1, 1}},
                  
                 {{1, 0, 1},    // 4
                  {1, 0, 1},
                  {1, 1, 1},
                  {0, 0, 1},
                  {0, 0, 1}},
                  
                 {{1, 1, 1},    // 5
                  {1, 0, 0},
                  {1, 1, 1},
                  {0, 0, 1},
                  {1, 1, 1}},
                  
                 {{1, 1, 1},    // 6
                  {1, 0, 0},
                  {1, 1, 1},
                  {1, 0, 1},
                  {1, 1, 1}},
                  
                 {{1, 1, 1},    // 7
                  {0, 0, 1},
                  {0, 0, 1},
                  {0, 0, 1},
                  {0, 0, 1}},
                  
                 {{1, 1, 1},    // 8
                  {1, 0, 1},
                  {1, 1, 1},
                  {1, 0, 1},
                  {1, 1, 1}},
                  
                 {{1, 1, 1},    // 9
                  {1, 0, 1},
                  {1, 1, 1},
                  {0, 0, 1},
                  {0, 0, 1}}};       

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
/// \param[in]  clearAllPin            CPU pin connected to MS6205 display "clear" pin 18A
//--------------------------------------------------------------
MS6205::MS6205(int shiftRegisterLatchPin, int shiftRegisterClockPin, int shiftRegisterDataPin, int setCursorPin, int setCharacterPin, int clearAllPin)
{
  _shiftRegisterLatchPin = shiftRegisterLatchPin;
  _shiftRegisterClockPin = shiftRegisterClockPin;
  _shiftRegisterDataPin = shiftRegisterDataPin;
  _setCursorPin = setCursorPin;
  _setCharacterPin = setCharacterPin;
  _clearAllPin = clearAllPin;
  
  // --- Set shift register pins to output mode ---
  pinMode(_shiftRegisterLatchPin, OUTPUT);
  pinMode(_shiftRegisterClockPin, OUTPUT);
  pinMode(_shiftRegisterDataPin, OUTPUT);
  
  // --- Set display pins to output mode ---
  pinMode(_setCursorPin, OUTPUT);
  pinMode(_setCharacterPin, OUTPUT);
  pinMode(_clearAllPin, OUTPUT);
  
  _showCursorPin = 0;
  _pagingEnabled = false;
  _cursorEnabled = false;
  
  // --- Initialize display ---
  showPage(0);
  setCursor(0, 0);
  clear();  
} // MS6205()

//--------------------------------------------------------------
/// \brief Set cursor
///
/// Sets the position of the display's cursor. Set the location at which           \
/// subsequent text written to the MS6205 will be displayed. 
///
/// \param[in]  column  Display column to write to, 0 (left) to 15 (right)
/// \param[in]  row     Display row to write to, 0 (upper) to 9 (lower)
//--------------------------------------------------------------
void MS6205::setCursor(int column, int row)
{
  // On MS6205 address bus, bits A1-A4 address the column (0-15) and bits A5-A8 address the row (0-9).
  // So the address goes from 0 (left-upper corner) to 159 (lower right corner).

  // --- Calculate address byte from row and column positions ---
  constrain(row, 0, NUMBER_OF_ROWS - 1);                          // Limit range of rows
  constrain(column, 0, NUMBER_OF_COLUMNS - 1);                    // Limit range of columns

  unsigned char address = (column & 0x0F) | (uint8_t)(row << 4);  // Assemble MS6205 address byte
  
  // --- Write position's address to display ---
  writeAddress(address);
} // setCursor()

//--------------------------------------------------------------
/// \brief Increment cursor
///
/// Adds n positions to the display's cursor.                                     \ 
/// Wraps around to the next line and to the very beginning (upper-left corner) if text is too long.
///
/// \param[in]  n  Positions to add to the current cursor position
//--------------------------------------------------------------
void MS6205::addCursor(int n)
{
  _address += n;                                                // Increment position across columns and rows
  if (_address >= NUMBER_OF_CHARACTERS)                         // If display is full:
  {
    _address -= NUMBER_OF_CHARACTERS;                           // Wrap around to the start
  }
  writeAddress(_address);                                       // Set next address
} // addCursor()

//--------------------------------------------------------------
/// \brief Write address to display
///
/// Writes a given address (position) to the display.
///
/// \param[in]  address   0 (left-upper corner) to 159 (lower right corner)
//--------------------------------------------------------------
void MS6205::writeAddress(int address)
{
  // --- Take local copy ---
  _address = address;

  // --- Output address byte through 74HC595 shift register ---
  writeToShiftRegister(address);

  // --- Toggle MS6205's "Set Address" line ---
  digitalWrite(_setCursorPin, LOW);                               // Pull "Set Address" control line 16A low
  delayMicroseconds(CONTROL_LINE_HOLD_TIME_US);                   // Hold for proper delay

  digitalWrite(_setCursorPin, HIGH);                              // Pull "Set Address" control line 16A high to apply address from address lines
  delayMicroseconds(CONTROL_LINE_HOLD_TIME_US);                   // Hold for proper delay
} // writeAddress()

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
void MS6205::write(String string)
{
  for (int i = 0; i < string.length(); i++)                       // For each character of the string:
  {   
    char character = string.charAt(i);
    character = toUpperCase(character);                           // MS6205 only supports uppercase latin letters
    writeCharacter(character);                                    // Write character to display

    _address++;                                                   // Increment position across columns and rows
    if (_address >= NUMBER_OF_CHARACTERS)                         // If display is full:
    {
      _address = 0;                                               // Wrap around to the start
    }
    writeAddress(_address);                                       // Set next address
  } // for()
} // write()

//--------------------------------------------------------------
/// \brief Write single character to the display
///
/// This method writes a single character to the display at the current cursor position.                                                          \
/// Position is *not* incremented or wrapped!                                                                                                     \
/// This method gives a more low-level access with more flexibility.                                                                              \
///
/// \param[in]  character  Character to display
//--------------------------------------------------------------
void MS6205::writeCharacter(char character)
{
  // --- Prepare data byte ---
  constrain(character, firstValidChar, lastValidChar - 1);      // Limit characters to supported range
 
  character = (unsigned char)~character;                        // Invert bits because the data bus is inverted  
  character = (unsigned char)character & 0x7F;                  // Keep only the lower 7 bits because the data bus is only 7 bits wide

  // --- Output data byte through 74HC595 shift register ---  
  writeToShiftRegister(character);

  // --- Toggle MS6205's "Set Character" line ---
  digitalWrite(_setCharacterPin, LOW);                          // Pull "Set Character" control line 16B low
  delayMicroseconds(CONTROL_LINE_HOLD_TIME_US);                 // Hold for proper delay
   
  digitalWrite(_setCharacterPin, HIGH);                         // Pull "Set Character" control line 16B high to apply character from data lines
  delayMicroseconds(CONTROL_LINE_HOLD_TIME_US);                 // Hold for proper delay
} // writeCharacter()

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
void MS6205::writeCharacter(int column, int row, char character)
{
  setCursor(column, row);                                       // Set cursor to given position
  
  // --- Prepare data byte ---
  constrain(character, firstValidChar, lastValidChar - 1);      // Limit characters to supported range
 
  character = (unsigned char)~character;                        // Invert bits because the data bus is inverted  
  character = (unsigned char)character & 0x7F;                  // Keep only the lower 7 bits because the data bus is only 7 bits wide

  // --- Output data byte through 74HC595 shift register ---  
  writeToShiftRegister(character);

  // --- Toggle MS6205's "Set Character" line ---
  digitalWrite(_setCharacterPin, LOW);                          // Pull "Set Character" control line 16B low
  delayMicroseconds(CONTROL_LINE_HOLD_TIME_US);                 // Hold for proper delay
   
  digitalWrite(_setCharacterPin, HIGH);                         // Pull "Set Character" control line 16B high to apply character from data lines
  delayMicroseconds(CONTROL_LINE_HOLD_TIME_US);                 // Hold for proper delay
} // writeCharacter()

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
void MS6205::writeBlock(int column, int row)
{
  setCursor(column, row);                                       // Set cursor to given position
  
  char character = blackBoxChar;                                // Send code that defines a fully black box
  character = (unsigned char)~character;                        // Invert bits because the data bus is inverted  
  character = (unsigned char)character & 0x7F;                  // Keep only the lower 7 bits because the data bus is only 7 bits wide
   
  writeToShiftRegister(blackBoxChar);

  digitalWrite(_setCharacterPin, LOW);                          // Pull "Set Character" control line 16B low
  delayMicroseconds(CONTROL_LINE_HOLD_TIME_US);                 // Hold for proper delay

  digitalWrite(_setCharacterPin, HIGH);                         // Pull "Set Character" control line 16B high to apply character from data lines
  delayMicroseconds(CONTROL_LINE_HOLD_TIME_US);                 // Hold for proper delay
} // writeBlock()

//--------------------------------------------------------------
/// \brief Clear the display
///
/// Clears all text from the display.
//--------------------------------------------------------------
void MS6205::clear(void)
{
  digitalWrite(_clearAllPin, LOW);                              // Pull "Clear All" control line low to clear everything
  delay(CLEAR_ALL_HOLD_TIME_US);                                // Hold for proper delay
  digitalWrite(_clearAllPin, HIGH);                             // Pull "Clear All" control line high
} // clear()

//--------------------------------------------------------------
/// \brief Initialize optional cursor functionality 
///
/// Initialize pins & variables needed for showing and hiding the cursor.                                   
///                                                                                                           
/// \param[in]  showCursorPin  CPU pin connected to MS6205 display "show cursor" pin 8A                    
//--------------------------------------------------------------
void MS6205::beginCursor(int showCursorPin)
{
  _showCursorPin = showCursorPin;
  
  pinMode(_showCursorPin, OUTPUT);
  
  showCursor();
  
  _cursorEnabled = true;
} // beginCursor()

//--------------------------------------------------------------
/// \brief Show optional cursor on display 
///
/// Shows a black box at the cursor position.
//--------------------------------------------------------------
void MS6205::showCursor(void)
{
  if (_cursorEnabled == true)
  {
    digitalWrite(_showCursorPin, LOW);     // A low signal hides the cursor at set position
  }
} // showCursor()

//--------------------------------------------------------------
/// \brief Hide optional cursor on display 
///
/// Hides cursor position.
//--------------------------------------------------------------
void MS6205::hideCursor(void)
{
  if (_cursorEnabled == true)
  {
    digitalWrite(_showCursorPin, HIGH);    // A high signal shows a black box as cursor at set position 
  }
} // hideCursor()

//--------------------------------------------------------------
/// \brief Initialize optional paging functionality 
///
/// Initialize pins & variables needed for paging and select page 0 (first).                                  \
///                                                                                                           
/// \param[in]  selectPage0Pin  CPU pin connected to MS6205 display "select page 0" pin 2A                    
/// \param[in]  selectPage1Pin  CPU pin connected to MS6205 display "select page 1" pin 2B
//--------------------------------------------------------------
void MS6205::beginPaging(int selectPage0Pin, int selectPage1Pin)
{
  _selectPage0Pin = selectPage0Pin;
  _selectPage1Pin = selectPage1Pin;
  
  pinMode(_selectPage0Pin, OUTPUT);
  pinMode(_selectPage1Pin, OUTPUT);
  
  showPage(0);
  
  _pagingEnabled = true;  
} // beginPaging()

//--------------------------------------------------------------
/// \brief Set visible page
///
/// \param[in]  page  [0-3] Page to display 
//--------------------------------------------------------------
void MS6205::showPage(int page)
{
  if (_pagingEnabled == true)
  {
    constrain(page, 0, NUMBER_OF_PAGES - 1);                      // Limit pages from 0-3
    page = ~page;                                                 // Page select lines are inverted
    digitalWrite(_selectPage0Pin, page & 0x01);                   // Set control line 2A
    digitalWrite(_selectPage1Pin, ((page & 0x02) >> 1));          // Set control line 2B
    delayMicroseconds(CONTROL_LINE_HOLD_TIME_US);                 // Hold for proper delay 
  } 
} // selectPage()

//--------------------------------------------------------------
/// \brief Write data through shift register to display
///
/// Writes a single character to the 74HC595 shift register and further to the display.                                                 \
/// This can be either character or address data.
///
/// \param[in]  data  Data byte to send to display
//--------------------------------------------------------------
void MS6205::writeToShiftRegister(char data)
{
  // --- Output byte through 74HC595 shift register ---  
  digitalWrite(_shiftRegisterLatchPin, LOW);                                 // Pull the shift register's latch pin low
  shiftOut(_shiftRegisterDataPin, _shiftRegisterClockPin, MSBFIRST, data);   // Shift out the bits to mixed data/address bus
  digitalWrite(_shiftRegisterLatchPin, HIGH);                                // Pull the shift register's latch pin high to switch its output
} // writeToShiftRegister()

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
void MS6205::writeBigNumber(int column, int row, int number)
{
  int temp;
  int factor;

  // --- Count the digits ---
  temp = number;
  factor = 1;
  while (temp > 0)
  {
    factor = factor * 10;
    temp = temp / 10;
  }

  // --- Print individual digits ---
  int digitsPrinted = 0;
  while (factor > 1)
  {
    int digit, col;
    factor = factor / 10;
    digit = number / factor;
    col = column + digitsPrinted * (BIG_DIGIT_WIDTH + BIG_SPACE_WIDTH);
    writeBigDigit(col, row, digit);
    number = number % factor;
    digitsPrinted++;
  }
} // writeBigNumber()

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
void MS6205::writeBigDigit(int column, int row, int digit)
{  
  for (int r = 0; r < BIG_DIGIT_HEIGHT; r++)                        // Go through all rows of the bigDigit..
  {
    for (int c = 0; c < BIG_DIGIT_WIDTH; c++)                       // ..and go through all columns of each row:
    {
      if (  (column + c < NUMBER_OF_COLUMNS)                        // If still inside character matrix for columns..
          &&(row + r < NUMBER_OF_ROWS))                             // ..and rows:
      {
        if (bigDigits[digit][r][c] == 1)                            // If painting the bigDigit requires a field "set":
        {
          writeBlock(column + c, row + r);                          // Write a black block at that position
        }
        else                                                        // Else if painting the bigDigit requires a field "cleared":
        {
          writeCharacter(column + c, row + r, ' ');                 // Write a space (empty field) at that postion
        }
      }  
      else                                                          // Overflow in columns or rows
      {        
        // => Don't write there!
        // This could result in an overflow to the next row. 
      }
    }
  }
} // writeBigDigit()    


