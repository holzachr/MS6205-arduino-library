# MS6205-arduino-library
An Arduino library for accessing the functions of an "Elektronika MS6205" vintage soviet multi-line character display with parallel data and address buses.

## ABOUT THE DISPLAY
The "Elektronika MS6205" display is a multi-line cold-cathode-tube display with internal high-voltage generation.
It has a visible area of 10 x 10 cm, and supports 160 characters in 10 rows with 16 characters each.
Parallel data and address busses are used to set the cursor position and the character to display there.
   
It runs of separate +5 V and +12 V supplies.
   
This library assumes that both the address and data bus are connected in parallel, where the data is
provided by the Arduino through a 74HC595 shift register.  
   
   
## SUPPORTED CHARACTERS
ASCII characters 32..127d are supported.                                                                                                     
Character codes 96-126 are assigned to cyrillic characters instead of `, lower latin chars and { | } ~.                                          
Code 127 defines a fully black box.                                                                                                          
                                                                                                                                                
```
Code       32    33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63                                
Character  Space !  "  #  ¤  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?                                 
                                                                                                                                                
Code       64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95                                   
Character  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _                                    
                                                                                                                                                
Code       96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127       
Character  Ю  А  Б  Ц  Д   Е   Ф   Г   Х   И   Й   К   Л   М   Н   О   П   Я   Р   С   Т   У   Ж   В   Ь   Ы   З   Ш   Э   Щ   Ч   █
```   
   
## CURSOR (optional)
MS6205 can show a black box at the cursor's current position.
If control line 8A is low, the cursor is hidden.
If control line 8A is high, a black box is shown.  
  
  
## PAGING (optional)
MS6205 supports up to 4 pages.                                                                            
Writing is always done to the current (visible) page. You cannot fill pages invisibly in background.      
After filling, the pages remain defined, until redefined, and can be selected randomly.                   
Pages are selected by inverted control lines 2A and 2B, which have pull-ups.                              
                                                                                                              
       2A  |  2B  | Page                                                                                      
     ------|------|------                                                                                     
      HIGH | HIGH | 0                                                                                         
      LOW  | HIGH | 1                                                                                         
      HIGH | LOW  | 2                                                                                         
      LOW  | LOW  | 3                                                                                         
                                                                                                              
1. Select a first page to write to                                                                  
2. Optionally clear it                                                                                    
3. Write to page                                                                                          
4. Select next page. Visible output changes, but page from steps 1-3 remains in MS6205 memory               
5. Optionally clear it                                                                                    
6. Write to next page                                                                                     
7. Now you can toggle between the two defined pages                                                       
8. Optionally repeat above steps to define more pages    
    
    
## SOCKET PIN ORDER
  
          +-----------------------------------------------------------------------------------------------+
     B    |32|31|30|  |28|27|26|  |24|23|22|  |20|19|18|  |16|15|14|  |12|11|10|  | 8| 7| 6|  | 4| 3| 2|  |    B
       +-----------------------------------------------------------------------------------------------------+
     A |  |32|31|30|  |28|27|26|  |24|23|22|  |20|19|18|  |16|15|14|  |12|11|10|  | 8| 7| 6|  | 4| 3| 2|  |  | A
       +-----------------------------------------------------------------------------------------------------+     
                               View from the outside into the device pin header
  
      
    
## ELECTRICAL CONNECTION
            
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
        
    
## SIGNAL DESCRIPTION
  
A logical "0" (LOW) level is registered on address, data and other control lines at a voltage between -0.5 and 0.4 V.
A logical "1" (HIGH) level is registered on address, data and other control lines at a voltage between 2.4 and 5.5 V.
For this reason, control from a +3.3 V ESP8266 module works fine, even with the 74HC595 supplied with only +3.3 V.
    
The bits on data bus are inverted. So to write an 'F', send hex 0x39 instead of usually 0x46.
    
The lower bits 1-4 on address bus indicate the column (0-15), the upper bits 5-8 address the row (0-9).

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
