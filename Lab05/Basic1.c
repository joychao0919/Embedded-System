#include <msp430.h>

#define LED_RED BIT0
#define LED_GREEN BIT6
#define GREEN 0
#define RED 1
#define NORMAL 2
#define EMERGENCY 3
#define ON 4
#define OFF 5

volatile unsigned int count;
volatile unsigned int state;
volatile unsigned int event;
volatile unsigned int is_high;
volatile float lpm0 = 0;
volatile float lpm3 = 0;
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
    TA1CCR0 = 7200; // 0.1s
    TA0CCR0 = 14400 - 1; // 1.2s
    is_high = 0;
    state = NORMAL;
    event = RED;
    count = 0;

    __enable_interrupt();
    __bis_SR_register(LPM3_bits + GIE);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){
    ADC10CTL0 |= ENC + ADC10SC;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){
    count++;
    switch(state){
    case NORMAL:
        //TA1CCR0 = 1200 - 1; // 0.1s
        TA0CCR0 = 14400 - 1; // 1.2s
        if(event==GREEN){
            TA1CCR0 = 4800;
            P1OUT |= LED_RED;
            P1OUT &=~ LED_GREEN;     // Toggle LEDs
            event = RED;
            count = 0;
            lpm3 += 0.4;
        }
        else if(event==RED){
            TA1CCR0 = 7200;
            P1OUT |= LED_GREEN;
            P1OUT &=~ LED_RED;
            event = GREEN;
            count = 0;
            lpm3 += 0.6;
        }
        break;
    case EMERGENCY:
        TA0CCR0 = 4800 - 1; // 0.4s

        if(event==ON){
            TA1CCR0 = 2400;
            P1OUT &=~ BIT0;
            P1OUT &=~ BIT6;
            event = OFF;
            count = 0;
            lpm0 += 0.2;
        }
        else if(event==OFF){
            TA1CCR0 = 3600;
            P1OUT |= BIT0;
            P1OUT |= BIT6;
            event = ON;
            count = 0;
            lpm0 += 0.3;
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
    if (ADC10MEM > 740){  // ADC10MEM = A1 > 740?
        if(state==NORMAL){
            state = EMERGENCY;
            event = ON;
            count = 0;
            _BIC_SR(LPM3_EXIT);
            __bis_SR_register(LPM0_bits + GIE);
        }
    }
    else{
        if(state==EMERGENCY){
            state = NORMAL;
            event = RED;
            count = 0;
            _BIC_SR(LPM0_EXIT);
            __bis_SR_register(LPM3_bits + GIE);
        }
    }
}

