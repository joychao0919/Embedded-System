#include <msp430.h>

#define LED_RED BIT0
#define LED_GREEN BIT6
#define GREEN 0
#define RED 1
#define NORMAL 2
#define EMERGENCY 3
#define ON 4
#define OFF 5

volatile unsigned int j;
volatile unsigned int count;
volatile unsigned int timecnt;
volatile unsigned int normalcount;
volatile unsigned int state;
volatile unsigned int event;
volatile unsigned int is_high;
volatile unsigned int lpm0 = 0;
volatile unsigned int lpm3 = 0;
volatile unsigned int avg;
volatile float high_time;
volatile float time0;
volatile float time3;
int sample;
int adc[6];

void main(void){
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    ADC10AE0 |= 0x10; // Enable temperature for analog in

    P1DIR |= LED_RED|LED_GREEN; // setting LED as output

    ADC10CTL1 = INCH_10 + SHS_1 + CONSEQ_0;    // Input from temperature sensor
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    ADC10CTL0 |= ENC; // Start sampling

    TA0CCR0 = 14399;
    TA0CCR1 = 14399;
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR;
    TA0CCTL1 = OUTMOD_3;
    BCSCTL3 |= LFXT1S_2;
    TA0CTL &= ~TAIFG;

    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR;  // up mode, divied 1, ACLK, clr timer
    BCSCTL3 |= LFXT1S_2;  // Enable VLO as MCLK/ACLK src
    TA1CTL &= ~TAIFG;
    TA1CCTL0 |= CCIE;  // Enable interrupt
    TA1CCR0 = 9;

    __enable_interrupt();
    state = NORMAL;
    event = RED;
    count = 0;
    timecnt = 0;

    __bis_SR_register(LPM3_bits + GIE);
    for(;;);

}


volatile unsigned int tem;
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    tem = ADC10MEM;
    switch(state){
    case NORMAL:
        //time3 += 1.2;
        if(ADC10MEM > 740){
            state = EMERGENCY;
            //time0 = 0;
            //time3 = 0;
            event = ON;
            ADC10DTC1 = 6;
            ADC10SA = (int)adc;
            TA0CCR0 = 4800-1; // sampling every 0.4sec
            TA0CCR1 = 4800-2; // TACCR1 OUT1
            TA0CTL &= ~TAIFG;
            ADC10CTL1 |= SHS_1+CONSEQ_2;
            ADC10CTL0 |= ENC;
            _BIC_SR(LPM3_EXIT);
            __bis_SR_register(LPM0_bits + GIE);
        }
        else{
            TA0CTL &= ~TAIFG;
            state = NORMAL;
            event = RED;
            TA0CCR0 = 14400-1;
            TA0CCR1 = 14400-2;
            ADC10CTL0 |= ENC;
            ADC10DTC1 = 0;
        }
        break;

    case EMERGENCY:
        //time0 += 2.4;
        avg = (adc[0]+adc[1]+adc[2]+adc[3]+adc[4])/5;
        tem = ADC10MEM;
        if(avg <= 740){
            state = NORMAL;
            event = RED;
            //time0 = 0;
            //time3 = 0;
            TA0CCR0 = 14400-1;
            TA0CCR1 = 14400-2;
            ADC10CTL1 &= ~CONSEQ_2;
            ADC10CTL0 |= ENC;
            ADC10CTL1 = 0;
            _BIC_SR(LPM0_EXIT);
            __bis_SR_register(LPM3_bits + GIE);
        }
        else{
            state = EMERGENCY;
            event = ON;
            ADC10CTL1 = 6;
            ADC10SA = (int)adc;
            TA0CCR0 = 4800-1;
            TA0CCTL1 = OUTMOD_3;
            TA0CCR1 = 4800-2;
            TA0CTL &= ~TAIFG;
        }
        break;
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){
    switch(state){
    case NORMAL: // red 0.6s green 0.4s
        if(event==GREEN){
            TA1CCR0 = 4800;
            P1OUT |= LED_GREEN;
            P1OUT &=~ LED_RED;     // Toggle LEDs
            event = RED;
            //count = 0;
            time3 += 0.4;
        }
        else if(event==RED){
            TA1CCR0 = 7200;
            P1OUT |= LED_RED;
            P1OUT &= ~LED_GREEN;
            event = GREEN;
            //count = 0;
            time3 += 0.6;
        }
        state = NORMAL;
        break;
    case EMERGENCY:
        if(event==ON){
            TA1CCR0 = 3600;
            P1OUT |= BIT0;
            P1OUT |= BIT6;
            event = OFF;
            //count = 0;
            time0 += 0.3;
        }
        else if(event==OFF){
            TA1CCR0 = 2400;
            P1OUT &= ~BIT0;
            P1OUT &= ~BIT6;
            event = ON;
            //count = 0;
            time0 += 0.2;
        }
        state = EMERGENCY;
        break;
    }
    TA1CCTL0 &= ~CCIFG;
    TA1CTL &= ~TAIFG;
    TA1CTL |= TACLR;

    //TA0CTL &= ~TAIFG;


}


