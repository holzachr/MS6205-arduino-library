/*
  MS6205.h - Library for scrolling text on a MS6205 vintage soviet character display.
  
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
*/

#include "Arduino.h"
#include "MS6205.h"
#include "MS6205_scroll.h"


//--------------------------------------------------------------
/// \brief Class constructor 
///
/// Creates scrollable text object
///
/// \param[in]  startColumn Column of first character to show scrolling at
/// \param[in]  startRow    Row of first character to show scrolling at
/// \param[in]  endColumn   Column of last character to show scrolling at
/// \param[in]  endRow      Row of last character to show scrolling at
/// \param[in]  delayTime   [ms] Delay between scrolling changes
/// \param[in]  text        String to display
/// \param[in]  pDisplay    Display to show scrolling on
//--------------------------------------------------------------
scrollText::scrollText(int startColumn, int startRow, int endColumn, int endRow, int delayTime, String text, MS6205 *pDisplay)    //cho parameter checks
{
  _text = text;                                          // Text to display
  _textLength = text.length();                           // Text length
  _delayTime = delayTime;                                // [ms] Delay between scrolling to the next state
  
  if (startColumn >= (NUMBER_OF_COLUMNS - 1))
    startColumn = NUMBER_OF_COLUMNS - 1;
  
  if (startRow >= (NUMBER_OF_ROWS - 1))
    startRow = NUMBER_OF_ROWS - 1;
  
  if (endColumn >= (NUMBER_OF_COLUMNS - 1))
    endColumn = NUMBER_OF_COLUMNS - 1;
  
  if (endRow >= (NUMBER_OF_ROWS - 1))
    endRow = NUMBER_OF_ROWS - 1;  
  
  _areaStart = (startColumn & 0x0F) | (startRow << 4);   // [position] Address of first character to show scrolling at
  int areaEnd = (endColumn & 0x0F) | (endRow << 4);      // [position] Address of last character to show scrolling at
  
  if (areaEnd > _areaStart)                              // If valid area defined:
  {
    _areaLength = (areaEnd - _areaStart + 1);            // [characters] Length of area to display scrolling at
  }
  else                                                   // Invalid area defined:
  {
    _areaLength = 1;                                     // Show only one character at start position, to indicate something's wrong
  }
  
  _state = _areaLength + _textLength;                    // State of scrolling
  _enabled = true;                                       // Enabled/disabled scrolling
  _pDisplay = pDisplay;                                  // Pointer to display to show scrolling on
} // scrollText()

//--------------------------------------------------------------
/// \brief Periodic update 
///
/// Call in loop() method
//--------------------------------------------------------------
void scrollText::update(void)
{
  if (  (_enabled == true)
      &&(_pDisplay != NULL)
      &&(millis() > (_millis + _delayTime)))
  {  
    // --- Clear scrolling area ---
    clearArea();

    // --- Scroll text ---
    if (_state >= _textLength)
    {
      // Scroll inside available area
      int row = (_areaStart + _state - _textLength) >> 4;
      int column = (_areaStart + _state - _textLength) & 0x0F;
      _pDisplay->setCursor(column, row);      
      _pDisplay->write(_text.substring(0, _areaLength + _textLength - _state));      
    }
    else
    {
      // Scroll out to the left      
      int row = _areaStart >> 4;
      int column = _areaStart & 0x0F;
      _pDisplay->setCursor(column, row);  
      _pDisplay->write(_text.substring(_textLength - _state, _areaLength + _textLength - _state));
    }
  
    // --- Count state ---
    _state--;
    if (_state < 0)
    {
      _state = _areaLength + _textLength;
    }


    _millis = millis();
  }
} // update()

//--------------------------------------------------------------
/// \brief Set text 
///
/// Change text to display in area
///
/// \param[in]  text        String to display
//--------------------------------------------------------------
void scrollText::setText(String text)
{
  _enabled = false;                        // Disable scrolling
  _text = text;                            // Text to display
  _textLength = text.length();             // Text length
  _state = _areaLength + _textLength;      // State of scrolling
  clearArea();                             // Clear display area  
  _enabled = true;                         // Enable scrolling
} // setText()

//--------------------------------------------------------------
/// \brief Clear scrolling area 
///
/// Clears defined area
//--------------------------------------------------------------
void scrollText::clearArea(void)
{
  int row = _areaStart >> 4;
  int column = _areaStart & 0x0F;
  String spaces = "";
  for (int i = 0; i < _areaLength; i++)
  {
    spaces += " "; 
  }
  
  if (_pDisplay != NULL)
  {
    _pDisplay->setCursor(column, row);
    _pDisplay->write(spaces);
  }
} // clearArea()



