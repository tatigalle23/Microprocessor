#include "msp.h"
#include "Clock.h"
#include <stdio.h>

#define LED_RED 1
#define LED_GREEN (LED_RED << 1)
#define LED_BLUE (LED_RED << 2)


void led_init(){
    //set p2 ad GPIO
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;

    //INPUT OR OUTPUT
    //current is output
    P2->DIR |= 0x07;

    //Turn off LED
    P2->OUT &= ~0x07;
}

void turn_off_led(){
    P2->OUT &= ~0x07;
}
void turn_on_led(int color){
    P2->OUT &= ~0x07;
    P2->OUT |= color;
}
void switch_init(){
    //setup switch as GPIO
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;

    //setup switch as input
    P1->DIR &= ~0x12;

    //enable pull-up resistor
    P1->REN |= 0x12;

    //now pull up
    P1-> OUT |= 0x12;
}

void main(void)
{
    int sw1;   
	Clock_Init48MHz();

    //LED initialization
    led_init();

    //switch initialization
    switch_init();
    turn_off_led();

    while(1){
        sw1= P1->IN & 0x02;
        if(!sw1){
            turn_on_led(LED_BLUE);

        }
        Clock_Delay1ms(100);
        turn_off_led();
    }

}
