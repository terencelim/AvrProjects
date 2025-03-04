#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>

#include <avr/pgmspace.h>

/*
//linear fade
char steps[] PROGMEM = { 255, 180, 128, 90, 64, 45, 32, 23, 16, 12, 8, 6, 4, 3, 2, 1 };

//randomly generated steps
char flicker[] PROGMEM = { 8,3,0,10,11,
	1,1,13,13,7,
12,15,10,4,13,
7,14,15,15,4,
11,11,14,3,4,
6,14,0,5,4,
12,15,1,5,15,
7,3,7,1,0,
13,3,5,10,7,
1,9,15,9,11,
4,14,15,13,10,
6,11,1,14,0,
9,9,13,0,8,
5,6,6,2,13,
14,9,3,12,0,
1,15,12,1,15,
6,7,8,7,12,
4,5,12,8,0,
2,2,1,14,6,
15,1,5,4,0 };
*/

//from http://petemills.blogspot.com/2012/02/attiny-candle.html
unsigned char flicker[] PROGMEM = 
{
0x77,0x7B,0x7D,0x7B,0x78,0xBA,0x95,0x53,0x5E,0x3E,0x3E,0x43,0x52,0x35,0x3E,0x75,0x95,
0x4B,0x65,0x7B,0x52,0x63,0x58,0x6B,0x55,0x62,0x77,0x72,0x87,0x85,0x8C,0x5D,0x7A,0x2E,
0x53,0x5D,0x50,0x62,0x55,0x6F,0x67,0x58,0x78,0x4E,0x55,0x95,0x6B,0x6D,0x7D,0x73,0x73,
0x7D,0x5B,0x6B,0x68,0x6A,0x6A,0x60,0x6B,0x77,0x77,0x98,0x7A,0x87,0x60,0x5B,0x6B,0xC9,
0x53,0x4A,0x53,0x68,0x45,0x3B,0x65,0x67,0x9C,0x60,0x67,0x53,0x73,0x75,0x63,0x8A,0x81,
0x7F,0x83,0x88,0x80,0x63,0x58,0x58,0x6B,0x7A,0x78,0x7B,0x83,0x8A,0x87,0x8A,0x85,0x08,
0x83,0x88,0x8A,0x8A,0x8A,0x8A,0x8A,0x8C,0x8A,0x8A,0x8A,0x8A,0x8A,0x88,0x83,0x7F,0x0B,
0x7D,0x7B,0x7A,0x78,0x77,0x75,0x70,0x70,0x72,0x70,0x70,0x4D,0x41,0x6D,0x68,0x60,0xB5,
0x35,0x35,0x3D,0x3B,0x41,0x45,0x52,0x5E,0x41,0x53,0x5D,0x60,0x65,0x6A,0x50,0x48,0xA0,
0x4B,0x4E,0x35,0x35,0x31,0x33,0x33,0x36,0x3B,0x40,0x50,0x4E,0x52,0x5D,0x60,0x53,0x15,
0x56,0x4B,0x35,0x2D,0x2E,0x2E,0x35,0x38,0x38,0x39,0x3B,0x38,0x31,0x58,0x40,0x60,0x77,
0x4D,0x50,0x5A,0x5D,0x43,0x40,0x53,0x58,0x5A,0x55,0x4E,0x31,0x31,0x2B,0x2E,0x33,0xD3,
0x31,0x36,0x35,0x36,0x38,0x39,0x39,0x38,0x40,0x4A,0x41,0x3B,0x50,0x62,0x40,0x36,0x4E,
0x29,0x2D,0x45,0x5E,0x5D,0x52,0x3E,0x33,0x3B,0x43,0x35,0x45,0x38,0x35,0x31,0x33,0x3E,
0x36,0x36,0x39,0x36,0x38,0x3B,0x41,0x36,0x36,0x30,0x39,0x33,0x2B,0x29,0x33,0x5A,0x98,
0x63,0x56,0x41,0x3D,0x50,0x52,0x55,0x60,0x65,0x55,0x3B,0x30,0x2E,0x30,0x3B,0x4E,0x66,
0x36,0x2E,0x2B,0x3B,0x39,0x3D,0x4A,0x50,0x3D,0x45,0x58,0x4E,0x4B,0x4E,0x4A,0x45,0xC5,
0x58,0x4B,0x55,0x5D,0x5B,0x56,0x58,0x5E,0x60,0x77,0x5E,0x38,0x5A,0x52,0x46,0x4B,0x79,
0x50,0x4A,0x4A,0x35,0x4E,0x41,0x2E,0x36,0x36,0x38,0x52,0x4B,0x46,0x3B,0x33,0x40,0xC4,
0x4E,0x60,0x5A,0x50,0x4D,0x43,0x4A,0x50,0x4B,0x48,0x40,0x3D,0x40,0x46,0x52,0x5B,0xFA,
0x62,0x63,0x63,0x5B,0x52,0x46,0x5B,0x43,0x55,0x45,0x26,0x35,0x3B,0x5B,0x43,0x4D,0xDB,
0x40,0x56,0x58,0x5A,0x5D,0x50,0x46,0x45,0x45,0x41,0x3B,0x43,0x72,0x87,0x90,0x8A,0x08,
0x8F,0x97,0x9D,0x95,0x73,0x65,0x6B,0x4D,0x46,0x45,0x55,0x55,0x41,0x56,0x55,0x55,0x31,
0x56,0x5A,0x5A,0x5B,0x5E,0x56,0x62,0x55,0x65,0x58,0x5A,0x62,0x68,0x6D,0x6D,0x6B,0x89,
0x68,0x6A,0x6F,0x65,0x6D,0x31,0x6F,0x55,0x48,0x50,0x55,0x67,0x5A,0x41,0x55,0x5E,0xC5,
0x60,0x65,0x68,0x68,0x63,0x60,0x6A,0x60,0x67,0x6A,0x7F,0x83,0x8C,0x87,0x88,0x92,0x3D,
0x8D,0x8F,0x88,0x8C,0x8C,0x85,0x82,0x6A,0x4E,0x35,0x23,0x11,0x19,0x43,0x3B,0x41,0x93,
0x67,0x4A,0x4A,0x3B,0x2E,0x30,0x45,0x41,0x4A,0x58,0x48,0x70,0x5B,0x6D,0x72,0x62,0x2F,
0x75,0x67,0x56,0x5A,0x5E,0x55,0x4D,0x77,0x53,0x2D,0x36,0x41,0x5D,0x55,0x40,0x40,0x03,
0x40,0x40,0x40,0x3E,0x41,0x5E,0x82,0x92,0x88,0x88,0x90,0x94,0x88,0x85,0x7B,0x63,0x4F,
0x55,0x53,0x56,0x55,0x50,0x53,0x55,0x03,0x34,0x01,0x3A,0x7E,0xFF,0x01,0x60,0x3E,0x36,
0x3E,0x28,0x01,0x8E
};
/*
,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x16,
0xFF
};
*/

uint16_t offset = 0;

ISR(TIM1_COMPA_vect)
{
	OCR0A = pgm_read_byte(flicker+offset);
	++offset;
	if (offset>=sizeof(flicker)) offset = 0;
}

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;// | _BV(CLKPS1); //no divisor
}

static void setup_pwm()
{
	//FAST PWM running at cpu speed
	TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);
	TCCR0B = _BV(CS00);//  | _BV(CS01);

	//timer to trigger advancing animation, 100ms
	TCCR1A = 0;
	TCCR1B = _BV(WGM12) | _BV(CS10) | _BV(CS12);
	TIMSK1 = _BV(OCIE1A);
	OCR1A = 781;
}

int main( void )
{
	cli();

	DDRB = _BV(PB2); //pwm output for LED

	setup_clock();
	setup_pwm();

	OCR0A = 0; //25%

	sei();

	while(1);

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
