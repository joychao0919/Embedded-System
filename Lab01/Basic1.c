#include <msp430.h>

int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
   P1DIR |= 0x41;        // Set P1.0 as output

   volatile unsigned int j;
   j = 0;

   for(;;) {
      volatile unsigned int i;  // prevent optimization

      j %= 8;

      if(j>=6) {
          P1OUT ^= 0x40;
          P1OUT &=~ 0x01;
      }
      else {
          P1OUT ^= 0x01;
          P1OUT &=~ 0x40;
      }

      i = 20000;         // SW Delay
      do i--;
      while(i != 0);
      j++;
   }
   return 0;
}

