#include "msp430.h"
#include "stdio.h"

#define UART_TXD 0x02 // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD 0x04 // RXD on P1.2 (Timer0_A.CCI1A)
#define UART_TBIT_DIV_2     (1000000 / (9600 * 2))
#define UART_TBIT           (1000000 / 9600)

#define LED_RED BIT0
#define LED_GREEN BIT6
#define LED3 0x41

#define RED 0
#define GREEN 1

#define COUNT_AVG 0
#define SEND_MSG 1
#define RECEIVE_CH 2

volatile unsigned int txData;  // UART internal TX variable
volatile unsigned char rxBuffer; // Received UART character
volatile unsigned char rem[100];
volatile int press=0, flag=0, type=0, i=0, cnt1=0, cnt2=0, state=0;
volatile unsigned int onTime=0, offTime=0, avg=0, global_interval=0, global_times=0, last_time_temp1=0, now_avg_tmp=0, num=0, print=0, finish=0, val=0, j=0;
volatile float displayCount=0;
int light;
char str[20];

volatile unsigned char outtxt[10];
volatile unsigned char intxt[15];
volatile unsigned char txtcount;

volatile int Temp[64];
volatile int tempTimes, timesCount, adc_idx;

void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);

void flash(char id, int on, int off);
void temp(int interval, int times);

volatile int isHot;
volatile int hotTimerCount;

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer

    DCOCTL = 0x00;             // Set DCOCLK to 1MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    P1OUT = 0x00;       // Initialize all GPIO
    P1SEL = UART_TXD + UART_RXD; // Use TXD/RXD pins
    P1DIR = 0xFF & ~UART_RXD; // Set pins to output

    TA1CCTL0 = CCIE; // Enable interrupts
    TA1CCR0 = 1200-1;
    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR;
    BCSCTL3 |= LFXT1S_2;  // Enable VLO as MCLK/ACLK src

    ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    TA0CCTL0 = CCIE; // Enable interrupts
    TA0CCTL1 = OUTMOD_3;  // TACCR1 set/reset
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR;
    TA0CCR0 = 1200;     // Sampling period
    TA0CCR1 = 1200-1;       // TACCR1 OUT1 on time

    P1DIR |= 0x41;       // Set P1.0 output
    int k;
    for(k=0;k<20;k++){
        str[k]='\0';
    }
    for(k=0;k<100;k++){
        rem[k]='\0';
    }
    __enable_interrupt();

    temp(1000, 2); //1Hz at least 2s
    flash('1', 500, 1500); //0,5s 1.5s
    isHot = 0;
    for(;;){
        switch(state){
            case 0:
                if(avg > 737){
                state = 1;
                print = 1;
                now_avg_tmp = avg;
                flash('0', 200, 300);
                //ADC10CTL0 &= ~(ENC + ADC10SC);
                // start UART
                //TimerA_UART_init();     // Start Timer_A UART
                //TimerA_UART_print("Hot!\r\n");
            }
            case 1:
                if(displayCount>=2){
                    displayCount = 0;
                    print = 1;
                }
                if(print==1){
                    print = 0;
                    TimerA_UART_init();
                    sprintf(str, "%d", now_avg_tmp);
                    TimerA_UART_print(str);
                    TimerA_UART_print("\r\n");
                    memset(str, 0, strlen(str));
                    finish = 1; // finish printing
                }
                if(type==1 && finish==1){ // wait for Ack!
                    TimerA_UART_tx(rxBuffer);
                    rem[i++]=rxBuffer;
                    type=0;
                    if(rem[i-1]=='!'&& rem[i-2]=='k'&&rem[i-3]=='c'&&rem[i-4]=='A'){
                        state = 0; // normal
                        flash('1', 500, 1500);
                        temp(1000, 2);
                        TimerA_UART_print("normal\r\n");
                        TA0CCTL0 = CCIE; // Enable interrupts
                        TA0CCTL1 = OUTMOD_3;  // TACCR1 set/reset
                        TA0CTL = MC_1|ID_0|TASSEL_1|TACLR;
                        avg = 0;
                        now_avg_tmp = 0;
                        finish = 0;
                        P1OUT &= ~0x41;
                        int a;
                        for(a=0; a<global_times; a++){ // reset Temp[]
                            Temp[a] = 0;
                        }
                    }
                }
            break;
        }
    }
}

/****** flash *******/
volatile unsigned int led, onTime, offTime;
void flash(char id, int on, int off){
    /*
    led = (id == RED)? LED_RED : LED_GREEN;
    onTime = on * 12;
    offTime = off * 12;

    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR;  // up mode, divied 1, ACLK, clr timer
    P1DIR |= led;
    P1OUT |= led;
    TA1CCR0 = onTime - 1;

    // Enable interrupt
    TA1CCTL0 = CCIE;
    __enable_interrupt();

    __bis_SR_register(LPM3_bits + GIE);  // sleep
    */
    on = on/100;
    off = off/100;
    if(id=='0'){
        //P1OUT |= LED_R ;
        light = LED_RED;
    }
    else{
        //P1OUT |= LED_G ;
        light = LED_GREEN;
    }
    //light = P1OUT;
    onTime = 1200*on;
    offTime = 1200*off;
    TA1CCR0 = onTime;
    __bis_SR_register(LPM3_bits + GIE);
}


