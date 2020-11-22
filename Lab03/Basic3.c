#include <msp430.h>
#include <intrinsics.h>// Intrinsic functions
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3

#define green_red 0
#define recording 1 //Check when press.
#define replay 2 //Both on-off for the same duration.
#define green_press 3
#define record_press 4

int state;
int count;
int record; //record time.
int Duration;
int press_num;
unsigned int timestamp; //index for isPress array. to record the "timestamp" when press Button.
int isPress[60]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};


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

  TA0CTL = MC_1|ID_0|TASSEL_1|TACLR;
  BCSCTL3 |= LFXT1S_2;
  TA0CTL &= ~TAIFG;
  // Up mode, divide clock by 8, clock from ACLK, clear
  TA0CCTL0 |= CCIE; // Enable interrupts
  TA0CCR0 = 1199; // Upper limit of count for TA0R CPU get interrupt every 0.1 sec.
/*
  //Set the counting timer.
  TA1CTL = MC_1|ID_3|TASSEL_2|TACLR; //Setup Timer1_A3 //up mode | divide by 8 | SMCLK ; 1375000
  BCSCTL2 |=  DIVS_3; // = 17187Hz(one second)= by DCO. default : by DCOclock.
*/
  count = 0;
  record = 0;
  Duration = 0;
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
            //if(count == 7){ //0.7sec, turn off green and turn on red.
             //   P1OUT &= ~LED2;
             //   P1OUT |= LED1;
            //}
            //if(count ==10){
                P1OUT &= ~LED1;
                P1OUT |= LED2;
                count = 0; //reset count.
            //}
            state = green_red;
        break;
        //When green_press: keep counting, if >=4 sec, automatically change to recording mode.
        case green_press: //flash 0.7 then to red_on 0.3;
            record++;//add every 0.1 sec.
            if(record>=40){//change to record mode.
                state = recording;
                timestamp = 0; //reset from 0 to count whether this second is Press.
                P1OUT |= LED1;
                P1OUT &=~LED2;
                P1IES |= B1; //1 :H to L avoid actions for releasing button.
                P1IFG &= ~B1;
                count = 0; //reset for later duration.
                record = 0;// reset for next count.
                break;
            }
            //if(count == 7){ //0.7sec, turn off green and turn on red.
            //    P1OUT &= ~LED2;
            //    P1OUT |= LED1;
            //}
            //if(count ==10){
                P1OUT &= ~LED1;
                P1OUT |= LED2;
                count = 0; //reset count.
            //}
            state = green_press;
        break;

        case recording: //in this mode: off green LED, and turn on red LED.
            //keep count the time during the whole recording mode.
        break;

        case record_press: //when pressing for 4 seconds, jump to replay.
            //During pressing, remember the timestamp.
            isPress[timestamp] = count; //record what timme we press.
            timestamp++; //if press, move to next entry of the isPress array.
            record++;
            if(record>=40){
         //       isPress[timestamp] = -1; //Clear final press signal.(It's for transitioning states, not for flashing LED.)
                Duration = count; //The duration of recording in Record mode.
                state = replay;
                count = 0;
            //    record = 0;//reset.
                timestamp = 0; //from index 0 to replay.
                P1OUT &= ~(LED1 | LED2); //close both.
            }
        break;
        case replay:
            if(count <= Duration /*isPress[timestamp]!= -1*/){//if equal to zero, break!
                if(count == isPress[timestamp]){
                    P1OUT |= (LED1|LED2);
                    isPress[timestamp] = -1;//clear timestamp.
                    timestamp++;
                }
                else{
                    P1OUT &= ~(LED1|LED2);
                }
            }
            else{
                state = green_red;
                count = 0;
                Duration = 0;
                P1IES |= B1; //High to low interrupt.
                P1IFG &= ~B1;
            }
        break;
    }
    TA0CCTL0 &= ~CCIFG; //Clear interrupt flag.
    TA0CCR0 = 1199;
    TA0CTL &= ~TAIFG;
    TA0CTL |= TACLR;
}
#pragma vector = PORT1_VECTOR //button press.
__interrupt void Port_1(void){

    //if button down -> both on
    switch(state){
    case green_red:
        //if press-> record time.
        record = 0;//reset record time.
       //Start record, not untile release-> keep pattern one.
        state = green_press;
        P1IES &= ~B1; //0 : low to high.
        P1IFG &= ~B1;
    break;
    case green_press:
        //if release(0->1 low to High (0)-> Now high to low detect.
       //if we enter this, means we release button when pressing(less than 4 sec, so go back to green_red.
       state = green_red;
       P1IES |= B1; //1 :H to L
       P1IFG &= ~B1;
    break;
    case recording:
        P1OUT |= LED2;
 /*       isPress[timestamp] = count; //record what timme we press.
        timestamp++; //if press, move to next entry of the isPress array.*/
        P1IES &= ~B1; //0:low to High detecting.
        P1IFG &= ~B1; //clear Port interrupt flag.
        state = record_press;
    break;
    case record_press: //when come in : Not pressed, so interrupt -> go back to record state.
         record = 0;//reset to 0.
         P1OUT &= ~LED2;
         P1IES |=B1; //1:High to Low.(Press button will trigger interrupt.)
         P1IFG &= ~B1;
         state = recording;
    break;
    }
}


