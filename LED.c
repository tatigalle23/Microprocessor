#include "msp.h"
#include "Clock.h"
#include <stdio.h>


void main(void)
{
    int i;    
	Clock_Init48MHz();

	//LED init
	P2->SEL0 &= ~0x07;
	P2->SEL1 &= ~0x07;
	P2->DIR |= 0x07;
	P2->OUT &= ~0x07;
		
	i=1;
	while(1){
	    P2->OUT =i;
	    i<<=1;
		//b because we are working with binaries
	    if(i == 0b1000)i=1;
	    Clock_Delay1ms(1000);
	}

}