#pragma vector = TIMER1_A0_VECTOR //ok
__interrupt void Timer1_A0_ISR (void){
    // handling hot
    P1OUT &= ~0x41;
    if(TA1CCR0==onTime){
        TA1CCR0 = offTime;
        P1OUT &= ~0x41;
        if(state==1){
            displayCount+=0.2;
        }
    }
    else{
        TA1CCR0 = onTime;
        P1OUT |= light;
        if(state==1){
            displayCount+=0.3;
        }
    }
    _BIC_SR(LPM3_EXIT);
  /*
    if(isHot){ // send 'hot!' message every second
        hotTimerCount++;
        if(hotTimerCount>=4){
            state = SEND_MSG;
            hotTimerCount = 0;
            __bic_SR_register_on_exit(LPM3_bits);
        }
    }

    if(TA1CCR0 == onTime-1){
        TA1CCR0 = offTime-1;
    }
    else{
        TA1CCR0 = onTime-1;
    }
    P1OUT ^= led;
    */
}

/******* temp ********/

void temp(int interval, int times){ //ok
    global_times = times;
    // init ADC
    // reference, sample and hold time to temperature sensor
    // reference on, ADC10 on, ADC10 iterupt enble
    /*
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
    ADC10CTL1 = INCH_10 + SHS_1 + CONSEQ_2;    // Input from temperature sensor
    ADC10CTL0 |= ENC + ADC10SC; // Start sampling
    */
    TA0CCR0 = interval*12;     // Sampling period
    //TA0CCTL1 = OUTMOD_3;  // TACCR1 set/reset
    TA0CCR1 = interval*12-1;       // TACCR1 OUT1 on time
    //TA0CTL = TASSEL_1 + MC_1 + TACLR;    // ACLK, up mode
    ADC10CTL0 |= ENC;
    //adc_idx = 0;
    //timesCount = 0;
}

// ADC10 interrupt service routine
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    if(state==0){
        val = ADC10MEM;
        Temp[j++] = val;
        if(j==global_times){
            avg = (Temp[global_times-1]+Temp[global_times-2])/2;
            j = 0;
        }
        ADC10CTL0 |= ENC;
    }
    /*
    Temp[adc_idx++] = ADC10MEM;
    if(adc_idx >= 64){
        Temp[0] = Temp[62];
        Temp[1] = Temp[63];
        adc_idx = 2;
    }
    timesCount++;
    if(timesCount >= tempTimes){ // stop adc
        ADC10CTL0 &= ~(ENC + ADC10SC);
    }
    if(adc_idx >= 2){
        state = COUNT_AVG;
        __bic_SR_register_on_exit(LPM3_bits);
    }
    */
}

/***** UART *****/
void TimerA_UART_print(char *string) { //ok
    while (*string) TimerA_UART_tx(*string++);
    //while (TA0CCTL0 & CCIE);
}
void TimerA_UART_init(void) { //ok
    TA0CCTL0 = OUT;   // Set TXD idle as '1'
    TA0CCTL1 = SCS + CM1 + CAP + CCIE; // CCIS1 = 0
    // Set RXD: sync, neg edge, capture, interrupt
    TA0CTL = TASSEL_2 + MC_2; // SMCLK, continuous mode
}
void TimerA_UART_tx(unsigned char byte) { //ok
    while (TA0CCTL0 & CCIE); // Ensure last char TX'd
    TA0CCR0 = TA0R;      // Current count of TA counter
    TA0CCR0 += UART_TBIT; // One bit time till 1st bit
    TA0CCTL0 = OUTMOD0 + CCIE; // Set TXD on EQU0, Int
    txData = byte;       // Load char to be TXD
    txData |= 0x300;    // Add stop bit to TXData
    txData <<= 1;       // Add start bit


}
#pragma vector = TIMER0_A0_VECTOR  // TXD interrupt
__interrupt void Timer0_A0_ISR(void) { //ok
    static unsigned char txBitCnt = 10;
    TA0CCR0 += UART_TBIT; // Set TA0CCR0 for next intrp
    if (txBitCnt == 0) {  // All bits TXed?
        TA0CCTL0 &= ~CCIE;  // Yes, disable intrpt
        txBitCnt = 11;      // Re-load bit counter
    } else {
        if (txData & 0x01) {// Check next bit to TX
            TA0CCTL0 &= ~OUTMOD2; // TX '1??by OUTMODE0/OUT
        } else {
            TA0CCTL0 |= OUTMOD2;
        } // TX '0??
        txData >>= 1;
        txBitCnt--;
    }
}
#pragma vector = TIMER0_A1_VECTOR // RXD interrupt
__interrupt void Timer_A1_ISR(void) {
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;
    switch (__even_in_range(TA0IV, TA0IV_TAIFG)) {
        case TA0IV_TACCR1:     // TACCR1 - UART RXD
            TA0CCR1 += UART_TBIT;// Set TACCR1 for next int
            if (TA0CCTL1 & CAP) { // On start bit edge
                TA0CCTL1 &= ~CAP;   // Switch to compare mode
                TA0CCR1 += UART_TBIT_DIV_2;// To middle of D0
            } else {             // Get next data bit
                rxData >>= 1;
                if (TA0CCTL1 & SCCI) { // Get bit from latch
                    rxData |= 0x80;
                }
                rxBitCnt--;
                if (rxBitCnt == 0) {  // All bits RXed?
                    type = 1;
                    rxBuffer = rxData;  // Store in global
                    rxBitCnt = 8;       // Re-load bit counter
                    TA0CCTL1 |= CAP;     // Switch to capture
                    //state = RECEIVE_CH;
                    //__bic_SR_register_on_exit(LPM3_bits);
                    // Clear LPM0 bits from 0(SR)
                }
            }
        break;
    }
}

