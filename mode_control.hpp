#ifndef MODE_CONTROL_HPP
#define MODE_CONTROL_HPP

#include <EEPROM.h>
#include "buzzer.hpp"          // buzzer control

#define DEF_TEMP_BARIER  180   // default temperature start parameter
#define DEF_TIME_BARIER  120   // default timer start parameter

#define DEF_TEMP_LR  50        // default possible low temperature start parameter 
#define DEF_TEMP_HR  220       // default possible hight temperature start parameter
#define DEF_TIME_LR  10        // default possible low timer start parameter
#define DEF_TIME_HR  240       // default possible hight timer start parameter

#define DEF_TEMP_EE_ADDR  0    // default temperature start parameter EEPROM address
#define DEF_TIME_EE_ADDR  4    // default timer start parameter EEPROM address

namespace EV {
	enum { 
	  NONE        = 0,   // or binary 0b000
	  SHORTPRESS  = 1,   // or binary 0b001
	  LONGPRESS   = 2,   // or binary 0b010
	  SECRETPRESS = 4    // or binary 0b100
	};
}


// Instanciated button objects from Hot_Air_Sterilizer.ino file
extern button_handler button_plus;
extern button_handler button_minus;
extern button_handler button_select;
extern button_handler button_start;

// Instanciated buzzer object from Hot_Air_Sterilizer.ino file
extern buzzer_control buzzer;




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition

class mode_control
{
	public:
		// Constructor (to be called in global)
		mode_control( byte temp_low_range = DEF_TEMP_LR, byte temp_high_range = DEF_TEMP_HR, byte temp_barier_ee_addr = DEF_TEMP_EE_ADDR,
					  byte time_low_range = DEF_TIME_LR, byte time_high_range = DEF_TIME_HR, byte time_barier_ee_addr = DEF_TIME_EE_ADDR );
	
		void init();                           // Initialization with default start parameters, to be called in the setup()
		
		// Handler, to be called in the loop()
		void control( button_handler &button_plus, button_handler &button_minus,
				   button_handler &button_select, button_handler &button_start );
		
		void set_temp_barier(byte);            // set heating relay cut-off temperature barier    
		void set_time_barier(byte);            // set ventilating relay cut-off time barier
		
		void set_current_mode(byte);           // set the current mode
		
		byte get_temp_barier() const;          // get heating relay cut-off temperature barier 
		byte get_time_barier() const;          // get ventilating relay cut-off time barier
		
		byte get_current_mode() const;         // get current mode state
		byte get_last_mode() const;            // get last mode state
	
		bool is_temp_barier_setting() const;   // for showing in display
		bool is_time_barier_setting() const;   // for showing in display
	
	private:  
		void set_temp_barier_EEPROM();         // set temperature barier to EEPROM
		void set_time_barier_EEPROM();         // set time barier to EEPROM
		
		byte get_temp_barier_EEPROM() const;   // get temperature barier from EEPROM
		byte get_time_barier_EEPROM() const;   // get time barier from EEPROM
				
	protected:
		byte current_mode;               // the current mode
		byte last_mode;                  // the last mode
		byte temp_barier;                // temperature start parameter
		byte time_barier;                // timer start parameter
		
		bool temp_barier_set_state;      // true if temp_barier is setting now
		bool time_barier_set_state;      // true if time_barier is setting now
		
		const byte temp_barier_ee_addr;  // temperature start parameter EEPROM address
		const byte time_barier_ee_addr;  // timer start parameter EEPROM address
		
		const byte temp_low_range;       // possible low temperature start parameter
		const byte temp_high_range;      // possible hight temperature start parameter
		
