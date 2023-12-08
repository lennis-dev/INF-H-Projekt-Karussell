/** Verwendet SoftwareI2C von:
 * mbed Library to use a software master i2c interface on any GPIO pins
 * Copyright (c) 2012 Christopher Pepper
 * Released under the MIT License: http://mbed.org/license/mit
 *
  * Example:
 * @code
 * #include "LCD.h"

 * lcd myLCD;
 * int main() {  
 *     int x=0;
 *     myLCD.clear();
 *     myLCD.cursorpos(0);
 *     myLCD.printf("x=%d",x);
 *     while(1)  {
 *     }
 * }
 * @endcode
 */

//Anpassungen in SoftwareI2C.cpp: _frequency_delay = 3;

#include "mbed.h"
#include "SoftwareI2C.h" 
   
class lcd
{   
    private:
    uint8_t Adresse;//=0x3F;
    //DigitalOut *ok;
    //DigitalOut *nok;
    //PortOut *po;
    //DigitalIn *t;
    SoftwareI2C *i2c;
    uint8_t wert;
    public:
    /** Create LCD Instance
    */
    lcd(void);
    
    /** löscht das Display
    */
    void clear(void);

    /** Positioniert den Cursor
    * @param pos 0.. 0xF 1. Zeile, 0x40..0x4F 2. Zeile
    */
    void cursorpos(uint8_t pos);

    /** Print formattet
    * @param *format Formatstring
    * @param ... Variablenliste
    */
    int printf(const char *format, ...);
    
      /** Locate to a screen column and row
   *
   * @param column  The horizontal position from the left, indexed from 0
   * @param row     The vertical position from the top, indexed from 0
   */
    void locate(int column, int row);

    /** Clear the screen and locate to 0,0 */
    void cls();
    /** Write a character to the LCD
    *
    * @param c The character to write to the display
    */
    void putc(int c);


private:
    void warte(void);
    void sendeByte(char b,uint8_t rw, uint8_t rs );
    void sendeNippel(char b,uint8_t rw, uint8_t rs );
    void init(void);

};
