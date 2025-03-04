#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
/*
ISR(TIM1_COMPA_vect)
{
	TCCR0A = _BV(COM0B1) | _BV(WGM01); //clear on match
	TCCR0B |= _BV(FOC0B); //force match to force pin low, strobe on
	TCCR0A = _BV(COM0B1) | _BV(COM0B0) | _BV(WGM01); //set high on match
	TCNT0 = 0; //simulate ctc
}
*/

#define BIT_ZERO 3
#define BIT_ONE 6

uint8_t value = 0x0;
uint8_t test = 110;
ISR(ADC_vect)
{
	/*
	value = ADCH;
	ADCSRA |= _BV(ADSC);

	if (value>190)	
	{
		test--;
		PORTB ^= _BV(PB3);
	}
	else
	{
		test++;
	}

	if (test>110) test = 110;
	if (test<11) test=11;
	*/
//	if (test>110)
//	OCR0B = test;

}

uint8_t bit = BIT_ZERO;

ISR(TIMER0_COMPB_vect)
{
	bit = (bit==BIT_ZERO)? BIT_ONE : BIT_ZERO;
	OCR0B = bit;
}

static void setup_clock()
{
//	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
//	CLKPR = 0x00; //no divisor
	/* 1 Mhz clock */
}

static void setup_timers()
{
	TCCR0A = _BV(COM0B1) | _BV(WGM00)| _BV(WGM01);
	TCCR0B = _BV(CS00) | _BV(WGM02);

	TIMSK = _BV(OCIE0B); //timer 0 counter B interrupt
}

/*
static void setup_adc()
{
	ADMUX = _BV(REFS2) | _BV(REFS1) | _BV(MUX1) | _BV(ADLAR); //ADC on PB4
	ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1);
}
*/

#define NOP asm volatile ("nop;")
void send_bytes(uint8_t* x, uint8_t size) {
	int8_t itr;

	uint8_t high, low, i, byte;

	cli();

	high = PORTB | _BV(PB1);
	low = high & ~_BV(PB1);

	while(i<size) {
		byte = x[i++];

		//this ASM is tuned for an AVR that runs at 8MHz
		asm volatile(
			"push %1\n\t"
			"ldi %0, 7\n\t"	//loop over 8 bits
			"cln\n\t"		//clear negative flag

			"nextbit%=: brlt loopend%=\n\t"		//check if all bits have been processed
			"out 0x18, %2\n\t"		//data bit high
			"rol %1\n\t"			//rotate left into carry
			"brcs sethigh%=\n\t"	//jump to high bit if carry is set, 2 ticks
			"setlow%=: nop\n\t"		//begin of set low timing
			"out 0x18, %3\n\t"		//set data bit low
			"subi %0, 1\n\t"		//decrement counter
			"nop\n\t"
			"nop\n\t"
			"rjmp nextbit%= ; to nextbit\n\t"	//jump to the start of the next bit
			"sethigh%=: subi %0, 1\n\t"		//begin of set high timing, decrement counter
			"nop\n\t"
			"nop\n\t"
			"out 0x18, %3\n\t"					//set data bit low
			"rjmp nextbit%= ; to nextbit\n\t" 	//jump to the start of the next bit

			"loopend%=: nop\n\t"
			"pop %1\n\t"
		:	"=&d" (itr)
		:	"r" (byte), "r" (high), "r" (low), "I" (_SFR_IO_ADDR(PORTB)), "I" (0x01)
		);
	}

	sei();
}

void send_color(uint8_t r, uint8_t g, uint8_t b) {
//	PORTB |= _BV(PB1);
//	PORTB = 0;
	send_byte(g);
	send_byte(r);
	send_byte(b);
}

int main( void )
{
	cli();

	setup_clock();
//	setup_timers();
//	setup_adc();
	DDRB = _BV(PB1) | _BV(PB3); //output pin

	OCR0A = 10; //Reset PWM counter here. Gives duration of 1.350us
//	OCR0B = BIT_ONE; //which bit to send

	sei();

//	ADCSRA |= _BV(ADSC);

//PORTB = _BV(PB3);
	while(1) {
		send_color(255,0,0);
		send_color(0,255,0);
		send_color(0,0,255);
		_delay_us (100);
//		PORTB = 0;
//		_delay_us (100);
	}

	return 0;
}

/*

Copyright (c) 2013 Joshua Allen

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

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