		const byte time_low_range;       // possible low timer start parameter
		const byte time_high_range;      // possible hight timer start parameter
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


mode_control::mode_control( byte temp_lr, byte temp_hr, byte temp_ee_addr, byte time_lr, byte time_hr, byte time_ee_addr)
	: temp_low_range(temp_lr), temp_high_range(temp_hr), temp_barier_ee_addr(temp_ee_addr),
	time_low_range(time_lr), time_high_range(time_hr), time_barier_ee_addr(time_ee_addr)
{
	
}


void mode_control::init()
{
	last_mode = 0;
	current_mode = 0;	
	
	byte temp_barier_eeprom = this->get_temp_barier_EEPROM();
	if ( (temp_low_range <= temp_barier_eeprom) && (temp_high_range >= temp_barier_eeprom) ) {
		temp_barier = temp_barier_eeprom;
	} else {
		temp_barier = DEF_TEMP_BARIER;
	}

	byte time_barier_eeprom = this->get_time_barier_EEPROM();
	if ( (time_low_range <= time_barier_eeprom) && (time_high_range >= time_barier_eeprom) ) {
		time_barier = time_barier_eeprom;
	} else {
		time_barier = DEF_TIME_BARIER;
	}

	temp_barier_set_state = 0;
	temp_barier_set_state = 0;
}


void mode_control::control( button_handler &button_plus, button_handler &button_minus,
						 button_handler &button_select, button_handler &button_start )
{
	byte event_button_plus = button_plus.handle();
	byte event_button_minus = button_minus.handle();
	byte event_button_select = button_select.handle();
	byte event_button_start = button_start.handle();
	
	byte last_event_button_plus = button_plus.get_last_event();
	byte last_event_button_minus = button_minus.get_last_event();
//	byte last_event_button_select = button_select.get_last_event();
//	byte last_event_button_start = button_start.get_last_event();

	buzzer.buttons( event_button_plus, event_button_minus, event_button_select, event_button_start );
	
	switch(current_mode) {
		// DEFAULT mode (ready for start, plus and minus not available)
		case 0:
		{
			if ( EV::SHORTPRESS == event_button_start ) {          // shortpress
				last_mode = current_mode;                          // save the last mode
				current_mode = 2;                                  // next -> start and control relays
				break;
			} else if ( EV::SHORTPRESS == event_button_select ) {  // shortpress
				last_mode = current_mode;                          // save the last mode
				current_mode = 1;                                  // next -> go to select mode for control start parameters
				break;
			}
			
			last_mode = current_mode;
			
			break;
		}
		
		// SELECT mode (start not available)
		case 1:
		{
			static bool select_parameter = 1;   // temperature selected by default
			
			if ( EV::SHORTPRESS == event_button_select ) {   // shortpress
				select_parameter = !select_parameter;
			}      
			
			// temperature start parameter control
			if ( select_parameter ) {
				time_barier_set_state = 0;
				temp_barier_set_state = 1;
				if ( (EV::SHORTPRESS == event_button_plus) && (temp_high_range > temp_barier) ) {   // shortpress
					++temp_barier;
				} else if ( ((EV::LONGPRESS | EV::SECRETPRESS) & (event_button_plus | last_event_button_plus)) && (temp_high_range > temp_barier) ) {   // longpress and secretpress
					++temp_barier;
				}
				if ( (EV::SHORTPRESS == event_button_minus) && (temp_low_range < temp_barier)) {   // shortpress
					--temp_barier;
				} else if ( ((EV::LONGPRESS | EV::SECRETPRESS) & (event_button_minus | last_event_button_minus)) && (temp_low_range < temp_barier) ) {   // longpress and secretpress
					--temp_barier;
				}
			// time start parameter control
			} else {
				temp_barier_set_state = 0;
				time_barier_set_state = 1;
				if ( (EV::SHORTPRESS == event_button_plus) && (time_high_range > time_barier) ) {   // shortpress
					++time_barier;
				} else if ( ((EV::LONGPRESS | EV::SECRETPRESS) & (event_button_plus | last_event_button_plus)) && (time_high_range > time_barier) ) {   // longpress and secretpress
					++time_barier;					
				}
				if ( (EV::SHORTPRESS == event_button_minus) && (time_low_range < time_barier) ) {   // shortpress
					--time_barier;
				} else if ( ((EV::LONGPRESS | EV::SECRETPRESS) & (event_button_minus | last_event_button_minus)) && (time_low_range < time_barier) ) {   // longpress and secretpress
					--time_barier;
				}
			}
			
			if ( EV::LONGPRESS == event_button_select ) {   // longpress
				this->set_temp_barier_EEPROM();
				this->set_time_barier_EEPROM();
				temp_barier_set_state = 0;
				time_barier_set_state = 0;
				last_mode = current_mode;                   // save the last mode  
				current_mode = 0;                           // DEFAULT mode, next -> ready for start
				break;
			}
			
			last_mode = current_mode;
			
			break;
		}
		
		// OPERATION mode (select, plus, minus not available)
		case 2:
		{
			if ( EV::SECRETPRESS == event_button_start ) {  // secretpress 3 sec
				last_mode = current_mode;                   // save the last mode
				current_mode = 0;                           // abort operation, DEFAULT mode, next ->	shut down relays
				break;
			}
			last_mode = current_mode;
			break;
		}
		
		// ERROR mode
		case 3:
		{
			// To reset error mode hold plus and minus buttons same time 
			if ( (EV::LONGPRESS & (event_button_plus | last_event_button_plus)) && (EV::LONGPRESS & (event_button_minus | last_event_button_minus)) ) {
				last_mode = current_mode;                   // save the last mode
				current_mode = 0;                           // reset error, next -> DEFAULT mode, ready for start
				break;
			}
			last_mode = current_mode;
			break;
		}

	}
}



void mode_control::set_temp_barier(byte temp_bar)
{
	temp_barier = temp_bar;
}


void mode_control::set_time_barier(byte time_bar)
{
	time_barier = time_bar;
}


void mode_control::set_current_mode(byte curr_mode)
{
	last_mode = current_mode;
	current_mode = curr_mode;
}


byte mode_control::get_temp_barier() const
{
	return temp_barier;
}


byte mode_control::get_time_barier() const
{
	return time_barier;
}


byte mode_control::get_current_mode() const
{
	return current_mode;
}

byte mode_control::get_last_mode() const
{
	return last_mode;
}

bool mode_control::is_temp_barier_setting() const
{
	return temp_barier_set_state;
}

bool mode_control::is_time_barier_setting() const
{
	return time_barier_set_state;
}

/////////////////////////////////////////////////////////////// EEPROM control

void mode_control::set_temp_barier_EEPROM()
{
	if ( (temp_low_range <= temp_barier) && (temp_high_range >= temp_barier) ) {
		EEPROM.put(temp_barier_ee_addr, temp_barier);
	}
}


void mode_control::set_time_barier_EEPROM()
{
	if ( (time_low_range <= time_barier) && (time_high_range >= time_barier) ) {
		EEPROM.put(time_barier_ee_addr, time_barier);
	}
}


byte mode_control::get_temp_barier_EEPROM() const
{
	byte temp_barier_eeprom;
	EEPROM.get(temp_barier_ee_addr, temp_barier_eeprom);
	if ( (temp_low_range <= temp_barier_eeprom) && (temp_high_range >= temp_barier_eeprom) ) {
		return temp_barier_eeprom;
	}
}


byte mode_control::get_time_barier_EEPROM() const
{
	byte time_barier_eeprom;
	EEPROM.get(time_barier_ee_addr, time_barier_eeprom);
	if ( (time_low_range <= time_barier_eeprom) && (time_high_range >= time_barier_eeprom) ) {
		return time_barier_eeprom;
	}
}


#endif // MODE_CONTROL_HPP
