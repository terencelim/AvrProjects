#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include <stdlib.h>
#include "avr_print.h"

#include "usb.h"
#include "avrUsbUtils.h"

#include "mytypes.h"

#define STEPCOUNT 6
//uint8_t steps[STEPCOUNT] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x05 };
//uint8_t i = 0;

#define PWM_MAX 0xffff
#define PWM_MIN 2448
//#define PWM_MIN 4896

#define MOTORMASK 0x03

#define STOP 0x00
#define FORWARD 0x01
#define BACKWARD 0x02
#define BRAKE 0x03

vuint8_t direction;

#define MOTOR_FLAG_ON 0x01
vuint8_t motorflags = 0;

// two output pins need to be driven with pwm so do our own handling of pin toggling using counter interrupts
ISR(TIMER1_OVF_vect)
{
	//interrupt to turn on PWM pulse
	PORTD = (PORTD & ~MOTORMASK) | direction;
}

ISR(TIMER1_COMPA_vect)
{
	//interrupt to turn off PWM pulse
//	PORTD = (PORTD & ~MOTORMASK) | BRAKE;
	PORTD = (PORTD & ~MOTORMASK) | STOP;
}

ISR(INT2_vect)
{
	uint8_t t = PIND & (_BV(PD2) | _BV(PD3));
	if (t == _BV(PD2))
		y_pos++;
}

ISR(INT3_vect)
{

	uint8_t t1 = PIND & (_BV(PD2) | _BV(PD3));
	if (t1 == _BV(PD3))
		y_pos--;
}

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00; //no divisor
}

//0 to 65535
static void set_motor_pwm(uint32_t t)
{
	if (!(motorflags && MOTOR_FLAG_ON))
	{
		//disable interrupt
		TIMSK1 &= ~_BV(TOIE1);
	}
	else if (t==0)
	{
		//disable interrupt
		TIMSK1 &= ~_BV(TOIE1);
	}
	else if (t<PWM_MIN) 
	{
		OCR1A = PWM_MIN;
		TIMSK1 |= _BV(TOIE1);
	}
	else
	{
		OCR1A = t;		
		TIMSK1 |= _BV(TOIE1);
	}
}

static void setup_timers()
{
	//16mhz / 256
//	TCCR1A = _BV(WGM11) | _BV(WGM10);//Toggle OC1A
	TCCR1B =  _BV(CS11); //CTC, 256 divisor, 62.5kHz

	TIMSK1 = _BV(OCIE1A) | _BV(TOIE1);

	set_motor_pwm(0);
}

static void SetupDriverPins()
{
	DDRD = _BV(PD0) | _BV(PD1) | _BV(PD6);
	PORTD = 0;
}

void forward() {
//	PORTD &= ~_BV(PD1);
//	PORTD |= _BV(PD0);
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		direction = FORWARD;
	}
}

void backward() {
//	PORTD &= ~_BV(PD0);
//	PORTD |= _BV(PD1);
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		direction = BACKWARD;
	}
}

void stop() {
	PORTD &= ~(_BV(PD1)|_BV(PD2));
	direction = STOP;
}

void slew(int32_t *dx)
{
	uint32_t adx = labs(*dx);
 
	if (*dx < 0 )
		backward();
	else
		forward();

	if (adx<0xF)	
		set_motor_pwm(0);
	else if (adx<0x2000)
		set_motor_pwm(PWM_MIN*3);
	else if (adx<0x10000)
		set_motor_pwm(PWM_MIN*8);
	else
		set_motor_pwm(0xffff);
		
}
 //AVR has 512 bytes os ram available

extern vuint8_t doUSBstuff;

vuint32_t x_pos = 0x0;
vuint32_t y_pos = 0x0;
vuint32_t x_dest = 0x0;
vuint32_t y_dest = 0x0;

int32_t ComputeOffset(vuint32_t* pos, vuint32_t* dest )
{
	uint32_t p,d;

	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		p = *pos;
		d = *dest;
	}

	return (d - p);
}

int main( void )
{
	int32_t tmp = 0x0;

	cli();

	EICRA = _BV(ISC21) | _BV(ISC20) | _BV(ISC31) | _BV(ISC30);
	EIMSK = _BV(INT2) | _BV(INT3);

	y_pos = 0;

//	DDRC = _BV(PC6); //output pin
	SetupDriverPins();

	setup_clock();
	setup_timers();
	SetupPrintf();

	USB_ZeroPrescaler();
	USB_Init();

	sei();

//	forward();
//	while(1);
//	set_motor_pwm(32000);

	while(1)
	{
		while (doUSBstuff)
		{
			doUSBstuff = 0;
			PollEndpoint0();
//			DoUsbThings();
		}
		yp = y_pos;

//		PORTD &= ~_BV(PD6);

//		if (t>0xff000000) t = 0;

		tmp = ComputeOffset(&y_pos, &y_dest);
		slew(&tmp);
/*
		sendhex8(&dy);
		sendchr('\n');
		*/
	}

	return 0;
}

/*

Copyright (c) 2015 Joshua Allen

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
