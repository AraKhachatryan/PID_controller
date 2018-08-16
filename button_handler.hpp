#ifndef BUTTON_HANDLER_HPP
#define BUTTON_HANDLER_HPP

#define DEF_DEBOUNCE_DELAY    25      // the debounce time
#define DEF_LONGPRESS_TIME    1000    // the long press time
#define DEF_SECRETPRESS_TIME  3000    // the secret press time
#define DEF_MAX_PRESS_TIME    20000   // the max press time of button

namespace EVENT {
	enum { 
	  NONE        = 0,   // or binary 0b000
	  SHORTPRESS  = 1,   // or binary 0b001
	  LONGPRESS   = 2,   // or binary 0b010
	  SECRETPRESS = 4    // or binary 0b100
	};
}



class button_handler
{
	public:
		// Constructor
		button_handler( byte pin, byte debounce_delay = DEF_DEBOUNCE_DELAY,
						int long_press_time = DEF_LONGPRESS_TIME, int secret_press_time = DEF_SECRETPRESS_TIME, int max_press_time = DEF_MAX_PRESS_TIME );

		// Initialization done after construction, to permit static instances
		void init();

		// Handler, to be called in the loop()
		byte handle();

		// return true if button pressed
		bool get_state() const;
		
		// return the last event
		byte get_last_event() const;
		
		// return the time duration of the pressed button
		int get_pressed_duration() const;
		
	protected:
		const byte pin;                // pin to which button is connected
		const byte debounce_delay;     // the debounce time
		const int long_press_time;     // the long press time
		const int secret_press_time;   // the secret press time
		const int max_press_time;      // the max press time of button
		
		bool now_pressed;     // the current reading from the button input pin
		bool was_pressed;     // the previous reading from the button input pin
		bool button_state;    // the button current debounced state
		
		byte event;            // the event at this time
		byte last_event;       // the last occurred event
		
		unsigned long int last_debounce_time;       // the last time the output pin was toggled
		unsigned long int button_switch_time;       // the last time the button switched
		unsigned long int button_pressed_duration;  // the time duration of the pressed button
};



button_handler::button_handler( byte p, byte db, int lp, int sp, int mp)
			   : pin(p), debounce_delay(db), long_press_time(lp), secret_press_time(sp), max_press_time(mp)
{
	
}


void button_handler::init()
{
	pinMode(pin, INPUT_PULLUP);   // internal pull-up 20k resistor, pushbutton's logic is inverted
	now_pressed = false;
	was_pressed = false;
	button_state = false;
	event = 0;
	last_event = 0;
	last_debounce_time = 0;
	button_switch_time = 0;
	button_pressed_duration = 0;
}


byte button_handler::handle()
{
	// by default if nothing special happens 
	event = EVENT::NONE;
	
	// read the state of button input pin
	// now_pressed is 1 when button is pressed and 0 when released
	now_pressed = !digitalRead(pin); 

	// If the switch changed, due to noise or pressing:
	if ( now_pressed != was_pressed ) {
		// reset the debouncing timer
		last_debounce_time = millis();
	}
	
	if ( millis() - last_debounce_time > debounce_delay ) {
		// whatever the reading is at, it's been there for longer than the
		// debounce delay, so take it as the actual current state:
		
		// if the button state has changed:
		if ( now_pressed != button_state ) {
			button_state = now_pressed;
			// reset the switching timer
			button_switch_time = millis();
			// and register an event after button state change
			if ( button_state ) {
				event = EVENT::SHORTPRESS;
				last_event = event;
			} else {
				event = EVENT::NONE;
				last_event = event;
			}				
		}
	}
  
	// remember button input reading state for the next loop call
	was_pressed = now_pressed;	

	// addutioanl button functional
	if ( button_state ) {
		button_pressed_duration = millis() - button_switch_time;
		
		// if button_pressed_duration in 1000...3000 ms, event is LONGPRESS
		if ( (button_pressed_duration > long_press_time) && (button_pressed_duration < secret_press_time) ) {
			// after button state change or SHORTPRESS, condidion is true only one time
			if ( (last_event & EVENT::LONGPRESS) != EVENT::LONGPRESS ){
				// event registered LONGPRESS only once
				event = EVENT::LONGPRESS;
				last_event = event;
			} else {
				event = EVENT::NONE;
			}
		// if button_pressed_duration in 3000...20000 ms, event is SECRETPRESS
		} else if ( (button_pressed_duration > secret_press_time) && (button_pressed_duration < max_press_time) ) {
			// after button state change or SHORTPRESS, condidion is true only one time
			if ( (last_event & EVENT::SECRETPRESS) != EVENT::SECRETPRESS ) {
				// event registered SECRETPRESS only once
				event = EVENT::SECRETPRESS;
				last_event = event;
			} else {
				event = EVENT::NONE;
			}
		}
	}
  
	return event;
}


bool button_handler::get_state() const
{
	return button_state;
}


byte button_handler::get_last_event() const
{
	return last_event;
}


int button_handler::get_pressed_duration() const
{
	return button_pressed_duration;
}


#endif // BUTTON_HANDLER_HPP
