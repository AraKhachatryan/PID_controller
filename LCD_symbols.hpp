#ifndef LCD_SYMBOLS_HPP
#define LCD_SYMBOLS_HPP

// Instanciated lcd object from Hot_Air_Sterilizer.ino file
extern LiquidCrystal_I2C lcd;

namespace lcd_symbols
{
	byte setChar_1[8] =  { 0b00000, 0b00010, 0b00110, 0b01110, 0b00110, 0b00010, 0b00000, 0b00000};
	byte setChar_2[8] =  { 0b00000, 0b00001, 0b00010, 0b00100, 0b00010, 0b00001, 0b00000, 0b00000};
	byte heatChar_1[8] = { 0b01000, 0b10101, 0b00010, 0b00000, 0b01000, 0b10101, 0b00010, 0b00000};
	byte heatChar_2[8] = { 0b00000, 0b01000, 0b10101, 0b00010, 0b00000, 0b01000, 0b10101, 0b00010};
	byte ventChar_1[8] = { 0b00000, 0b00000, 0b01100, 0b00101, 0b11011, 0b10100, 0b00110, 0b00000};
	byte ventChar_2[8] = { 0b00000, 0b00000, 0b00010, 0b11010, 0b00100, 0b01011, 0b01000, 0b00000};
	  
	void create()
	{
		lcd.createChar(0, setChar_1);
		lcd.createChar(1, setChar_2);
		lcd.createChar(2, heatChar_1);
		lcd.createChar(3, heatChar_2);
		lcd.createChar(4, ventChar_1);
		lcd.createChar(5, ventChar_2);
	}
	
	inline void set(const byte &col, const byte &row)
	{
		int heatSymbolTimeDelta = millis() % 2000;
		lcd.setCursor(col, row);  
		if (heatSymbolTimeDelta <= 1000) {
			lcd.write(0);
		} else if (heatSymbolTimeDelta > 1000) {
			lcd.write(1);
		}
	}
	
	inline void heat(const byte &col, const byte &row)
	{
		int heatSymbolTimeDelta = millis() % 2000; 
		lcd.setCursor(col, row);    
		if (heatSymbolTimeDelta <= 1000) {
			lcd.write(2);
		} else if (heatSymbolTimeDelta > 1000) {
			lcd.write(3);
		}
	}
	
	inline void vent(const byte &col, const byte &row)
	{
		int heatSymbolTimeDelta = millis() % 2000;
		lcd.setCursor(col, row);  
		if (heatSymbolTimeDelta <= 1000) {
			lcd.write(4);
		} else if (heatSymbolTimeDelta > 1000) {
			lcd.write(5);
		}
	}
	
}

#endif // LCD_SYMBOLS_HPP
