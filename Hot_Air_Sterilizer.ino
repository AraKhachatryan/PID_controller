#include <Wire.h> 
#include <LiquidCrystal_I2C.h> // https://github.com/marcoschwartz/LiquidCrystal_I2C
#include <Adafruit_MAX31865.h> // https://github.com/adafruit/Adafruit_MAX31865
#include "LCD_symbols.hpp"
#include "button_handler.hpp"
#include "mode_control.hpp"
#include "flow_control.hpp"


// I2C 1602 display
#define LCD_ADDR  0x3F // For PCF8574T in I2C module adress is 0x3F
#define LCD_COLS  16
#define LCD_ROWS  2

// MAX31865 controller pinout (software SPI)
#define PIN_SPI_CS   2
#define PIN_SPI_SDI  3
#define PIN_SPI_SDO  4
#define PIN_SPI_CLK  5

// The value of the Rref resistor. Use 430.0! (in MAX31865 controller)
#define RREF 437.5 //437.37

// Relays pinout
#define PIN_RELAY_HEAT  6
#define PIN_RELAY_VENT  7

// Buttons pinout
#define PIN_BUTTON_PLUS    9
#define PIN_BUTTON_MINUS   10
#define PIN_BUTTON_SELECT  11
#define PIN_BUTTON_START   12

// Buzzer pinout
#define PIN_BUZZER  8




// Instanciate lcd object 
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// Instanciate MAX31865 object
Adafruit_MAX31865 max31865 = Adafruit_MAX31865(PIN_SPI_CS, PIN_SPI_SDI, PIN_SPI_SDO, PIN_SPI_CLK);

// Instanciate button objects
button_handler button_plus(PIN_BUTTON_PLUS);
button_handler button_minus(PIN_BUTTON_MINUS);
button_handler button_select(PIN_BUTTON_SELECT);
button_handler button_start(PIN_BUTTON_START);

// Instanciate mode object
mode_control mode;

// Instanciate flow object with relays pinouts
flow_control flow(PIN_RELAY_HEAT, PIN_RELAY_VENT);

// Instanciate buzzer object
buzzer_control buzzer(PIN_BUZZER);




/************* Temperature approximation **************/
float average_temp()
{
	static float curr_temp_sum = 0;
	static byte temp_probe_count = 0;	
	static float average_temp = 0;
	
	curr_temp_sum += max31865.temperature(100, RREF);
	++temp_probe_count;
	
	if ( temp_probe_count == 5 ) {
		average_temp = curr_temp_sum / temp_probe_count;
		curr_temp_sum = average_temp * 2;
		temp_probe_count = 2;
	}
	
	return average_temp;
}
/************* Temperature approximation **************/

/***************** FAULT DETECTION ********************/
uint8_t fault_detect()
{
	// Check and print any faults
	uint8_t read_fault = max31865.readFault();
	uint8_t fault = 0;
	if (read_fault) {
		fault = read_fault;
		max31865.clearFault();
	}
	return fault;
}
/***************** FAULT DETECTION ********************/

/******************************** PRINT FAULT *********************************/
void print_fault( float rtd_resistance, int rtd_temperature, byte MAX31865_fault )
{	
	lcd.setCursor(0, 0);
	lcd.print("RTD ");
	
	int first_row_pos = 4;
	lcd.setCursor(first_row_pos, 0);
	lcd.print( rtd_resistance, 2);
	
	if ( rtd_resistance < 10 ) {
		first_row_pos += 4;
	} else if ( rtd_resistance < 100 ) {
		first_row_pos += 5;
	} else {
		first_row_pos += 6;
	}
	lcd.setCursor(first_row_pos, 0);
	lcd.print("\364");
	++first_row_pos;
	
	lcd.setCursor(first_row_pos, 0);
	lcd.print(" ");
	++first_row_pos;
	
	lcd.setCursor(first_row_pos, 0);	
	lcd.print(rtd_temperature);
	
	if ( rtd_temperature < -100  ) {
		first_row_pos += 4;
	} else if ( rtd_temperature < -10 &&  rtd_temperature > -100 ) {
		first_row_pos += 3;
	} else if ( rtd_temperature < 0 &&  rtd_temperature > -10 ) {
		first_row_pos += 2;
	} else if ( rtd_temperature < 10 ) {
		first_row_pos += 1;
	} else if ( rtd_temperature < 100 ) {
		first_row_pos += 2;
	} else {
		first_row_pos += 3;
	}
	lcd.setCursor(first_row_pos, 0);
	lcd.print("\337");
	++first_row_pos;

	while ( first_row_pos < 16 ) {
		lcd.setCursor(first_row_pos, 0);
		lcd.print(" ");
		++first_row_pos;
	}
			
	lcd.setCursor(0, 1);
	lcd.print("Err D");
	uint8_t i; 
	uint8_t fault_register;
	uint8_t second_row_pos = 5;

	for ( i = 128, fault_register = 7; fault_register >= 2; i>>=1, fault_register-- ) {
		if (MAX31865_fault & i) {
			lcd.setCursor(second_row_pos, 1);
			lcd.print(fault_register);
			++second_row_pos;
		} else {
			lcd.setCursor(second_row_pos, 1);
			lcd.print(".");
			++second_row_pos;
		}
		if ( fault_register != 2 ) {
			lcd.setCursor(second_row_pos, 1);
			lcd.print("|");
			++second_row_pos;
		}
	}
	
}
/******************************** PRINT FAULT *********************************/




