/*
	This is a program for emulating how the software will work on the attiny44. Used for developing and debugging algorithims.
	gcc -Wall -lm test.c
*/

#include <stdint.h>
#include <stdio.h>

#define TICKS_SEC 100
#define LOOKUP_LENGTH 204

uint16_t step_lookup_table[] = {
0x0000,0x0000,0x0418,0x15A3,0x0237,0x0BB2,0x01E8,0x0A0E,0x01A2,0x089A,0x0187,0x0809,0x0197,0x085A,0x0183,0x07ED,0x0140,0x068B,0x013B,0x066E,
0x012D,0x0622,0x0114,0x059D,0x0117,0x05AA,0x010B,0x0569,0x00FE,0x0523,0x00F5,0x04F2,0x00F8,0x04FF,0x00F6,0x04F2,0x00F4,0x04E5,0x00E1,0x0481,
0x00D3,0x0437,0x00D4,0x043A,0x00D9,0x0451,0x00B7,0x03A2,0x00C2,0x03D8,0x00C5,0x03E5,0x00B2,0x0383,0x00C6,0x03E6,0x00DB,0x044D,0x00CF,0x040E,
0x00F5,0x04CA,0x012C,0x05D7,0x012E,0x05DB,0x011D,0x0581,0x0116,0x0559,0x0111,0x053B,0x010E,0x0527,0x010B,0x0513,0x0108,0x04FF,0x0100,0x04D3,
0x00F6,0x049E,0x00F1,0x0481,0x00E2,0x0435,0x00F0,0x0473,0x00DB,0x040B,0x00DE,0x0415,0x00E3,0x0428,0x00D8,0x03F0,0x00C7,0x039D,0x00C8,0x039E,
0x00CB,0x03A8,0x00C4,0x0384,0x00BE,0x0365,0x00CA,0x0398,0x00C8,0x038B,0x00D2,0x03B4,0x00C7,0x037E,0x00B1,0x0318,0x00AB,0x02FA,0x00A4,0x02D8,
0x00A6,0x02DE,0x00A1,0x02C5,0x00A0,0x02BE,0x009D,0x02AE,0x0096,0x028D,0x00AA,0x02E1,0x0097,0x028C,0x0092,0x0274,0x0090,0x0269,0x0082,0x022B,
0x0096,0x027E,0x007A,0x0205,0x0074,0x01EA,0x0072,0x01E0,0x007F,0x0215,0x0079,0x01FA,0x0071,0x01D7,0x0067,0x01AC,0x0069,0x01B3,0x0073,0x01DB,
0x0068,0x01AC,0x0069,0x01AF,0x006F,0x01C6,0x0067,0x01A4,0x005C,0x0176,0x005B,0x0171,0x005E,0x017C,0x0060,0x0183,0x0069,0x01A6,0x00AD,0x02B5,
0x00B9,0x02E0,0x00C4,0x0307,0x00B9,0x02D7,0x00B4,0x02BF,0x00AB,0x0298,0x00B3,0x02B3,0x00B7,0x02BE,0x00AB,0x028C,0x00AB,0x0288,0x009F,0x0257,
0x00A5,0x026A,0x001E,0x0070
};

uint32_t prev_lookup_time = 0;
uint32_t prev_lookup_steps = 0;
uint32_t next_lookup_time = 0;
uint32_t next_lookup_steps = 0;

uint32_t max_a = 0;
uint32_t max_b = 0;

uint32_t time_ticks = 0; /* 1/100th seconds */
uint32_t completed_steps = 0;
uint32_t total_steps = 0;


uint16_t pgm_read_word(uint16_t* p)
{
	return *p;
}

inline float flerpStep(uint32_t time0, uint32_t time1, uint32_t step0, uint32_t step1, uint32_t time_x)
{
	float a = (time_x - time0)*1.0f;
	a/=(time1-time0);
	return step0+(step1-step0)*a;
}

inline uint32_t lerpStep(uint32_t time0, uint32_t time1, uint32_t step0, uint32_t step1, uint32_t time_x)
{
	/* fixed point math, use 15 bits for fractional component (picked based on largest possible a=104799) */
	uint32_t a,b;
	float xxxx = flerpStep(time0, time1, step0, step1, time_x);
	a = (time_x - time0);
//	if (a>max_a)max_a=a; //only useful when skipping bitshifting
	a<<=15;
	a/=(time1-time0);
//	d = ((step1-step0)*a)>>15;
	b = (step1-step0);
	if (a>max_a)max_a=a;
	a*=b;
	if (b>max_b)max_b=b; //only useful when skipping bitshifting
//	b=a;
	if(a&0x4000) a+= 0x8000; /* round up */
	a>>=15;
	a+=step0;
//	printf("%d %d %f ... %d\n", time_x, a, round(xxxx), b);
	return a;
}

void GetNextLookupValues(uint32_t current_time)
{
	uint16_t i;
	uint32_t tmp;
	next_lookup_time = 0;
	next_lookup_steps = 0;
	for (i = 0;i<LOOKUP_LENGTH;i+=2)
	{
		printf("loop\n");
		tmp = pgm_read_word(step_lookup_table+i);
		next_lookup_time += tmp*TICKS_SEC; /* get timestamp, convert into tick time */
		next_lookup_steps += pgm_read_word(step_lookup_table+i+1); //get steps
		if (next_lookup_time>current_time) return;
	}
}

uint32_t ComputeStep(uint32_t current_time)
{
	if (current_time>next_lookup_time)
	{
		prev_lookup_time = next_lookup_time;
		prev_lookup_steps = next_lookup_steps;
		GetNextLookupValues(current_time);
	}

	if (next_lookup_time==current_time) return next_lookup_steps;

	return lerpStep(prev_lookup_time, next_lookup_time, prev_lookup_steps, next_lookup_steps, current_time);
}

int main( void )
{
	while (time_ticks<(60*60*6*TICKS_SEC))
	{
		uint32_t steps = ComputeStep(time_ticks);
//		printf("T %d %d\n", time_ticks, steps);
		if (steps>total_steps)
		{
			printf("go %d -> %d\n", total_steps, steps);
			total_steps = steps;
		}
//		printf("%d 0x%04x 0x%04x %d\n", time_ticks, prev_lookup_time, next_lookup_time, steps);
		++time_ticks;
	}

	printf("max a %d\n",max_a);
	printf("max b %d\n",max_b);

	return 0;
}

/*

Copyright (c) 2014, Joshua Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Joshua Allen nor the names of other contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
