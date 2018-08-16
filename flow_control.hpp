#ifndef FLOW_CONTROL_HPP
#define FLOW_CONTROL_HPP


const byte ON  = 1;
const byte OFF = 0;


class flow_control
{
	public:
		flow_control( byte pin_relay_heat, byte pin_relay_vent );
		
		void init();
		void control( byte current_mode, int current_temp, byte temp_barier, byte time_barier );

		bool get_heat_relay_state() const;
		bool get_vent_relay_state() const;
		
		bool is_timer_started() const;
		bool is_operation_finished() const;
		
		int get_elapsed_time() const;
		
	private:  
		void heat_relay(bool);
		void vent_relay(bool);
		void heating_power_control( float coefficient );
		void middle_power_heating();
		void low_power_heating();		
		
	protected:
		const byte pin_relay_heat;
		const byte pin_relay_vent;
		
		bool heat_relay_state;
		bool vent_relay_state;
	
		unsigned long int timer_start;
		bool timer_bitset;
		int elapsed_time;
		bool operation_finished;
		bool completed;
};


flow_control::flow_control( byte p_relay_heat, byte p_relay_vent )
			 : pin_relay_heat(p_relay_heat), pin_relay_vent(p_relay_vent)
{
	
}


void flow_control::init()
{
	// initialize relays pins
	pinMode(pin_relay_heat, OUTPUT);
	pinMode(pin_relay_vent, OUTPUT);
	
	heat_relay_state = false;
	vent_relay_state = false;
	
	timer_start = 0;
	timer_bitset = false;
	elapsed_time = 0;   // sterilization time in minutes
	operation_finished = false;
}


void flow_control::control( byte current_mode, int current_temp, byte temp_barier, byte time_barier )
{
	if ( current_mode == 2 ) {
		
		if ( current_temp >= temp_barier && !timer_bitset) {
			timer_start = millis();
			timer_bitset = true;
		}
		
		if ( timer_bitset ) {
			elapsed_time = (millis() - timer_start) / 60000;
		}
		
		if ( elapsed_time <= time_barier ) {
			
			vent_relay(ON);
			
			if ( current_temp < (temp_barier - 12) ) {
				heat_relay(ON);
			} else if ( (current_temp >= (temp_barier - 12)) && (current_temp < temp_barier) ) {
				float coefficient = (float)temp_barier / 220.0;
				heating_power_control( coefficient );
			} else if (current_temp >= temp_barier) {
				heat_relay(OFF);
			}
			
		} else {
			heat_relay(OFF);
			vent_relay(OFF);
			operation_finished = true;
			timer_bitset = false;
			timer_start = 0;
			elapsed_time = 0;
		}
		
	} else if ( current_mode == 0 || current_mode == 1) {
		heat_relay(OFF);
		vent_relay(OFF);
		
		timer_start = 0;
		elapsed_time = 0;
		timer_bitset = false;
		operation_finished = false;
	} else if ( current_mode == 3 ) {
		digitalWrite(pin_relay_heat, LOW);
		digitalWrite(pin_relay_vent, LOW);
		timer_start = 0;
		elapsed_time = 0;
		timer_bitset = false;
		operation_finished = false;
	}
}


bool flow_control::get_heat_relay_state() const
{
	return heat_relay_state;
}


bool flow_control::get_vent_relay_state() const
{
	return vent_relay_state;
}


bool flow_control::is_timer_started() const
{
	return timer_bitset;
}


bool flow_control::is_operation_finished() const
{
	return operation_finished;
}


int flow_control::get_elapsed_time() const
{
	return elapsed_time;
}


void flow_control::heat_relay( bool state )
{
	if ( state ) {
		digitalWrite(pin_relay_heat, HIGH);
		heat_relay_state = true;
	} else {
		digitalWrite(pin_relay_heat, LOW);
		heat_relay_state = false;
	}
}


void flow_control::vent_relay( bool state )
{
	if ( state ) {
		digitalWrite(pin_relay_vent, HIGH);
		vent_relay_state = true;
	} else {
		digitalWrite(pin_relay_vent, LOW);
		vent_relay_state = false;
	}
}


void flow_control::heating_power_control( float coefficient )
{
	int heatRelayTimeDelta = millis() % 5000;
	int period = coefficient * 5000;
	
	if ( heatRelayTimeDelta <= period ) {
		digitalWrite(pin_relay_heat, HIGH);
		heat_relay_state = true;
	} else if ( heatRelayTimeDelta > period ) { 
		digitalWrite(pin_relay_heat, LOW);
		heat_relay_state = false;
	}
}


void flow_control::middle_power_heating()
{
	int heatRelayTimeDelta = millis() % 5000;
	
	if ( heatRelayTimeDelta <= 2500 ) {
		digitalWrite(pin_relay_heat, HIGH);
		heat_relay_state = true;
	} else if ( heatRelayTimeDelta > 2500 ) { 
		digitalWrite(pin_relay_heat, LOW);
		heat_relay_state = false;
	}
}


void flow_control::low_power_heating()
{
	int heatRelayTimeDelta = millis() % 5000;
	
	if ( heatRelayTimeDelta <= 1750 ) {
		digitalWrite(pin_relay_heat, HIGH);
		heat_relay_state = true;
	} else if ( heatRelayTimeDelta > 1750 ) { 
		digitalWrite(pin_relay_heat, LOW);
		heat_relay_state = false;
	}
}


#endif // FLOW_CONTROL_HPP
