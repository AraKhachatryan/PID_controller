#ifndef BUZZER_HPP
#define BUZZER_HPP


class buzzer_control
{
	public:
		buzzer_control( byte pin );
		void init();
		void buttons( const byte &event_button_plus, const byte &event_button_minus, const byte &event_button_select, const byte &event_button_start );
		void finish();
		
	protected:
		const byte pin;
};


buzzer_control::buzzer_control( byte p )
			   : pin(p)
{

}			   


void buzzer_control::init()
{
	// initialize buzzer pin
	pinMode(pin, OUTPUT);
}


inline void buzzer_control::buttons( const byte &event_button_plus, const byte &event_button_minus, const byte &event_button_select, const byte &event_button_start )
{
	
	byte button_short_pressed = 0;
	button_short_pressed |= event_button_plus;
	button_short_pressed |= event_button_minus;
	button_short_pressed |= event_button_select;
	button_short_pressed |= event_button_start;
	button_short_pressed &= 1;
	
	byte button_long_pressed = 0;
	button_long_pressed |= event_button_plus;
	button_long_pressed |= event_button_minus;
	button_long_pressed |= event_button_select;
	button_long_pressed |= event_button_start;
	button_long_pressed &=  2;
			
	byte button_secret_pressed = 0;
	//button_secret_pressed |= event_button_plus;
	//button_secret_pressed |= event_button_minus;
	//button_secret_pressed |= event_button_select;
	button_secret_pressed |= event_button_start;
	button_secret_pressed &=  4;
	
	if ( button_short_pressed == 1 ) {
		tone(pin, 500, 200);
	} else if ( button_long_pressed == 2 ) {
		tone(pin, 700, 400);
	} else if ( button_secret_pressed == 4 ) {
		tone(pin, 800, 500);
	} else {
		//noTone(8);
	}	
	
}


void buzzer_control::finish()
{
	tone(pin, 700, 5000);
}


#endif // BUZZER_HPP
