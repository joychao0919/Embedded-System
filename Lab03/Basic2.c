#include <msp430.h>
#include <intrinsics.h>// Intrinsic functions
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3

#define green_red 0
#define both 1
#define redON_OFF 2 //red on 0.3, off 0.2.
#define green_red_pressing 3

int state;
int count;
int record; //record time.

void main(void) {
  WDTCTL = WDTPW|WDTHOLD; // Stop watchdog timer
  P1OUT = ~(LED1|LED2);
  P1OUT |= B1;
  P1DIR = LED1|LED2;
  P1REN = BIT3; //Set button.
  P1IE |= BIT3;                    // P1.3 interrupt enabled
  P1IES |= BIT3;                  // P1.3 Hi/lo edge 1->Hi to low.
  P1IFG &= ~BIT3;               // P1.3 IFG cleared

  //initialize green LED ON.
  P1OUT |= LED2;

  TA0CCR0 = 1199; // Upper limit of count for TA0R CPU get interrupt every 0.1 sec.
  TA0CTL = MC_1|ID_0|TASSEL_1|TACLR;
  TA0CTL &= ~TAIFG;
  // Up mode, divide clock by 8, clock from ACLK, clear
  TA0CCTL0 |= CCIE; // Enable interrupts

  //Set the counting timer.
  TA1CTL = MC_1|ID_3|TASSEL_2|TACLR; //Setup Timer1_A3 //up mode | divide by 8 | SMCLK ; 1375000
  BCSCTL2 |=  DIVS_3; // = 17187Hz(one second)= by DCO. default : by DCOclock.
  TA1CCR0 =1199;//Timer1 interrupt every 0.1 sec.
  //TA1CCTL0 |= CCIE;

  count = 0;
  record = 0;
  state = green_red;// initial state. (Pattern 1)
  __enable_interrupt(); // Enable interrupts (intrinsic)
  for (;;) { } // Loop forever doing nothing
}
// Interrupt service routine for CCR0 of Timer0_A3
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){
    count++; //every 0.1 sec CPU check inside.
    //use states.
    switch(state){
        case green_red: //flash 0.7 then to red_on 0.3;

            if(count == 7){ //0.7sec, turn off green and turn on red.
                P1OUT &= ~LED2;
                P1OUT |= LED1;
            }
            if(count ==10){
                P1OUT &= ~LED1;
                P1OUT |= LED2;
                count = 0; //reset count.
            }
            state = green_red;
        break;

        case both:
            if(record!=0){
                record--;
                if(count == 7){ //0.7sec, turn off green and turn on red.
                P1OUT &= ~(LED2|LED1);
                }
                 if(count ==10){
                P1OUT |= (LED1|LED2);
                count = 0;
                 }
                state = both;
            }
            else{
                state = green_red;
            }
        break;
        case redON_OFF:
            P1OUT &= ~LED1;
            P1OUT &= ~LED2;//
            if(record!=0){
                record--;
                if(count == 3){
                     P1OUT |= LED1;

                 }
                 if(count ==5){
                     P1OUT &= ~LED1;
                     count = 0;
                 }
                 state = redON_OFF;
            }
            else{//record ==0
                state = green_red;
            }
        break;
        case green_red_pressing:
            record ++;
            if(count ==7){ //0.7sec, turn off green and turn on red.
                P1OUT &= ~LED2;
                P1OUT |= LED1;
            }
            if(count ==10){
                P1OUT &= ~LED1;
                P1OUT |= LED2;
                count = 0; //reset count.
            }
        break;
    }
    TA0CCTL0 &= ~CCIFG; //Clear interrupt flag.
    TA0CCR0 = 1199;
    TA0CTL &= ~TAIFG;
}
#pragma vector = PORT1_VECTOR //button press.
__interrupt void Port_1(void){

    //if button down -> both on
    switch(state){
    case green_red:
        //if press-> record time.
        record = 0;//reset record time.
        record ++;
       //Start record, not untile release-> keep pattern one.
        state = green_red_pressing;
       P1IES &= ~B1; //0 : low to high.
       P1IFG &= ~B1;
    break;
    case green_red_pressing:
        //if release(0->1 low to High (0)-> Now high to low detect.
        record ++;
       //Start record, not untile release-> keep pattern one.
       if(record <=50){//short pattern.
           count = 0;
           state = both;
       }
       else{//long pattern.
           count = 0;
           state = redON_OFF;
       }  //still pressing.
       P1IES |= B1; //1 :H to L
       P1IFG &= ~B1;
    break;

    case both: //originally high to low; now -> Low to high.
        P1IES |= B1; //1 : High to low.
        P1IFG &= ~B1;
            //TA0CCR0 = 8599;
        break;
    case redON_OFF:

    break;
    }
}


