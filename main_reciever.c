// Code originally from:
// https://github.com/andrei-cb/I2C-Display-and-MSP430/tree/master
// Then modified by Joe Young and Dawson Franklin

#include "lcd.h"
#include "stdio.h"
#include "msp430g2553.h"

char temp;
char hum;
int bit;
int humidity = 0;
int temperature = 0;
char str_hum[3];
char str_temp[3];


void readData(void){
    while(!(IFG2&UCA0RXIFG)); //wait for the receive flag to be set
    if (bit ==0){ //if on the first bit of data
        temp = UCA0RXBUF; //read to temp
        bit = 1;
    }
    else if(bit == 1){ //if on second bit of data
        hum = UCA0RXBUF; //read to humidity
        bit = 0;
    }
    humidity = (int)hum; //cast to integer
    sprintf(str_hum, "%d", humidity); //convert to string
    temperature = (int)temp; //cast to integer
    sprintf(str_temp, "%d", temperature); //convert to string


}

int main()
{
    WDTCTL = WDTPW + WDTHOLD; // Stop watchdog

    _EINT(); // enable interrupts

    //UltrasonicInit(); //initialize ultrasonic module
    LcdInit(); // initialize LCD display

    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer                           // P1.0/6 setup for LED output
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    BCSCTL3 |= LFXT1S_2;


    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;
    UCA0CTL1 |= UCSWRST + UCSSEL_2;            // CLK = SMCLK
    UCA0BR0 = 52;                           // set for 9600 baud rate running on SMCLK
    UCA0BR1 = 0x00;
    UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 3
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    bit = 0;



    while (1)
    {
        readData();
        LcdSetPosition(1,1);
        LcdWriteString("Temperature:");
        LcdSetPosition(1,13);
        LcdWriteString(str_temp);
        LcdSetPosition(2,1);
        LcdWriteString("Humidity:");
        LcdSetPosition(2,10);
        LcdWriteString(str_hum);
        __delay_cycles(200000);
    }

return 0;

}

