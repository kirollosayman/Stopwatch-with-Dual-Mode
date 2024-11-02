// Include libraries
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//Global variables
char sec1=0,sec2=0,min1=0,min2=0,hour1=0,hour2=0;
char mode=1,pause_flag=0;
char mode_button_flag=1,set_buttons_flag=1;

//Peripherals Declaration
void Timer1_Init(void);
void Segment_Iint(void);
void Mode_button_Iint(void);
void Mode_led_Iint(void);
void buzzer_Iint(void);
void reset_button_INT0_Iint(void);
void resume_button_INT2_Iint(void);
void pause_button_INT1_Iint(void);

//Function Declaration
void hour_increment (void);
void min_increment (void);
void sec_increment (void);
void hour_decrement (void);
void min_decrement (void);
void sec_decrement (void);
void pause (void);
void resume (void);
void reset (void);
void Set_buttons_Iint(void);

//ISR
ISR(TIMER1_COMPA_vect)
{
	if(mode)
	{
		sec_increment();
	}
	else
	{
		sec_decrement();
	}
}
ISR(INT0_vect)
{
	reset();
}
ISR(INT1_vect)
{
	pause();
}
ISR(INT2_vect)
{
	resume();
}

//Main code
int main(void)
{
	//Initialization
	Timer1_Init();
	Segment_Iint();
	Mode_button_Iint();
	Mode_led_Iint();
	buzzer_Iint();
	reset_button_INT0_Iint();
	resume_button_INT2_Iint();
	pause_button_INT1_Iint();
	Set_buttons_Iint();

	while(1)
	{
		//Check for toggling mode
		if(!(PINB&(1<<PB7)))
		{
			_delay_ms(10);
			if(!(PINB&(1<<PB7))&&mode_button_flag)
			{
				mode^=1;
				mode_button_flag=0;
			}
		}
		else
		{
			mode_button_flag=1;
		}

		//check for mode
		if(mode)
		{
			PORTD |=(1<<PD4); //Turn on red led for count up
			PORTD &=~(1<<PD5);//Turn off yellow led for count down
		}
		else
		{
			PORTD |=(1<<PD5); //Turn on yellow led for count down
			PORTD &=~(1<<PD4);//Turn off red led for count up
		}

		//Set needed time when pause the stop watch
		if(pause_flag)
		{
			/*Use else if to reduce check time as
			 and use only one flag for all six buttons
			 and press one button in each time*/
			if(!(PINB&(1<<PB1)))
			{
				_delay_ms(10);
				if(!(PINB&(1<<PB1))&&set_buttons_flag)
				{
					hour_increment();
					set_buttons_flag=0;
				}
			}
			else if(!(PINB&(1<<PB0)))
			{
				_delay_ms(10);
				if(!(PINB&(1<<PB0))&&set_buttons_flag)
				{
					hour_decrement();
					set_buttons_flag=0;
				}
			}
			else if(!(PINB&(1<<PB4)))
			{
				_delay_ms(10);
				if(!(PINB&(1<<PB4))&&set_buttons_flag)
				{
					min_increment();
					set_buttons_flag=0;
				}
			}
			else if(!(PINB&(1<<PB3)))
			{
				_delay_ms(10);
				if(!(PINB&(1<<PB3))&&set_buttons_flag)
				{
					min_decrement();
					set_buttons_flag=0;
				}
			}
			else if(!(PINB&(1<<PB6)))
			{
				_delay_ms(10);
				if(!(PINB&(1<<PB6))&&set_buttons_flag)
				{
					sec_increment();
					set_buttons_flag=0;
				}
			}
			else if(!(PINB&(1<<PB5)))
			{
				_delay_ms(10);
				if(!(PINB&(1<<PB5))&&set_buttons_flag)
				{
					sec_decrement();
					set_buttons_flag=0;
				}
			}
			else
			{
				set_buttons_flag=1;
			}
		}

		//Loop on 7 segments
		PORTA = (PORTA&0xC0) | (1<<5);
		PORTC = (PORTC&0xF0) | (sec1&0x0F);
		_delay_ms(2);

		PORTA = (PORTA&0xC0) | (1<<4);
		PORTC = (PORTC&0xF0) | (sec2&0x0F);
		_delay_ms(2);

		PORTA = (PORTA&0xC0) | (1<<3);
		PORTC = (PORTC&0xF0) | (min1&0x0F);
		_delay_ms(2);

		PORTA = (PORTA&0xC0) | (1<<2);
		PORTC = (PORTC&0xF0) | (min2&0x0F);
		_delay_ms(2);

		PORTA = (PORTA&0xC0) | (1<<1);
		PORTC = (PORTC&0xF0) | (hour1&0x0F);
		_delay_ms(2);

		PORTA = (PORTA&0xC0) | (1<<0);
		PORTC = (PORTC&0xF0) | (hour2&0x0F);
		_delay_ms(2);

		//It works but not good in proteus
		/*for(char index=5;index>=0;index--)
		{
			char num=0;
			switch (index)
			{
				case 5:
					num=sec1;
					break;
				case 4:
					num=sec2;
					break;
				case 3:
					num=min1;
					break;
				case 2:
					num=min2;
					break;
				case 1:
					num=hour1;
					break;
				case 0:
					num=hour2;
					break;
			}
			PORTA = (PORTA&0xC0) | (1<<index);
			PORTC = (PORTC&0xF0) | (num&0x0F);
			_delay_ms(2);
		}*/
	}
}

