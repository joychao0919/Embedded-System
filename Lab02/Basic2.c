#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6

void main (void) {
  WDTCTL = WDTPW|WDTHOLD; // Stop watchdog timer

  volatile unsigned int i;
  volatile unsigned int is_green;
  volatile unsigned int both_on;
  volatile unsigned int flag = 0;

  P1OUT = ~LED1;
  P1DIR = LED1 | LED2;
  is_green = 1;
  both_on = 1;

  P1DIR &=~ BIT3;
  P1REN |= BIT3;
  P1OUT |= BIT3;

  TA0CCR0 = 1199;
  //up mode, use SMCLK, clr timer
  TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
  BCSCTL3 |= LFXT1S_2;



  for (;;) { // Loop forever
    while (!(TA0CTL & TAIFG)) {} // Wait for time up

    if((P1IN & BIT3)==0) {
        if(flag!=1) {
            flag = 1;
            P1OUT &= 0xBE;
            i = 0;
            both_on = 0;
        }
    }
    else {
        if(flag!=0) {
            flag = 0;
            P1OUT &= 0xBE;
            i = 0;
            is_green = 0;
        }
    }

    if(flag==0) {
        if(is_green==0 && i==3){
            P1OUT |= BIT6;
            P1OUT &=~ BIT0;
            is_green = 1;
            i = 0;
        }
        else if(is_green==1 && i==7){
            P1OUT |= BIT0;
            P1OUT &=~ BIT6;
            is_green = 0;
            i = 0;
        }
        i++;
    }

    else {
        if(both_on==0 && i==3){
            P1OUT |= BIT6;
            P1OUT |= BIT0;
            both_on = 1;
            i = 0;
        }
        else if(both_on==1 && i==7){
            P1OUT &=~ BIT0;
            P1OUT &=~ BIT6;
            both_on = 0;
            i = 0;
        }
        i++;
    }


    TA0CTL &= ~TAIFG;  // Clear overflow flag

  } // Back around infinite loop
}

