#include "msp430g2553.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>


#define ONE_SEC 12500  // Timer period for 1 sec on this clk
#define TIMER_PERIOD 31250  // Timer period for 125 kHz clock and 0.25 sec interval

/*-----------------------------------------------------------------------------*/
// CHANGE THIS VALUE TO SEE IN HOW MANY SECONDS TO WAKE UP YOUR PLANT AND POLL IT'S STATE
#define CHECK_INTERVAL 3600 // defining how many seconds we wait until we check the state of the plant's environment
/*-----------------------------------------------------------------------------*/
int curr_interval;
float water_voltage, soil_voltage, light_voltage; // variables to determine the environmental conditions
unsigned int adc[8]; // array to hold the values from the analog reads from sensor

int periods[] = {1000000/261.63, // more buzzer sound
   1000000/329.63,
   1000000/392.00,
   1000000/523.5};
int which_period = 0; // which sound we want to play (customizable up to 4 options)

unsigned char byteToTransmit[3]; // i2c transmission byte
unsigned char byteSent = 0; // flag for sent
unsigned char RXData[7]; // i2c data read back
unsigned int byteRead = 0; // flag for read
unsigned char Reads; // number of reads
uint32_t temp; // stores temp reading
uint32_t hum; // stores humidity reading
float floattemp; // stores temp in celcius
float floathum; // stores humidity in percent
char chartemp; // char versions of above variables
char charhum;


#define SDA_PIN BIT7 // pin definitions for the i2c bus
#define SCL_PIN BIT6
#define PRESCALE 12

void I2cTransmitInit(unsigned char slaveAddress)
{
    P1SEL      |= SDA_PIN + SCL_PIN;               // Assign I2C pins to USCI_B0
    P1SEL2     |= SDA_PIN + SCL_PIN;
    UCB0CTL1    = UCSWRST;                         // Enable SW reset
    UCB0CTL0    = UCMST + UCMODE_3 + UCSYNC;       // I2C Master, synchronous mode
    UCB0CTL1    = UCSSEL_2 + UCSWRST;              // Use SMCLK, keep SW reset
    UCB0BR0     = PRESCALE;                        // Set prescaler - SMCLK = ~1048Khz/12 = 87.3Khz
    UCB0BR1     = 0;
    UCB0I2CSA   = slaveAddress;                    // Set slave address
    UCB0CTL1   &= ~UCSWRST;                        // Clear SW reset, resume operation
    UCB0I2CIE   = UCNACKIE;                        // Interrupt on slave Nack
    IE2         = UCB0TXIE;                        // Enable TX interrupt
}

void init_I2Cread(unsigned char slaveAddress){
    P1SEL |= BIT6 + BIT7;   // Assign P1.6 to SCL and P1.7 to SDA
    P1SEL2 |= BIT6 + BIT7;

    // Configure I2C module
    UCB0CTL1 |= UCSWRST;    // Enable software reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;   // Master mode, I2C mode, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;  // Clock source: SMCLK, keep in reset
    UCB0BR0 = 12;   // Set clock divider for desired clock frequency (adjust according to your application)
    UCB0BR1 = 0;
    UCB0I2CSA  = slaveAddress; //set sub address
    UCB0CTL1 &= ~UCSWRST;   // Release software reset
    IE2 |= UCB0RXIE;                          // Enable RX interrupt

}


void I2C_read(unsigned char slaveAddress) {
    // Send start condition
    init_I2Cread(slaveAddress);
    byteRead = 7; // begin read


    UCB0CTL1 |= UCTXSTT;
   // while(UCB0CTL & UCTXTT);
    // Main transmit mode, generate start condition

 //   I2cNotReady();
                          // Enter LPM0 w/ interrupts
}
void I2cTransmit(unsigned char slaveAddress, unsigned char third, unsigned char reg, unsigned char byte, unsigned char Read)
{
    I2cTransmitInit(slaveAddress); // send transmit signal
    byteToTransmit[1] = reg;
    byteToTransmit[0] = byte;
    byteToTransmit[2] = third;
    byteSent = 3;
    UCB0CTL1 |= UCTR + UCTXSTT; // Generate start condition
}

unsigned char I2cNotReady()
{
    return (UCB0STAT & UCBBUSY); // Check if I2C bus is busy
}
void sendData(char data){ // sending UART DATA
    while(!(IFG2&UCA0TXIFG)); // wait for it to be clear
    UCA0TXBUF = data;

}

