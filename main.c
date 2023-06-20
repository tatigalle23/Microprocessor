#include "msp.h"
#include "Clock.h"
#include <stdio.h>

#define LED_RED 1
#define LED_GREEN (LED_RED << 1)
#define LED_BLUE (LED_RED << 2)

uint16_t first_left;
uint16_t first_right;
uint16_t period_left;
uint16_t period_right;
uint16_t left_count;



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

/*void turn_on_led(){
    P2->OUT |= 0x07;
}*/
void turn_off_led(){
    P2->OUT &= ~0x07;
}
void turn_on_led(int color){
    P2->OUT &= ~0x07;
    P2->OUT = color;
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

//Systick timer
void systick_init(void){
    SysTick-> LOAD = 0x00FFFFFF;
    SysTick-> CTRL = 0x00000005;
}

void systick_wait1ms(){
    SysTick-> LOAD = 0xFFFFFFFF;
    SysTick -> VAL = 0;
    while((SysTick-> CTRL & 0x00010000)== 0){

    }
}

void systick_wait1s(){
    int i;
    int count = 1000;

    for(i=0; i<count; i++){
        systick_wait1ms();
    }
}

void motor_init(){
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;
    P3->DIR |= 0xC0;
    P3->OUT &= ~0xC0;

    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0x30;
    P5->DIR |= 0x30;
    P5->OUT &= ~0x30;

    P2->SEL0 &= ~0xC0;
    P2->SEL1 &= ~0xC0;
    P2->DIR |= 0xC0;
    P2->OUT &= ~0xC0;

    pwm_init34(7500,0,0);

}

void pwm_init34(uint16_t period, uint16_t duty3,uint16_t duty4){
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0=0x0000;

    TIMER_A0->CCTL[3]=0x0040;
    TIMER_A0->CCR[3]=duty3;
    TIMER_A0->CCTL[4]=0x0040;
    TIMER_A0->CCR[4]= duty4;

    TIMER_A0->CTL=0x02F0;

    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;
}

void move(uint16_t leftDuty, uint16_t rightDuty){
    P3->OUT |= 0xC0;
    TIMER_A0->CCR[3]= leftDuty;
    TIMER_A0->CCR[4]= rightDuty;
}

void left_forward(){
    P5->OUT &= ~0x10;
}
void left_backward(){
    P5->OUT |= 0X10;
}
void right_forward(){
    P5->OUT &= ~0x20;
}
void right_backward(){
    P5->OUT |= 0x20;
}
void (*TimerA2Task)(void);
void TimerA2_Init(void(*task)(void), uint16_t period){
    TimerA2Task= task;
    TIMER_A2->CTL=0x0280;
    TIMER_A2->CCTL[0]=0x0010;
    TIMER_A2->CCR[0]=(period-1);
    TIMER_A2->EX0=0x0005;
    NVIC->IP[3]=(NVIC->IP[3]&0xFFFFFF00)|0x00000040;
    NVIC->ISER[0]=0x00001000;
    TIMER_A2->CTL |=0x0014;
}
void TA2_0_IRQHandler(void){
    TIMER_A2->CCTL[0]&= ~0x0001;
    (*TimerA2Task)();
}
void task(){
    printf("Interrupt occurs!\n");
}

void timer_A3_capture_init(){
    P10->SEL0 |= 0x30;
    P10->SEL1 &= ~0x30;
    P10->DIR &= ~0x30;

    TIMER_A3->CTL &= ~0x0030;
    TIMER_A3->CTL = 0x0200;
    TIMER_A3->CCTL[0]= 0x4910;
    TIMER_A3->CCTL[1]= 0x4910;
    TIMER_A3->EX0 &= ~0x0007;

    NVIC->IP[3]= (NVIC->IP[3]&0x0000FFFF)|0x404000000;
    NVIC->ISER[0]= 0x0000C000;
    TIMER_A3->CTL |= 0x0024;
}
void TA3_0_IRQHandler(void){
    TIMER_A3->CCTL[0] &= ~0x0001;
    period_right= TIMER_A3->CCR[0]-first_right;
    first_right=TIMER_A3->CCR[0];
}
//void TA3_N_IRQHandler(void){
//    TIMER_A3->CCTL[1] &= ~0x0001;
//    period_left= TIMER_A3->CCR[1]-first_left;
//    first_left=TIMER_A3->CCR[1];
//}
void TA3_N_IRQHandler(void){
    TIMER_A3->CCTL[1] &= ~0x0001;
    left_count++;
}
uint32_t get_left_rpm(){
    return 2000000/period_left;
}

void main(void)
{

    //*****initialization
	Clock_Init48MHz();
	systick_init();
	motor_init();
	timer_A3_capture_init();
    left_forward();
    right_backward();
    move(1000,1000);

    while(1){
        if(left_count>60){
            move(0,0);            
        }
    };
}
