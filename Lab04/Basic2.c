#include <msp430.h>
#include <intrinsics.h>

#define LED_RED BIT0
#define LED_GREEN BIT6
#define BTN BIT3

#define RED_GREEN 0
#define BOTH 1
#define PATTERN3 2

#define RED 3
#define GREEN 4
#define NORMAL_PRESS 5

#define ON 6
#define OFF 7
#define BOTH_PRESS 8

#define PATTERN3_PRESS 10

#define MEASURE 11
#define MEASURE_PRESS 12

volatile unsigned int state;
volatile unsigned int event;
volatile unsigned int mode;

volatile unsigned int count;
volatile unsigned int record;
volatile unsigned int is_push;
volatile unsigned int is_sensor;
volatile int is_high, is_high_past;
int adc[5];
int sample = 0;



volatile int avg = 0;

void main(void){
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE + REFON + SREF_1; //Select reference.
    ADC10CTL0 |= REFON;
    ADC10CTL0 &= ~REF2_5V;
    ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;    // Input from Temperature. //SHS_1(01) TimerA: OUT1.
    ADC10AE0 |= 0x10; // Enable Temperature for analog in
    P1DIR = BIT0|BIT6;    // Set P1.0 to output
    ADC10CTL0 &= ~(ENC + ADC10SC); // Nor enable samplinf. ( 1 : Start sampling)

    ADC10DTC1 = 5;
    ADC10SA = (int)adc;

    P1OUT |= BIT3;
    P1REN = BIT3; //Set button.
    P1IE |= BIT3;                    // P1.3 interrupt enabled
    P1IES |= BIT3;                  // P1.3 Hi/lo edge 1->Hi to low.
    P1IFG &= ~BIT3;

    TA0CCR0 = 3435;     // 0.2sec
    TA0CCTL1 = OUTMOD_3;  // TA0CCR1 set/reset
    TA0CCR1 = 3434;       // TA0CCR1 OUT1 on time
    TA0CTL = MC_1|ID_3|TASSEL_2|TACLR; //Setup Timer1_A3 //up mode | divide by 8 | SMCLK ; 1375000 1.1MHZ
    BCSCTL2 |=  DIVS_3; // = 17187by DCO. default : by DCOclock.
    BCSCTL1 = CALBC1_1MHZ;               // Set range
    DCOCTL = CALDCO_1MHZ;

    P1OUT = ~(BIT0|BIT6);
    P1OUT |= BIT0;

    //timer1
    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR; // Up mode, divide clock by 8, clock from ACLK, clear
    BCSCTL3 |= LFXT1S_2;
    TA1CTL &= ~TAIFG;
    TA1CCTL0 |= CCIE; // Enable interrupts
    TA1CCR0 = 1199; //0.1sec

    // enable interrupt
    __enable_interrupt();
    // _BIS_SR(GIE);

    // start
    state = RED_GREEN;
    event = RED;

    count = 0;
    record = 0;

    for(;;){}
}

