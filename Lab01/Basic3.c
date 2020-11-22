#include <msp430.h>

int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
   P1DIR |= 0x41;        // Set P1.0 as output

   P1DIR &=~ BIT3;
   P1REN |= BIT3;
   P1OUT |= BIT3;

   volatile unsigned int j;
   j = 0;

   volatile unsigned int flag;
   flag = 1;

   for(;;) {

      volatile unsigned int i;  // prevent optimization

      if(flag==1) {
          P1OUT ^= 0x41;
      }
      else {
          j %= 8;
          if(j>=6) {
              P1OUT ^= 0x40;
          }
          else {
              P1OUT ^= 0x01;
          }
          j++;
      }

      if((P1IN & BIT3) ==0){  //Is button down?
          if(flag!=1) {
              P1OUT &= 0xBE; //initialize
              flag = 1;
          }

      }
      else{
          if(flag!=0) {
              P1OUT &= 0xBE;
              flag = 0;
              j = 0;
          }

      } // No, turn red LED on


      i = 20000;         // SW Delay
      do i--;
      while(i != 0);

   }
   return 0;
}

