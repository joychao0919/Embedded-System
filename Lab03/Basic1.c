#include <msp430.h>
#include <intrinsics.h> // Intrinsic functions
#define LED_RED BIT0
#define LED_GREEN BIT6
#define BTN BIT3
#define GREEN 1
#define RED 0

volatile unsigned int is_push;
volatile unsigned int i;
volatile unsigned int count;
volatile unsigned int state;

void main(void) {
    WDTCTL = WDTPW|WDTHOLD; // Stop watchdog timer
    P1OUT |= LED_RED;
    P1OUT &= ~LED_GREEN;
    P1DIR = LED_RED | LED_GREEN;

    // timer0
    TA0CCR0 = 1199;
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR;
    BCSCTL3 |= LFXT1S_2;
    TA0CCTL0 = CCIE;
    __enable_interrupt();

    P1OUT |= BTN;
    P1REN |= BTN;
    P1IE |= BTN;
    P1IES |= BTN;
    P1IFG &= ~BTN;
    _BIS_SR(GIE);
    is_push = 0;

    state = RED;

    for (;;) { }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){
    for (;;) { // Loop forever
        while (!(TA0CTL & TAIFG)) {} // Wait for time up
        count++;
        if((P1IN & BTN) == 0){ //push
            if(is_push == 0){ //btn change
                is_push=1;
                P1OUT ^= LED_RED;
            }
        }
        else{
            if(is_push == 1){ //btn change
                is_push=0;
                P1OUT ^= LED_RED;
            }
        }

        if(state==GREEN && count==7){
            P1OUT ^= LED_RED | LED_GREEN;     // Toggle LEDs
            state = RED;
            count = 0;
        }
        else if(state==RED && count==3){
            P1OUT ^= LED_RED | LED_GREEN;     // Toggle LEDs
            state = GREEN;
            count = 0;
        }
        TA0CTL &= ~TAIFG;  // Clear overflow flag
      }
}