volatile unsigned int tem;
volatile float Voltage;
volatile float Celsius;
// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){

    avg = (adc[0] + adc[1] + adc[2] + adc[3] + adc[4])/5;
    Voltage = (avg*1.5)/1023;
    Celsius = (Voltage - 0.986) / 0.00355;

    if(avg>735){
        event = PATTERN3;
        P1OUT |= BIT0;
        P1OUT &= ~BIT6;
    }
    else{
        sample++;
        event = BOTH;
        P1OUT |= BIT0;
        P1OUT |= BIT6;
    }

    ADC10SA = (int)adc;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){
    count++;
    switch(state){
    case RED_GREEN:
        ADC10CTL0 &= ~(ENC + ADC10SC); // start sampling
        if(event==RED && count==7){
            P1OUT |= BIT6;
            P1OUT &= ~BIT0;
            event = GREEN;
            count = 0;
        }
        else if(event==GREEN && count==3){
            P1OUT |= BIT0;
            P1OUT &= ~BIT6;
            event = RED;
            count = 0;
        }
        state = RED_GREEN;
    break;

    case NORMAL_PRESS:
        record++;
        if(record>=30){
            count = 0;
            record = 0;
            mode = ON;
            state = MEASURE;
            P1OUT |= BIT0;
            P1OUT &= ~BIT6;
            P1IES |= BIT3;
            P1IFG &= ~BIT3;
            break;
        }
        else if(event==RED && count%10==7){
            P1OUT |= BIT6;
            P1OUT &= ~BIT0;
            event = GREEN;
            count = 0;
        }
        else if(event==GREEN && count%10==3){
            P1OUT |= BIT0;
            P1OUT &= ~BIT6;
            event = RED;
            count = 0;
        }
        state = NORMAL_PRESS;
    break;

    case MEASURE:
        ADC10CTL0 |= ENC + ADC10SC;
        if(event==BOTH){
            if(count>5){
                count = 5;
            }
            if(count==5 && mode==ON){
                P1OUT &= ~BIT0;
                P1OUT &= ~BIT6;
                mode = OFF;
                count = 0;
            }
            else if(count==5 && mode==OFF){
                P1OUT |= BIT0;
                P1OUT |= BIT6;
                mode = ON;
                count = 0;
            }
        }
        if(event==PATTERN3){
            if(count>3){
                count = 3;
            }
            if(count==3 && mode==ON){
                P1OUT &= ~BIT6;
                P1OUT &= ~BIT0;
                mode = OFF;
                count = 0;
            }
            else if(count==3 && mode==OFF){
                sample++;
                P1OUT &= ~BIT6;
                P1OUT |= BIT0;
                mode = ON;
                count = 0;
            }
        }
        state = MEASURE;
    break;

    case MEASURE_PRESS:
        record++;
        if(record>=20){
            state = RED_GREEN;
            event = RED;
            P1IES |= BIT3;
            P1IFG &= ~BIT3;
            count = 0;
            record = 0;
            break;
        }

        else if(event==BOTH){
            if(count>5){
                count = 5;
            }
            if(count==5 && mode==ON){
                P1OUT &= ~BIT0;
                P1OUT &= ~BIT6;
                mode = OFF;
                count = 0;
            }
            else if(count==5 && mode==OFF){
                P1OUT |= BIT0;
                P1OUT |= BIT6;
                mode = ON;
                count = 0;
            }
        }
        else if(event==PATTERN3){
            if(count>3){
                count = 3;
            }
            if(count==3 && mode==ON){
                P1OUT &= ~BIT6;
                P1OUT &= ~BIT0;
                mode = OFF;
                count = 0;
            }
            else if(count==3 && mode==OFF){
                P1OUT &= ~BIT6;
                P1OUT |= BIT0;
                mode = ON;
                count = 0;
            }
        }

        state = MEASURE_PRESS;
    break;
    }
    TA1CCTL0 &= ~CCIFG; //Clear interrupt flag.
    TA1CCR0 = 1199;
    TA1CTL &= ~TAIFG;
    TA1CTL |= TACLR;
}


#pragma vector = PORT1_VECTOR //button press
__interrupt void Port_1(void) {
    switch(state){
    case RED_GREEN:
        record = 0;
        state = NORMAL_PRESS;
        P1IES &= ~BIT3;
        P1IFG &= ~BIT3;
    break;

    case NORMAL_PRESS:
        state = RED_GREEN;
        event = RED;
        P1IES |= BIT3;
        P1IFG &= ~BIT3;
    break;

    case MEASURE:
        record = 0;
        state = MEASURE_PRESS;
        P1IES &= ~BIT3;
        P1IFG &= ~BIT3;
    break;

    case MEASURE_PRESS:
        state = MEASURE;
        P1IES |= BIT3;
        P1IFG &= ~BIT3;
    break;
    }
}

