/*	Author: Ashley Pang
 *	Lab Section: 021
 *	Assignment: Lab #11
 *	Exercise Description: Test Demo for LED
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "keypad.h"
#include "scheduler.h"
#include "timer.h"
#endif

//--------------------------------------
// LED Matrix Demo SynchSM
// Period: 100 ms
//--------------------------------------
enum Demo_States {shift};
int Demo_Tick(int state) {

    // Local Variables
    static unsigned char pattern = 0x80;    // LED pattern - 0: LED off; 1: LED on
    static unsigned char row = 0xFE;      // Row(s) displaying pattern.
                            // 0: display pattern on row
                            // 1: do NOT display pattern on row
    // Transitions
    switch (state) {
        case shift:
	    break;
        default:
	    state = shift;
            break;
    }
    // Actions
    switch (state) {
	case shift:
	    if (row == 0x7F && pattern == 0x01) { // Reset demo
                pattern = 0x80;
                row = 0xFE;
            } else if (pattern == 0x01) { // Move LED to start of next row
                pattern = 0x80;
                row = (row << 1) | 0x01;
            } else { // Shift LED one spot to the right on current row
                pattern >>= 1;
            }
            break;
        default:
    	break;
    }
    PORTC = ~pattern;    // Pattern to display
    PORTD = ~row;        // Row(s) displaying pattern
    return state;
}

enum setNumPadStates { setB };

int setBUsingNumPad(int state) {
    unsigned char x;
    switch (state) {
	case setB:
	    state = setB;
	    break;
   	default:
	    state = setB;
	    break;
    }
    switch (state) {
	case setB:
    	    x = GetKeypadKey();
	    PORTB = (x == '\0') ? 0x01 : 0x02;
	    break;
	default:
	    break;
    }
    return state;
}

int main(void) {
    DDRA = 0xF0; PORTA = 0x0F;		// PORTA: KEYPAD all input
    DDRB = 0x3F; PORTB = 0xC0;		// PORTB: B7-B6 = input, B5-B0 = output
    DDRC = 0xFF; PORTC = 0x00;		// PORTC: MATRIX COLUMNS all output
    DDRD = 0xFF; PORTD = 0x00;		// PORTD: MATRIX ROWS all output

    static task task1, task2;
    task* tasks[] = { &task1, &task2 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    const char start = -1;
    task1.state = start;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Demo_Tick;
    task2.state = start;
    task2.period = 10;
    task2.elapsedTime = task2.period;
    task2.TickFct = &setBUsingNumPad;

    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for( i = 1; i < numTasks; i++) {
    	GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();

    while(1) {
    	for (i = 0; i < numTasks; i++) {
	    if (tasks[i]->elapsedTime == tasks[i]->period) {
	    	tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
		tasks[i]->elapsedTime = 0;
	    }
	    tasks[i]->elapsedTime += GCD;
	}
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 0;
}
