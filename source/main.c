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

unsigned int arrayNum = 16;
unsigned char array_ALL0[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
unsigned char array_0[] = { 0x3C, 0x42, 0x46, 0x4A, 0x52, 0x62, 0x42, 0x3C };
unsigned char array_1[] = { 0x08, 0x18, 0x28, 0x08, 0x08, 0x08, 0x08, 0x3C };
unsigned char array_2[] = { 0x38, 0x44, 0x44, 0x04, 0x7C, 0x40, 0x40, 0x7C };
unsigned char array_3[] = { 0x3C, 0x04, 0x04, 0x04, 0x3C, 0x04, 0x04, 0x3C };
unsigned char array_4[] = {  };
unsigned char array_5[] = {  };
unsigned char array_6[] = {  };
unsigned char array_7[] = {  };
unsigned char array_8[] = {  };
unsigned char array_9[] = {  };
unsigned char array_A[] = {  };
unsigned char array_B[] = {  };
unsigned char array_C[] = {  };
unsigned char array_D[] = {  };
unsigned char array_star[] = {  };
unsigned char array_pound[] = {  };
unsigned char array_play[] = {  };
unsigned char array_swoosh[] = {  };

enum numPadStates { numPadS1 };

int getNumPadInput(int state) {
    unsigned char x;
    switch (state) {
	case numPadS1:
	    state = numPadS1;
	    break;
	default:
	    state = numPadS1;
	    break;
    }
    switch (state) {
        case numPadS1:
            x = GetKeypadKey();
	    switch (x) {
	    	case '\0': arrayNum = 16; break;     // array_ALL0
  		case '0':  arrayNum = 0; break;      // array_0
    		case '1':  arrayNum = 1; break;      // array_1
        	case '2':  arrayNum = 2; break;      // array_2
        	case '3':  arrayNum = 3; break;      // array_3
        	case '4':  arrayNum = 4; break;      // array_4
        	case '5':  arrayNum = 5; break;      // array_5
        	case '6':  arrayNum = 6; break;      // array_6
        	case '7':  arrayNum = 7; break;      // array_7
        	case '8':  arrayNum = 8; break;      // array_8
        	case '9':  arrayNum = 9; break;      // array_9
        	case 'A':  arrayNum = 10; break;      // array_A
        	case 'B':  arrayNum = 11; break;      // array_B
        	case 'C':  arrayNum = 12; break;      // array_C
        	case 'D':  arrayNum = 13; break;      // array_D
        	case '*':  arrayNum = 14; break;      // array_Star
        	case '#':  arrayNum = 15; break;      // array_Pound
        	default:  arrayNum = 16; break;       // array_ALL0
	    }
            break;
        default:
            state = numPadS1;
            break;
    }
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
    task1.TickFct = &getNumPadInput;
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
