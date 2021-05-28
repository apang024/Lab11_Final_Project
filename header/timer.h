#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0;	// TimerISR() sets this to 1. C programmer clear to 0

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model
unsigned long _avr_timer_M = 1;		// Start count from here, down to 0, default 1 ms
unsigned long _avr_timer_cntcurr = 0;	// Current internal count of 1 ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1B
	TCCR1B = 0x0B;	// 0000 1011; Clear timer on compare mode (TCNT1 counts 125,000 ticks/s)
	
	// AVR output compare register OCR1A
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A (# of ticks)

	// AVR timer interrupt mask register
	TIMSK1 = 0x02;	// bit1: OCIE1A -- enables compare match interrupt
	
	// Initialize AVR Counter
	TCNT1 = 0;
	
	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	_avr_timer_cntcurr = _avr_timer_M;
	
	// Enable global interrupts
	SREG |= 0x80;	// 0x80: 1000 0000
}

void TimerOff() {
	TCCR1B = 0x00;	// 0000 0000 , timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// use TimerISR instead of this one
ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
