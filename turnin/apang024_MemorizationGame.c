/*	Author: Ashley Pang
 *	Lab Section: 021
 *	Assignment: Lab #11
 *	Exercise Description: Memorization Game 
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtu.be/Pp2Gut2hblo
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "keypad.h"
#include "scheduler.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#endif

void set_PWM(double frequency) {
        static double current_frequency;
        if (frequency != current_frequency) {
                if (!frequency) { TCCR3B &= 0x08; } // stops timer/counter
                else { TCCR3B |= 0x03; }            // resumes/continues timer/counter

                // Prevents OCR3A from overflowing, 0x954 is smallest freq
                if (frequency < 0.954) { OCR3A = 0xFFFF; }

                // Prevents OCR3A from underflowing, 32150 largest freq
                else if (frequency > 31250) { OCR3A = 0x0000; }

                // Set OCR3A based on desired freq
                else { OCR3A = (short) (8000000 / (128 * frequency)) - 1; }

                TCNT3 = 0;      // reset counter
                current_frequency = frequency;  // updates current freq
        }
}

void PWM_on() {
        TCCR3A = (1 << COM3A0);
        TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
        set_PWM(0);
}

void PWM_off() {
        TCCR3A = 0x00;
        TCCR3B = 0x00;
}

unsigned int arrayNum = 16;
unsigned char array_ALL0[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 	// 16
unsigned char array_0[8] = { 0x3C, 0x42, 0x46, 0x4A, 0x52, 0x62, 0x42, 0x3C };		// 0
unsigned char array_1[8] = { 0x08, 0x18, 0x28, 0x08, 0x08, 0x08, 0x08, 0x3C };		// 1
unsigned char array_2[8] = { 0x38, 0x44, 0x44, 0x04, 0x7C, 0x40, 0x40, 0x7C };		// 2
unsigned char array_3[8] = { 0x3C, 0x04, 0x04, 0x04, 0x3C, 0x04, 0x04, 0x3C };		// 3
unsigned char array_4[8] = { 0x44, 0x44, 0x44, 0x44, 0x7E, 0x04, 0x04, 0x04 };		// 4
unsigned char array_5[8] = { 0x7C, 0x40, 0x40, 0x40, 0x7C, 0x04, 0x04, 0x7C };		// 5
unsigned char array_6[8] = { 0x7C, 0x44, 0x40, 0x40, 0x7C, 0x44, 0x44, 0x7C };		// 6
unsigned char array_7[8] = { 0x3C, 0x44, 0x44, 0x04, 0x04, 0x04, 0x04, 0x04 };		// 7
unsigned char array_8[8] = { 0x7C, 0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x7C };		// 8
unsigned char array_9[8] = { 0x7C, 0x44, 0x44, 0x7C, 0x04, 0x04, 0x44, 0x7C };		// 9
unsigned char array_A[8] = { 0x10, 0x28, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44 };		// 10
unsigned char array_B[8] = { 0x7C, 0x42, 0x42, 0x7C, 0x44, 0x42, 0x42, 0x7C };		// 11
unsigned char array_C[8] = { 0x38, 0x44, 0x40, 0x40, 0x40, 0x40, 0x44, 0x38 }; 		// 12
unsigned char array_D[8] = { 0x78, 0x44, 0x42, 0x42, 0x42, 0x42, 0x44, 0x78 };		// 13
unsigned char array_star[8] = { 0x00, 0x92, 0x54, 0x38, 0xFE, 0x38, 0x54, 0x92 };	// 14
unsigned char array_pound[8] = { 0x24, 0x24, 0xFF, 0x24, 0x24, 0xFF, 0x24, 0x24 };	// 15
unsigned char array_play[8] = { 0xE8, 0xA8, 0xEE, 0x80, 0x25, 0x55, 0x72, 0x52 };	// 17
unsigned char array_swoosh[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };	// 18
unsigned char array_smile[8] = { 0x00, 0x24, 0x24, 0x24, 0x00, 0x42, 0x3C, 0x00 };	// 19
unsigned char array_frown[8] = { 0x00, 0x24, 0x24, 0x24, 0x00, 0x3C, 0x42, 0x00 };	// 20

int LED_Red_Blink = 0;
int LED_Green = 0;
int LED_Flash = 0;
int button = 0;
int read_input = 0;
unsigned char tempBlink = 0x00;
unsigned char tempBlink1 = 0x00;
unsigned char tempBlink2 = 0x00;

enum numPadStates { numPadS1, wait_Read };

int getNumPadInput(int state) {
    unsigned char x;
    switch (state) {
	case numPadS1:
	    if (read_input) {
	    	state = numPadS1;
	    }
	    else {
	    	state = wait_Read;
	    }
	    break;
	case wait_Read:
            if (read_input) {
                state = numPadS1;
            }
            else {
                state = wait_Read;
            }
            break;
	default:
	    state = wait_Read;
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
	case wait_Read: break;
        default:
            break;
    }
    return state;
}

enum LEDMatrixStates { shiftRow };

unsigned char currArray[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int LEDMatrix(int state) {
    static unsigned char i = 0;
    static unsigned char pattern;
    static unsigned char row = 0x01;

    switch (arrayNum) {
    	case 0: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_0[j]; } break;
	case 1: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_1[j]; } break;
	case 2: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_2[j]; } break;
	case 3: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_3[j]; } break;
	case 4: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_4[j]; } break;
        case 5: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_5[j]; } break;
        case 6: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_6[j]; } break;
	case 7: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_7[j]; } break;
        case 8: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_8[j]; } break;
        case 9: for(unsigned int j = 0; j < 8; j++) { currArray[j] =  array_9[j]; } break;
	case 10: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_A[j]; } break;
        case 11: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_B[j]; } break;
        case 12: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_C[j]; } break;
        case 13: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_D[j]; } break;
        case 14: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_star[j]; } break;
        case 15: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_pound[j]; } break;
        case 16: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_ALL0[j]; } break;
        case 17: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_play[j]; } break;
        case 18: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_swoosh[j]; } break;
	case 19: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_smile[j]; } break;
	case 20: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_frown[j]; } break;
	default: for(unsigned int j = 0; j < 8; j++) { currArray[j] = array_ALL0[j]; } break;
    }

    switch (state) {
        case shiftRow: break;
        default: state = shiftRow; break;
    }
    switch (state) {
        case shiftRow:
            if (i == 8) {       // Total 8 rows
                i = 0;
                row = 0x01;
            }
            else {
                pattern = currArray[i++];
                PORTC = ~pattern;
                PORTD = row;
                row <<= 1;
            }
            break;
        default: break;
    }
    return state;
}

enum LED_Red_BlinkStates { B_init, B3On, B3Off};

int LED_Red_BlinkSM(int state) {

    switch(state) {
        case B_init:
	    if (LED_Red_Blink) {
	    	state = B3On;
	    }
	    else {
	    	state = B3Off;
	    }
            break;
        case B3On:
	    state = B3Off;
            break;
	case B3Off:
	    if (LED_Red_Blink) {
                state = B3On;
            }
            else {
                state = B3Off;
            }
	    break;
	default: state = B_init;
    }
    switch(state) {
        case B_init:
	    tempBlink = 0x00;
            break;
        case B3On:
	    tempBlink = 0x20;
            break;
        case B3Off:
	    tempBlink = 0x00;
            break;
	default: break;
    }

    PORTB = (PORTB & 0xDF) | tempBlink | tempBlink2;
    return state;
}

enum LED_GreenStates { G_increment };

int LED_GreenIncrement(int state) {

    switch(state) {
        case G_increment:
            state = G_increment;
            break;
    	default: state = G_increment;
    }
    switch(state) {
        case G_increment:
            switch (LED_Green) {
		case 0: tempBlink1 = 0x00; break;
		case 1: tempBlink1 = 0x01; break;
		case 2: tempBlink1 = 0x03; break;
		case 3: tempBlink1 = 0x07; break;
		case 4: tempBlink1 = 0x0F; break;
		case 5: tempBlink1 = 0x1F; break;
		default: tempBlink1 = 0x00; LED_Green = 0; break;
	    }
            break;
	default: break;
    }

    PORTB = (PORTB & 0xE0) | tempBlink1 | tempBlink2;
    return state;
}

enum buttonState { button_press, button_release };

int buttonSM (int state) {
    unsigned char tempB7 = ~PINB & 0x80;
    switch(state) {
    	case button_press:
	    if (tempB7) {
	    	state = button_release;
		// LED_Green++;
	    }
	    else {
	    	state = button_press;
	    }
	    break;
	case button_release:
	    if (tempB7) {
                state = button_release;
            }
            else {
                state = button_press;
            }
	    break;
	default: state = button_release; break;
    }
    switch(state) {
        case button_press: button = 0; break;
        case button_release: button = 1; break;
        default: break;
    }
    return state;
}

enum FlashLightsStates { seq, wait };

int flash_lights(int state) {
    unsigned char pattern[] = { 0x21, 0x12, 0x0C, 0x12, 0x21};
    unsigned int numPattern = 6;
    static int j = 0;

    switch (state) {
	case seq:
	    if (LED_Flash) {
	    	state = seq;
	    }
	    else {
	    	state = wait;
	    }
	    break;
	case wait:
	    if (LED_Flash) {
                state = seq;
            }
            else {
                state = wait;
            }
	    break;
	default: state = wait; break;
    }
    switch (state) {
    	case seq:
	    tempBlink2 = pattern[j++];
	    if (j >= numPattern) { j = 0; }
	    PORTB = (PORTB & 0xC0) | tempBlink | tempBlink1 | tempBlink2;
	    break;
	case wait:
	    tempBlink2 = 0x00;
	    PORTB = (PORTB & 0xC0) | tempBlink | tempBlink1 | tempBlink2;
	    break;
	default: break;
    }
    return state;
}

enum buzzerStates { waitC, play_C };

int buzzer(int state) {
    static unsigned int cnt = 10;

    // Transitions
    switch (state) {
	case waitC:
	    if (button && cnt > 0) {
	    	state = play_C;
	    }
	    else {
	    	state = waitC;
	    }
	    break;
	case play_C:
	    if (cnt > 0) {
	    	state = play_C;
	    }
	    else {
	    	state = waitC;
	    }
	    break;
	default: state = waitC; break;
    }
    // Actions
    switch (state) {
        case waitC:
	    if (!button) { cnt = 10; }
	    set_PWM(0);
            break;
        case play_C:
	    set_PWM(261.63);
	    cnt--;
            break;
	default: break;
    }
    return state;
}

enum GameStates { start, wait1, play, LED_off, read, read_wait, correctInput, wrongInput, WIN, reset_wait };
unsigned int array_memory[] = { 16, 16, 16, 16, 16 };

int gameSM(int state) {
    static unsigned int cnt = 3;
    static unsigned int k = 0; 
    static unsigned int y = 0;
    // Transitions
    switch (state) {
    	case start:
	    if (button) { state = wait1; }
	    else { state = start; }
	    break;
	case wait1:
	    if (button || cnt > 0) { state = wait1; }
            else { state = play; }
	    break;
	case play:
	    if (button) { state = reset_wait; }
            else { state = LED_off; }
	    break;
	case LED_off:
	    if (button) { state = reset_wait; }
            else if (k < 5) { 
		state = play; 
	    }
	    else { 
	    	k = 0;
		state = read;
	    }
	    break;
	case read:
	    if (button) { state = reset_wait; }
	    else if (arrayNum != 16) {			// Check for input
		y = arrayNum;
	    	state = read_wait;
	    }
	    else { state = read; }
	    break;
	case read_wait:
	    if (button) { state = reset_wait; }
	    else if (arrayNum != 16) { state = read_wait; }
	    else if ((arrayNum == 16) && (y == array_memory[k])) { // Stop pressing keypad
	    	LED_Green++;		// Correct increase Green
		state = correctInput;
	    }
	    else { state = wrongInput; }
	    break;
	case correctInput:
	    if (button) { state = reset_wait; }
	    else if (k < 4) {
		k++;			// Mealy Action
	    	state = read;
	    }
	    else {			// K >= 5
	    	read_input = 0;		// Mealy Action
		state = WIN;
	    }
	    break;
	case wrongInput:
	    if (button) { state = reset_wait; }
	    else {
	    	read_input = 0;
		state = wrongInput;
	    }
	    break;
	case WIN:
	    if (button) { state = reset_wait; }
	    else { state = WIN; }
	    break;
	case reset_wait:
	    if (button) { state = reset_wait; }
            else { state = start; }
	    break;
	default: state = start; break;
    }
    // Actions
    switch (state) {
        case start:
	    cnt = 3;
	    arrayNum = 17;	// 17 - ARR_PLAY
	    LED_Flash = 1;
	    read_input = 0;
	    LED_Green = 0;
	    LED_Red_Blink = 0;
            break;
        case wait1:
	    k = 0;
	    cnt--;
	    arrayNum = 18;
	    LED_Flash = 0;
	    read_input = 0;
            LED_Green = 0;
            LED_Red_Blink = 0;
            break;
        case play:
	    arrayNum = rand() % 16;	// Any number from 0 to 15
	    array_memory[k] = arrayNum;
	    LED_Red_Blink = 1;
	    k++;
            break;
        case LED_off:
	    arrayNum = 16;		// Turn off display
            break;
        case read:
	    read_input = 1;
	    LED_Red_Blink = 0;
	    // y = array_memory[k];
	    // y = arrayNum;
            break;
        case read_wait:
            break;
        case correctInput:
            break;
        case wrongInput:
	    read_input = 0;
	    arrayNum = 20;		// :(
	    LED_Red_Blink = 1;
            break;
        case WIN:
	    read_input = 0;
	    arrayNum = 19;		// :)
            break;
        case reset_wait:
	    read_input = 0;
	    LED_Red_Blink = 0;
            break;
	default: break;
    }
    return state;
}

int main(void) {
    DDRA = 0xF0; PORTA = 0x0F;		// PORTA: KEYPAD all input
    DDRB = 0x7F; PORTB = 0x80;		// PORTB: B7-B6 = input, B5-B0 = output
    DDRC = 0xFF; PORTC = 0x00;		// PORTC: MATRIX COLUMNS all output
    DDRD = 0xFF; PORTD = 0x00;		// PORTD: MATRIX ROWS all output

    static task task1, task2, task3, task4, task5, task6, task7, task8;
    task* tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    const char start = -1;
    task1.state = start;
    task1.period = 20;
    task1.elapsedTime = task1.period;
    task1.TickFct = &getNumPadInput;
    task2.state = start;
    task2.period = 1;
    task2.elapsedTime = task2.period;
    task2.TickFct = &LEDMatrix;
    task3.state = start;
    task3.period = 250;
    task3.elapsedTime = task3.period;
    task3.TickFct = &LED_Red_BlinkSM;
    task4.state = start;
    task4.period = 50;
    task4.elapsedTime = task4.period;
    task4.TickFct = &LED_GreenIncrement;
    task5.state = start;
    task5.period = 20;
    task5.elapsedTime = task5.period;
    task5.TickFct = &buttonSM;
    task6.state = start;
    task6.period = 500;
    task6.elapsedTime = task6.period;
    task6.TickFct = &flash_lights;
    task7.state = start;
    task7.period = 100;
    task7.elapsedTime = task7.period;
    task7.TickFct = &buzzer;
    task8.state = start;
    task8.period = 500;
    task8.elapsedTime = task8.period;
    task8.TickFct = &gameSM;
    
    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for( i = 1; i < numTasks; i++) {
    	GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();
    PWM_on();

    while(1) {
    	for (i = 0; i < numTasks; i++) {
	    if (tasks[i]->elapsedTime >= tasks[i]->period) {
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

/* Works Cited:
 * - Benjamin Denzlier (bdenz001) - discussed how to time multiplex and created a SM to test
 * - 8x8 LED Matrix PINOUT - https://www.zpag.net/Electroniques/Arduino/8x8_dot_matrix_1588bs.html
 * - CS120B Resource Folder
 */