void tempRead(){
    unsigned char temp0;
    unsigned char temp1;
    unsigned char temp2;
    unsigned char hum0;
    unsigned char hum1;
    unsigned char hum2;
    float denom;
    denom = pow(2, 20);
    temp = 0;
    hum = 0;
    I2cTransmit(0x38,0xAC,0x33,0x00,3);
    __delay_cycles(200000);
    I2C_read(0x38);
    __delay_cycles(200000);
    temp2 = RXData[3] & 0x0F;
    temp1 = RXData[2];
    temp0 = RXData[1];
    hum2 = RXData[5];
    hum1 = RXData[4];
    hum0 = RXData[3] & 0xF0;
    temp <<= 8;
    temp |= temp2;
    temp <<= 8;
    temp |= temp1;
    temp <<= 8;
    temp |= temp0;
    hum <<= 8;
    hum |= hum2;
    hum <<= 8;
    hum |= hum1;
    hum <<= 8;
    hum |= hum0;
    hum >>=4;
    floattemp = (float)temp;
    floathum = (float)hum;
    floattemp = ((floattemp/denom) *200) -50;
    floathum = (floathum/denom)*100;
}

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

    // Configure Clock
    BCSCTL1 = CALBC1_1MHZ;     // Set range
    DCOCTL = CALDCO_1MHZ;      // Set DCO step + modulation
    BCSCTL3 |= LFXT1S_2;       // ACLK = VLO

    // UART CONFIGURATION
    P1SEL = BIT1|BIT2;
    P1SEL2 = BIT1|BIT2;                     // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSWRST+UCSSEL_2;
    UCA0BR0 = 0x68; // 19200 baud at 1Mhz - coudl change to 0x68 for 9600 baud
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 3
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 &= ~(UCA0RXIE| UCA0TXIE);               // Enable USCI_A0 TX/RX interrupt

    // Configuring output pins
    P2DIR |= BIT5; // We need P2.5 to be output
    P2DIR |= BIT1; // Set P2.1 as output
    P2DIR |= BIT3; // Output on 2.3
    P2DIR |= BIT4; // Output on 2.4
    P2SEL |= BIT5; // P2.5 is TA1.2 PWM output
    P2SEL &= ~BIT3; // make sure bit3 is gpio
    P2SEL &= ~BIT4; // make sure the bit4 is gpio

    TA1CCTL2 = OUTMOD_7; // setting to set reset mode
    TA1CTL = TASSEL_2 + MC_1; // SMCLK, upmode


    // Configuring 10 bit adc
    ADC10CTL1 = INCH_7 + ADC10DIV_0 + CONSEQ_3 + SHS_0; // enabling adc to read from entire of port 1
    ADC10CTL0 = SREF_0 + ADC10SHT_2 + MSC + ADC10ON; // turning adc on and configuring some variables
    ADC10AE0 = BIT5 + BIT4 + BIT3 + BIT0; // only saying that ADC works for P1.0, 1.3, 1.4, 1.5
    ADC10DTC1 = 8; // Data transfer packet size

    int volmod = 10; // variable for turning the buzzer off essentially

    // Configure Timer A0 to give us interrupts being driven by ACLK
    TA0CTL = TASSEL_1 + MC_2 + TAIE;  // ACLK, upmode, counter interrupt enable

    TA0CCR0 = ONE_SEC;  // Register 0 counter value to trigger interrupt
    TA0CCTL0 = CCIE;    // CCR0 interrupt enabled
    TA0CTL = TASSEL_2 + MC_1 + ID_3;    // Use SMCLK as the clock source, Up mode
    _EINT(); // enable interrupts
    __enable_interrupt(); // global interrupt enable

    while(1){
        if (curr_interval >= CHECK_INTERVAL){
        ADC10CTL0 &= ~ENC;
        while (ADC10CTL1 & BUSY); // wait until the adc is not busy
        ADC10CTL0 |= ENC + ADC10SC;
        ADC10SA = (unsigned int)adc; // grab all the values from the ADC after confirming that it is the 10 bit adc
        chartemp = (char)floattemp; // cast temp and humidity to char
        charhum = (char)floathum;

        soil_voltage = adc[3] * 3.3 / 1023; // convert soil ADC reading to voltage
        water_voltage = adc[4] * 3.3 / 1023; // do same for water level adc reading
        light_voltage = adc[2] * 3.3 / 1023; // and light level adc reading

        if (soil_voltage > 2.7){
             // turn on the pump if soil is too dry
            P2OUT |= BIT3;
            curr_interval = CHECK_INTERVAL - 1; // set it here so we check very quickly if the soil still needs pump
        } else {
             // turn off the pump once ideal wetness achieved
            P2OUT &= ~BIT3;
        }
        if (water_voltage < 1.0){
            // screech that you don't have water
             // Divide by 2
            volmod = 1;
            curr_interval = CHECK_INTERVAL - 1; // quick checks to make sure it turns off within a second of getting water
        } else {
            volmod = 10; // turn buzzer volume down to essentially 0

        }
        if (light_voltage > 3.1){ // these can update on the hour, won't be a problem
            P2OUT |= BIT4; // turn off the lights
        } else {
            P2OUT &= ~BIT4; // turn on the lights
        }
        tempRead(); // read the temperature and humidity always
        sendData(chartemp); // send data to LCD
        sendData(charhum);
        TA1CCR0 = periods[which_period]; // update the state of the buzzer always
        TA1CCR2 = periods[which_period]>>volmod;
        } else {
            curr_interval++; // increment curr interval to get closer to the next time to run logic
        }
        __bis_SR_register(LPM0_bits + GIE); // enter LPM0 after being woken up and running this once
    }
}

// Timer 0 interrupt service routine for register 0
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A0 (void)
#else
#error Compiler not supported!
#endif
{
    TA0CCR0 += ONE_SEC;                       // Increment register 0 value for next interrupt
    __bic_SR_register_on_exit(LPM0_bits);     // Clear LPM3 bits from 0(SR)
}


//#pragma vector = USCIAB0TX_VECTOR
//__interrupt void USCIAB0TX_ISR(void)
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
    if (!(IFG2 & UCB0RXIFG))
    {
        if (byteSent == 0) // If the byte was sent...
        {
            UCB0CTL1 |= UCTXSTP; // Generate stop condition
            IFG2 &= ~UCB0TXIFG;
        }
        else
        {
            UCB0TXBUF = byteToTransmit[byteSent-1]; // Send the byte
            byteSent--; // Modify the variable accordingly
        }
    }
    else if ((IFG2 & UCB0RXIFG))
    {
        if (byteRead == 0) // If the byte was sent...
        {
            UCB0CTL1 |= UCTXSTP; // Generate stop condition
            IFG2 &= ~UCB0RXIFG;
        }
        else
        {
            RXData[byteRead-1] = UCB0RXBUF;                         //Read data
            byteRead--; // Modify the variable accordingly
        }
    }
    else{
        IFG2 &= ~UCA0TXIFG;
        IFG2 &= ~UCA0RXIFG;
    }

}
