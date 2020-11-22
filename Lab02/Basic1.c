#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6

void main (void) {
  WDTCTL = WDTPW|WDTHOLD; // Stop watchdog timer

  volatile unsigned int i;
  volatile unsigned int is_green;

  P1OUT = ~LED1;
  P1DIR = LED1 | LED2;
  is_green = 1;

  TA0CCR0 = 1199;
  //up mode, use SMCLK, clr timer
  TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
  BCSCTL3 |= LFXT1S_2;



  for (;;) { // Loop forever
    while (!(TA0CTL & TAIFG)) {} // Wait for time up
    i++;
    if(is_green==0 && i==3){
        P1OUT ^= LED1 | LED2;     // Toggle LEDs
        is_green = 1;
        i = 0;
    }
    else if(is_green==1 && i==7){
        P1OUT ^= LED1 | LED2;     // Toggle LEDs
        is_green = 0;
        i = 0;
    }
    TA0CTL &= ~TAIFG;  // Clear overflow flag

  } // Back around infinite loop
}

