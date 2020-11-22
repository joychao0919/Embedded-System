#include <msp430.h>

#define B1 BIT3

int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
   P1DIR |= 0x41;        // Set P1.0 as output
   //P1OUT = 0x40;
   P1OUT |= B1;
   P1REN = B1;

   volatile unsigned int i;
   i = 0;

   volatile unsigned int flag;
   flag = 1;

   for(;;) {
      volatile unsigned int j;  // prevent optimization

      if(i==20000){
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
                P1OUT &= 0xBE;
                flag = 1;
            }

          }
          else{
            if(flag!=0) {
                P1OUT &= 0xBE;
                flag = 0;
                j = 0;
            }

          }
          i = 0;
      }
      i++;
   }
   return 0;
}