//Peripherals initialization
void Timer1_Init(void)
{
	/*A we work with 16 MHz
	 * Choose CTC mode and select 256 prescaler and
	 * put 62500 in comparator register and open interrupt
	 * to make interrupt every one second*/
	TCCR1A = (1<< FOC1A) | (1<< FOC1B);
	TCCR1B = (1<< WGM12) | (1<<CS12);
	TCNT1 =0;
	OCR1A = 62500;
	TIMSK |= (1<<OCIE1A);
	SREG |= (1<<7);
}
void Segment_Iint(void)
{
	DDRC |= 0x0F; // Set first 4 pins output for Decoder
	DDRA |= 0x3F; //set first 6 pins output for multiplexed
}
void Mode_button_Iint(void)
{
	DDRB &= ~(1<<PB7);    // Set  pin 7 in PORTB as input for button
	PORTB |= (1<<PB7);    // Set  pin 7 in PORTB with value 1 for internal pull up
}
void Mode_led_Iint(void)
{
	DDRD |= (1<<PD4) | (1<<PD5);    // Set  pin 4 & 5 in PORTD as output for led of modes
	PORTD &= ~(1<<PD4) & ~(1<<PD5); //Set  pin 4 & 5 in PORTD with value 0 turn off leds
}
void buzzer_Iint(void)
{
	DDRD |= (1<<PD0);    // Set  pin 0 in PORTD as output for buzzer
	PORTD &= ~(1<<PD0); //Set  pin 0 in PORTD with value 0 turn off buzzer
}
void reset_button_INT0_Iint(void)
{
	DDRD  &= (~(1<<PD2));               // Configure INT0/PD2 as input pin
	PORTD |= (1<<PD2);    				// Set  pin 2 in PORTD with value 1 for internal pull up
	MCUCR |=(1<<ISC01);  				// Trigger INT0 with the falling edge
	GICR  |= (1<<INT0);                 // Enable external interrupt pin INT0
	SREG  |= (1<<7);                    // Enable interrupts by setting I-bit

}
void resume_button_INT2_Iint(void)
{
	DDRB  &= (~(1<<PB2));               // Configure INT2/PB2 as input pin
	PORTB |= (1<<PB2);    				// Set  pin 2 in PORTB with value 1 for internal pull up
	MCUCSR &=~(1<<ISC2);  				// Trigger INT2 with the falling edge
	GICR  |= (1<<INT2);                 // Enable external interrupt pin INT2
	SREG  |= (1<<7);                    // Enable interrupts by setting I-bit

}
void pause_button_INT1_Iint(void)
{
	DDRD  &= (~(1<<PD3));               // Configure INT1/PD3 as input pin
	MCUCR |=(1<<ISC11);  				// Trigger INT0 with the falling edge
	GICR  |= (1<<INT1);                 // Enable external interrupt pin INT0
	SREG  |= (1<<7);                    // Enable interrupts by setting I-bit
}
void Set_buttons_Iint(void)
{
	DDRB &= ~0x7B;    		// Set  pins 0,1,3,4,5 & 6 in PORTB as input for set buttons
	PORTB |= 0x7B;  	// Set  pins 0,1,3,4,5 & 6 in PORTB with value 1 for internal pull up
}

//Function Definition
void hour_increment (void)
{
	if(hour2==2 && hour1==3) //Loop every day if want to be 100 hour change this values to 99
	{
		hour1=0;
		hour2=0;
	}
	else if(hour1==9)
	{
		hour1=0;
		hour2++;
	}
	else
	{
		hour1++;
	}
}
void min_increment (void)
{
	if(min2==5 && min1==9)
	{
		min1=0;
		min2=0;
		hour_increment();
	}
	else if (min1==9)
	{
		min1=0;
		min2++;
	}
	else
	{
		min1++;
	}
}
void sec_increment (void)
{
	//Close alarm
	PORTD &= ~(1<<PD0);
	if(sec2==5 && sec1==9)
	{
		sec1=0;
		sec2=0;
		min_increment();
	}
	else if (sec1==9)
	{
		sec1=0;
		sec2++;
	}
	else
	{
		sec1++;
	}
}
void hour_decrement (void)
{
	if(hour2==0 && hour1==0)
	{

	}
	else if(hour1==0)
	{
		hour2--;
		hour1=9;
	}
	else
	{
		hour1--;
	}
}
void min_decrement (void)
{
	if(min2==0 && min1==0)
	{
		min1=9;
		min2=5;
		hour_decrement();
	}
	else if(min1==0)
	{
		min2--;
		min1=9;
	}
	else
	{
		min1--;
	}
}
void sec_decrement (void)
{
	if(hour2==0 && hour1==0 && min2==0 && min1==0 && sec2==0 && sec1==0)
	{
		// Alarm ring
		PORTD |= (1<<PD0);
	}
	else if(sec2==0 && sec1==0)
	{
		sec1=9;
		sec2=5;
		min_decrement();
	}
	else if(sec1==0)
	{
		sec2--;
		sec1=9;
	}
	else
	{
		sec1--;
	}
}
void pause (void)
{
	TCCR1B &= ~(1<<CS12);
	pause_flag=1;
}
void resume (void)
{
	TCCR1B |= (1<<CS12);
	pause_flag=0;
}
void reset (void)
{
	sec1=0;
	sec2=0;
	min1=0;
	min2=0;
	hour1=0;
	hour2=0;
}