void setup()
{  
	lcd.init();                     
	lcd.backlight();
	lcd.setCursor(1, 0);
	lcd.print("GYUMRI MEDICAL");
	lcd.setCursor(5, 1);
	lcd.print("CENTER");
	delay(3000);
	lcd.clear();
	
	lcd_symbols::create();
	
	max31865.begin(MAX31865_3WIRE);
	Serial.begin(9600);
	
	// initialize buttons pins
	button_plus.init();
	button_minus.init();
	button_select.init();
	button_start.init();
	
	// intitialize default mode
	mode.init();
	//mode.set_temp_barier(111);
	//mode.set_time_barier(111);
	
	// intitialize flow control parameters and relay pins
	flow.init();
	
	// initialize buzzer pin
	buzzer.init();
}


void loop()
{
	mode.control( button_plus, button_minus, button_select, button_start );
	
	byte temp_barier = mode.get_temp_barier();
	byte time_barier = mode.get_time_barier();
	
	byte current_mode = mode.get_current_mode();
	byte last_mode = mode.get_last_mode();
	
	int current_temp = int( average_temp() );
	byte MAX31865_fault = fault_detect();
	
	if ( (MAX31865_fault && last_mode != 3) || (current_temp < 0) || (current_temp > 230) ) {
		mode.set_current_mode(3);
	}
	
	flow.control( current_mode, current_temp, temp_barier, time_barier );
	
	
	if ( !MAX31865_fault && current_mode != 3 ) {
		
		if ( last_mode == 3 ) {
			lcd.clear();
		}
		
		/**************111 First Row 111*****************/
		lcd.setCursor(0, 0);
		lcd.print("Temp ");
		
		if ( temp_barier < 100 ) {
			lcd.setCursor(5, 0);
			lcd.print(" ");
			lcd.setCursor(6, 0);
		} else {
			lcd.setCursor(5, 0);
		}
		lcd.print(temp_barier);    // print temperature barier
	  
		if ( mode.is_temp_barier_setting() ) {
			lcd_symbols::set(8, 0);
		} else {
			lcd.setCursor(8, 0);
			lcd.print(" ");
		}

		if ( current_temp < 100) {
			lcd.setCursor(10, 0);
			lcd.print(" ");
			lcd.setCursor(11, 0);
		} else {
			lcd.setCursor(10, 0);
		}
		lcd.print(current_temp);   // print current temperature
		lcd.setCursor(13, 0);
		lcd.print("\337");
		lcd.setCursor(14, 0);
		lcd.print("C");
		
		/**************222 Second Row 222*****************/
		lcd.setCursor(0, 1);
		lcd.print("Time ");
		
		if ( time_barier < 100 ) {
			lcd.setCursor(5, 1);
			lcd.print(" ");
			lcd.setCursor(6, 1);
		} else {
			lcd.setCursor(5, 1);
		}
		lcd.print(time_barier);    // print time barier
	  
		if ( mode.is_time_barier_setting() ) {
			lcd_symbols::set(8, 1);
		} else {
			lcd.setCursor(8, 1);
			lcd.print(" ");
		}
		///////////////////////////////////////////////////////////////// second row
		
		
		if ( flow.get_heat_relay_state() ) {
			lcd_symbols::heat(15, 0);   // print heat symbol
		} else {
			lcd.setCursor(15, 0);
			lcd.print(" ");             // clear heat symbol
		}
		
		if ( flow.get_vent_relay_state() ) {
			lcd_symbols::vent(15, 1);   // print vent symbol
		} else {
			lcd.setCursor(15, 1);
			lcd.print(" ");             // clear vent symbol
		}
		
		int elapsed_time = flow.get_elapsed_time();
		
		if ( flow.is_timer_started() ) {
			if ( elapsed_time < 10 ) {
				lcd.setCursor(10, 1);
				lcd.print("  ");
				lcd.setCursor(12, 1);
			} else if (elapsed_time < 100) {
				lcd.setCursor(10, 1);
				lcd.print(" ");
				lcd.setCursor(11, 1);
			} else {
				lcd.setCursor(10, 1);
			}
			lcd.print(elapsed_time);    // print sterilization time in minutes
			lcd.setCursor(13, 1);
			lcd.print("m");
		}
		
		static unsigned long int FinishTimeDelta;
		static bool finish_biiset;
		
		if ( flow.is_operation_finished() ) {
			lcd.setCursor(10, 1);
			lcd.print(" END");
			buzzer.finish();

			finish_biiset = 1;
			FinishTimeDelta = millis();

			mode.set_current_mode(0);   // All done, back to default mode
		}
		
		if ( finish_biiset && (millis() - FinishTimeDelta > 60000) ) {
			lcd.setCursor(10, 1);
			lcd.print("    ");          // clear "FINISH" sign
			finish_biiset = 0;
		}
		
	} else {
		
		uint16_t rtd = max31865.readRTD();
		float ratio = rtd;
		ratio /= 32768;		
		float rtd_resistance = RREF * ratio;
		
		int rtd_temperatureerature = int( max31865.temperature(100, RREF) );
		
		print_fault( rtd_resistance, rtd_temperatureerature, MAX31865_fault );
		
	}
	
	
}
