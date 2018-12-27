/*
  MS6205_scroll.h - Library for scrolling text on a MS6205 vintage soviet character display.
  
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

#ifndef MS6205_SCROLL_H
#define MS6205_SCROLL_H

#include "Arduino.h"
#include "MS6205.h"

class scrollText  
{
  public:
  
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
    scrollText(int startColumn, int startRow, int endColumn, int endRow, int delayTime, String text, MS6205 *pDisplay);
    
    //--------------------------------------------------------------
    /// \brief Periodic update 
    ///
    /// Call in loop() method
    //--------------------------------------------------------------
    void update(void);
    
    //--------------------------------------------------------------
    /// \brief Set text 
    ///
    /// Change text to display in area
    ///
    /// \param[in]  text        String to display
    //--------------------------------------------------------------
    void setText(String text);
  
  private:
    unsigned long _millis;        // [ms] State of global millis() timer
    int _delayTime;               // [ms] Delay between scrolling to the next state
    int _areaStart;               // [position] Address of first character to show scrolling at
    int _areaLength;              // [characters] Length of area to display scrolling at
    String _text;                 // Text to display
    int _textLength;              // Text length
    int _state;                   // State of scrolling
    bool _enabled;                // Enabled/disabled scrolling 
    MS6205 * _pDisplay;           // Pointer to display to show on
    
    void clearArea(void);  
};

#endif // MS6205_SCROLL_H