/*
Anam
Tatiana
*/

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
uint16_t right_count;
int lap_count;

void IR_init()
{
    // 0, 2, 4, 6 IR Emitter : P5
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    // 1, 3, 5, 7 IR Emitter: P9
    P9->SEL0 &= ~0x04;
    P9->SEL1 *= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    // 0~7 IR Sensor
    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR &= ~0xFF;
}

void led_init()
{
    // set p2 ad GPIO
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;

    // INPUT OR OUTPUT
    // current is output
    P2->DIR |= 0x07;

    // Turn off LED
    P2->OUT &= ~0x07;
}

void turn_off_led()
{
    P2->OUT &= ~0x07;
}
void turn_on_led(int color)
{
    P2->OUT &= ~0x07;
    P2->OUT = color;
}

void motor_init()
{
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

    pwm_init34(7500, 0, 0);
}

void pwm_init34(uint16_t period, uint16_t duty3, uint16_t duty4)
{
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;

    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;
}

void move(uint16_t leftDuty, uint16_t rightDuty)
{
    P3->OUT |= 0xC0;
    TIMER_A0->CCR[3] = leftDuty;
    TIMER_A0->CCR[4] = rightDuty;
}

void left_forward()
{
    P5->OUT &= ~0x10;
}
void left_backward()
{
    P5->OUT |= 0X10;
}
void right_forward()
{
    P5->OUT &= ~0x20;
}
void right_backward()
{
    P5->OUT |= 0x20;
}
void (*TimerA2Task)(void);
void TimerA2_Init(void (*task)(void), uint16_t period)
{
    TimerA2Task = task;
    TIMER_A2->CTL = 0x0280;
    TIMER_A2->CCTL[0] = 0x0010;
    TIMER_A2->CCR[0] = (period - 1);
    TIMER_A2->EX0 = 0x0005;
    NVIC->IP[3] = (NVIC->IP[3] & 0xFFFFFF00) | 0x00000040;
    NVIC->ISER[0] = 0x00001000;
    TIMER_A2->CTL |= 0x0014;
}
void TA2_0_IRQHandler(void)
{
    TIMER_A2->CCTL[0] &= ~0x0001;
    (*TimerA2Task)();
}
void task()
{
    printf("Interrupt occurs!\n");
}

void timer_A3_capture_init()
{
    P10->SEL0 |= 0x30;
    P10->SEL1 &= ~0x30;
    P10->DIR &= ~0x30;

    TIMER_A3->CTL &= ~0x0030;
    TIMER_A3->CTL = 0x0200;
    TIMER_A3->CCTL[0] = 0x4910;
    TIMER_A3->CCTL[1] = 0x4910;
    TIMER_A3->EX0 &= ~0x0007;

    NVIC->IP[3] = (NVIC->IP[3] & 0x0000FFFF) | 0x404000000;
    NVIC->ISER[0] = 0x0000C000;
    TIMER_A3->CTL |= 0x0024;
}
void TA3_0_IRQHandler(void)
{
    TIMER_A3->CCTL[0] &= ~0x0001;
    period_right = TIMER_A3->CCR[0] - first_right;
    first_right = TIMER_A3->CCR[0];
}
void TA3_N_IRQHandler(void)
{
    TIMER_A3->CCTL[1] &= ~0x0001;
    left_count++;
}
uint32_t get_left_rpm()
{
    return 2000000 / period_left;
}

void main(void)
{

    //***Clock initialization
    Clock_Init48MHz();
    motor_init();
    IR_init();
    led_init();
    timer_A3_capture_init();

    //***SENSOR IMPLEMENTATION
    int sensor1, sensor4, sensor5, sensor6, sensor7, sensor8;
    int sensor2;
    int sensor3;
    uint16_t prev_left_count;
    uint16_t new_left_count;
    lap_count = 0;

    while (1)
    {
        P5->OUT |= 0x08;
        P9->OUT |= 0x04;

        P7->DIR = 0xFF;
        P7->OUT = 0xFF;
        Clock_Delay1us(10);

        P7->DIR = 0x00;
        Clock_Delay1us(90);

        Clock_Delay1us(1000);
        sensor1 = P7->IN & 0x01;
        sensor2 = P7->IN & 0x02;
        sensor3 = P7->IN & 0x04;
        sensor4 = P7->IN & 0x08;
        sensor5 = P7->IN & 0x10;
        sensor6 = P7->IN & 0x20;
        sensor7 = P7->IN & 0x40;
        sensor8 = P7->IN & 0x80;

        if (sensor4 == 8 && sensor5 == 16)
        {
            if (sensor2 == 2 && sensor3 == 4 && sensor6 == 32 && sensor7 == 64 && sensor1 == 0 && sensor8 == 0)
            {
                // STOP CONDITION
                lap_count += 1;
                if (lap_count == 1)
                {
                    turn_on_led(LED_GREEN);
                    left_forward();
                    right_forward();
                    move(0, 0);
                    Clock_Delay1us(100);
                    left_count = 0;

                    while (left_count < 90)
                    {
                        move(2000, 2000);
                    }
                    Clock_Delay1us(1000);
                }

                else if (lap_count > 1)
                {
                    turn_on_led(LED_RED);
                    move(0, 0);
                    Clock_Delay1us(5000);
                }
                Clock_Delay1us(1000);
            }
            else if (sensor3 == 4 && sensor6 == 0)
            {
                // turning right
                prev_left_count = left_count;
                new_left_count = 0;
                left_forward();
                right_backward();
                move(2000, 2000);
                if (new_left_count < 180)
                {

                    new_left_count = left_count - prev_left_count;
                }
            }
            else if (sensor3 == 4 && sensor6 == 32)
            {
                // turn right when the T part comes in the squares
                left_forward();
                right_backward();
                move(2000, 3000);
                Clock_Delay1us(1000);
            }
            else if (sensor2 == 2 && sensor1 == 1 && sensor3 == 0 && sensor6 == 0 && sensor7 == 0 && sensor8 == 0)
            {
                // Narrow Curve
                move(0, 0);
                Clock_Delay1us(1000);

                left_forward();
                right_backward();
                left_count = 0;

                while (left_count < 300)
                {
                    move(2000, 2000);
                }
                move(0, 0);
                Clock_Delay1us(1000);

                left_forward();
                right_forward();
                left_count = 0;

                while (left_count < 30)
                {
                    move(2000, 2000);
                }

                Clock_Delay1us(1000);
            }
            else
            {
                // Go Straight
                turn_off_led();
                left_forward();
                right_forward();
                move(2000, 2000);
                Clock_Delay1us(1000);
            }
        }
        else if (sensor5 == 0 && sensor4 == 8)
        {
            left_forward();
            right_backward();
            move(2000, 3000);
            Clock_Delay1us(1000);
        }
        else if (sensor4 == 0 && sensor5 == 16)
        {

            right_forward();
            left_backward();
            move(3000, 2000);
            Clock_Delay1us(1000);
        }
        else
        {
            // turn left
            prev_left_count = left_count;
            new_left_count = 0;
            left_backward();
            right_forward();
            move(2000, 2000);
            if (new_left_count < 300)
            {
                new_left_count = left_count - prev_left_count;
            }
        }

        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;

        Clock_Delay1us(1000);
    }
}