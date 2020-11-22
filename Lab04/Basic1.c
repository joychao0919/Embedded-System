#include <msp430.h>

#define LED_RED BIT0
#define LED_GREEN BIT6
#define GREEN 0
#define RED 1
#define RED_GREEN 2
#define BOTH 3
#define ON 4
#define OFF 5

volatile unsigned int count;
volatile unsigned int state;
volatile unsigned int event;
volatile unsigned int is_high;
volatile unsigned int lpm0 = 0;
volatile unsigned int lpm3 = 0;
volatile float high_time;
int sample;

void main(void){
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    P1DIR |= LED_RED | LED_GREEN; // setting LED as output

    // init timer1
    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR;  // up mode, divied 1, ACLK, clr timer
    BCSCTL3 |= LFXT1S_2;  // Enable VLO as MCLK/ACLK src
    TA1CCTL0 = CCIE;  // Enable interrupt

    // init timer0
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR;  // up mode, divied 1, ACLK, clr timer
    BCSCTL3 |= LFXT1S_2;  // Enable VLO as MCLK/ACLK src
    TA0CCTL0 = CCIE;  // Enable interrupt

    // init ADC
    // reference, sample and hold time to temperature sensor
    // reference on, ADC10 on, ADC10 iterupt enble
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
    ADC10CTL1 = INCH_10;    // Input from temperature sensor
    ADC10CTL0 |= ENC + ADC10SC; // Start sampling

    P1OUT |= LED_RED;  // LED_RED on
    P1OUT &= ~LED_GREEN;  // LED_Green off
    TA1CCR0 = 1200 - 1; // 0.1s
    TA0CCR0 = 7200 - 1; // 0.6s
    is_high = 0;
    state = RED_GREEN;
    event = RED;
    count = 0;

    __enable_interrupt();
    for(;;){}
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){
    ADC10CTL0 |= ENC + ADC10SC;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){
    count++;
    switch(state){
    case RED_GREEN:
        //lpm3++;

        if(event==GREEN && count==3){
            P1OUT |= LED_RED;
            P1OUT &=~ LED_GREEN;     // Toggle LEDs
            event = RED;
            count = 0;
        }
        else if(event==RED && count==7){
            P1OUT |= LED_GREEN;
            P1OUT &=~ LED_RED;
            event = GREEN;
            count = 0;
        }
        break;
    case BOTH:
        //lpm0++;

        if(event==ON && count==5){
            P1OUT &=~ BIT0;
            P1OUT &=~ BIT6;
            event = OFF;
            count = 0;
        }
        else if(event==OFF && count==5){
            P1OUT |= BIT0;
            P1OUT |= BIT6;
            event = ON;
            count = 0;
        }
        break;
    }

    TA0CTL &= ~TAIFG;


}

volatile unsigned int tem;
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    sample++;
    tem = ADC10MEM;
    if (ADC10MEM > 746){  // ADC10MEM = A1 > 740?
        if(state==RED_GREEN){
            state = BOTH;
            event = ON;
            count = 0;
        }
    }
    else{
        if(state==BOTH){
            state = RED_GREEN;
            event = RED;
            count = 0;
        }
    }
}

