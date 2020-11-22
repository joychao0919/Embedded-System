#include msp430.h 
#define LED1 BIT0 red
#define LED2 BIT6 green
#define B1 BIT3 button

#define GREEN_ON 0
#define RED_ON 1
#define BOTH_ON 2
#define BOTH_OFF 3

#define PRESSED 4
#define TWO_SEC 5
#define UNPRESSED 6

int main(void)
{
	WDTCTL = WDTPW  WDTHOLD;	 stop watchdog timer
	P1OUT = ~(LED1LED2);
	P1DIR = LED1LED2;
	P1REN = B1;
	P1OUT = B1;

	TA0CTL = MC_1ID_0TASSEL_1TACLR; Setup Timer_A
    BCSCTL3 = LFXT1S_2; Enable VLO as MCLKACLK src

	TA1CTL = MC_1ID_3TASSEL_2TACLR; Setup Timer1_A3 up mode  divide by 8  SMCLK ; 1375000
	BCSCTL2 =  DIVS_3;  = 17187  2 = by DCO. default  by DCOclock.
	
	int state;
	state = GREEN_ON;

	int event;
	event = UNPRESSED;

	int flag = 0;

	for (;;) {  Loop forever
	    switch(state){
	    case GREEN_ON
	        switch(event){
	          case PRESSED
	              if(flag==0){
	                  TA1CTL &=~TAIFG;
	                  TA1CTL = TACLR;
	                  TA1CCR0 = 34374;
	                  flag = 1;
	              }

	              if((P1IN & B1)==0 ){
	                      if(!(TA1CTL & TAIFG)){
	                          TA0CCR0= 8399;
	                          P1OUT ^= LED2;
	                          while (!(TA0CTL & TAIFG)) {
	                          }
	                          TA0CTL &= ~TAIFG;
	                          P1OUT ^= LED2;
	                          state = RED_ON;
	                          event = PRESSED;
	                      }
	                      else{
	                          TA1CTL &= ~TAIFG;
	                          if((P1IN & B1)==0){
	                          state = BOTH_ON;
	                          event = TWO_SEC;
	                          break;
	                          }
	                      }
	                  }
	              else {
	                  flag = 0;
	                  state = RED_ON;
	                  event = UNPRESSED;
	                  break;
	              }
	              break;
	          case TWO_SEC
	               state = BOTH_ON;
	               break;
	          case UNPRESSED
	              if((P1IN & B1) ==0){
	                  state = RED_ON;
	                  event = PRESSED;
	              }
	              TA0CCR0= 8399;
	              P1OUT ^= LED2;
	              while (!(TA0CTL & TAIFG)) {
	              }
	              TA0CTL &= ~TAIFG;
	              P1OUT ^= LED2;
	              state = RED_ON;
	              break;
	          default

	          }
	          break;
	      case RED_ON
	          switch(event){
	          case PRESSEDWait for two second.
	              if(flag==0){
	                  TA1CTL &=~TAIFG;
	                  TA1CTL = TACLR;
	                  TA1CCR0 = 34374;
	                  flag = 1;
	              }

	              if((P1IN & B1)==0 ){
	                  start to count for 2 sec.
	                      if(!(TA1CTL & TAIFG)){
	                          TA0CCR0 = 3599;
	                          P1OUT ^= LED1;
	                          while (!(TA0CTL & TAIFG)) {  Wait overflow
	                          } CPU polling and doing nothing
	                          TA0CTL &= ~TAIFG;  Clear overflow flag
	                          P1OUT ^= LED1; Toggle LEDs
	                          state = GREEN_ON;
	                          event = PRESSED;
	                      }
	                      else{
	                          TA1CTL &= ~TAIFG;
	                          if((P1IN & B1)==0){
	                          state = BOTH_ON;
	                          event = TWO_SEC;
	                          break;
	                          }
	                      }
	                  }
	              else{
	                  flag = 0;
	                  state = GREEN_ON;
	                  event = UNPRESSED;
	                  break;
	              }
	              break;
	          case TWO_SECState = BOTH_ON.
	              state = BOTH_ON;
	              break;
	          case UNPRESSED equal to default.
	              flash red LED2
	              if((P1IN & B1) ==0){  Yes, event =PRESSED.
	                  state = GREEN_ON;
	                  event = PRESSED;

	              }
	              else{
	              TA0CCR0 = 3599;
	              P1OUT ^= LED1;
	              while (!(TA0CTL & TAIFG)) {  Wait overflow
	              } CPU polling and doing nothing
	              TA0CTL &= ~TAIFG;  Clear overflow flag
	              P1OUT ^= LED1; Toggle LEDs
	              state = GREEN_ON;
	              }
	              break;
	          default

	          }
	          break;
	      case BOTH_ON
	          flag = 0;
	          if((P1IN & B1)==0){
	              Now do the execution.
	              TA0CCR0= 8399;
	              P1OUT = LED1LED2;
	              while(!(TA0CTL & TAIFG)){}
	              TA0CTL &= ~TAIFG;Clear overflow flag.
	              state = BOTH_OFF;
	              P1OUT ^= LED1LED2;
	          }
	          else{
	             state = GREEN_ON;
	          }
	          break;

	      case BOTH_OFF
	          flag = 0;
	          if((P1IN & B1)==0){
	              just count off 0.3sec.
	                 TA0CCR0 = 3599;
	                 while(!(TA0CTL & TAIFG)){}
	                 TA0CTL &= ~TAIFG;Clear overflow flag.
	                 state = BOTH_ON;

	          }
	          else{UNPRESSED.
	              event = UNPRESSED;
	              state = GREEN_ON;
	              break;
	          }
	          break;
	      }

	      Events  PRESSED  TWO_SEC  UNPRESSED.
	      default  basic1.

	    }  Back around infinite loop
	}
