# HERBert
HERBert is a modified garden environment to allow busy users to keep plants alive without requiring much attention. It runs on TI's MSP430G2553. 

## Overview
This project is designed to run on TI's MSP430G2553 Launchpad. The goal is to automate everything about taking care of a potted plant. The functionality is as follows:
  1. Automatically water the plant whenever the soil is too dry.
  2. Sound a buzzer if the water level in the water reservoir is too low, and needs to be refilled manually.
  3. Turn on the grow lights when ambient lighting is too low for healthy plant growth.
  4. Monitor temperature and humidity of the environment and report it to the user on an LCD screen.

## Video Link
https://drive.google.com/file/d/1HoqjslrN5Bkjr_PRIginp1mUCrIiuvK6/view?usp=drivesdk
## Parts List
The following parts were used for the hardware setup:
1. Water level sensors (1x)
2. Capacitive Soil Moisture Sensors (1x)
3. Photo-resistor (1x)
4. Plant Grow Lights (1x)
5. i2c LCD display (1x)
6. Sumbersible water pump and hose (1x)
7. Relays (2x)
8. MSP430G2553 (2x)

## Hardware Setup

### MSP Pinout
There were two MSPs used in this design that communicated between each other using the UART serial interface to pass the temperature and humidity values. 
The first MSP (reciever) drives an LCD display through the 5V onboard power supply, and pins 1.6 and 1.7 as these are the pins associated with the LCD's I2C interface.
Pins 1.1 and 1.2 were reserved for UART communication with the other MSP.

The second MSP (transmitter) uses pins 1.3, 1.4, and 1.5 to read the water level, soil moisture, and light levels from their respective sensors as an analog voltage value. This was done through setting up the 10-bit onboard ADC. Pins 1.6 and 1.7 were used exclusively for the temperature and humidity sensor which operated using i2c. Pins 1.1 and 1.2 were used for serial communication with the reciever MSP to send the temperature and humidity values to the reciever. Pins 2.3 and 2.4 were used as outputs to relays to control the water pump and lights respectively. Pin 2.5 was used to connect to the positive terminal of the buzzer (the negative terminal is grounded) in order to play a sound upon low water levels. 

### Sensor Setup & Peripherals
1. Soil sensor: connect Vcc to 3.3V, Ground to GND, and the data wire as mentioned previously.
2. Water level sensor: same as soil sensor, 3.3V power, common ground, and data wire as per the MSP pinout.
3. Photoresistor: use a voltage divider with 3.3V power, followed by the photoresistor, followed by a 330 Ohm resistor to ground. Tap the divider inbetween the two resistors and wire this to the MPS as indiciated previously.
4. Temperature and humidity sensor: Connect Vcc to 3.3V, Ground to ground. Connect pin 1.6 to the scl pin of the sensor and then add a 330 Ohm pull up resistor to Vcc. Connect pin 1.7 to the sda pin of the sensor and then add a 330 Ohm pull up resistor to Vcc. 
5. LCD display: Connect Vcc to 5V, Ground to ground. Connect pin 1.6 to the scl pin of the display and then add a 330 Ohm pull up resistor to Vcc. Connect pin 1.7 to the sda pin of the sensor and then add a 330 Ohm pull up resistor to Vcc. 
6. Relays: connect the water pump relay to pin 2.3 for the COM port, 3.3V power, and ground GND. The motor connects to the other end in the normally open (N O position). Connect the grow light relay to pin 2.4 for the COM port, 3.3V power, and ground GND. The motor connects to the other end in the normally connected (NC position).

## Logic
![image](https://github.com/pchar4/HERBert/assets/43528347/1956edae-2d9d-4435-aca7-d3e4b41ac486)



## Code Architecture

The code architecture can be split into four main parts: periodic state checks for power savings, ADC setup, I2C setup, and UART setup. Details are as follows:

### Periodic State Checks
The ISR wakes up ever second, increments a counter variable and goes to sleep. If the counter variable hits a user defined "number of seconds" defined at the top of the file, it then enters the state where it checks the environment of the plant. If the plant needs water or the water needs to be filled, it then sets the counter to enter the state check the very next second so that we aren't pumping water in hour increments. Once the every-second-checks indiciate that the once dry soil is no longer dry, or that the low water is now filled, it then exits this mode and goes back to the user-defined check period, which in this case is on an hourly basis. Between checks the device enters low power mode in order to save energy/power.

### ADC Setup
This uses the onboard 10-bit ADC of the MSP. The setup involves creating an array to store a packet of information from the adc, then selecting which pins we want to actually enable the reads from, (a subset of port 1 in our case). After that, we would check to see that the ADC was no longer busy and query the values from it. The ADC operates on repeat-single channel mode (to continually get values), operates at the full clock speed and has a sample-and-hold time to 16 ADC10CLK cycles. This determines the duration the ADC samples the input signal, which is set to as long as possible to get a stable value.

After the setup, when reading, we first disable conversions, wait until the ADC is free, then start conversions and store the first address the series of conversions at the start of the array, which we then read later on to get the values we need.

### I2C Setup
The I2C Setup varies from the sensor to the display. 
Sensor: Initialize the ports on the MSP430 to allow for I2C communication. This involves selecting the clock and data pins, setting the communication rate (~100kHz for standard I2C), writing the sub address, setting main mode, and enabling interrupt service routines. To collect a measurement from the sensor, you first must write the command to the sensor to start a new measurement. This involves setting the start condition (which transmits the sub address), wait for the acknowledgement, sending the register (0xAC), then the command which is two bytes 0x33 and 0x00. After the last acknowlegement, transmit the stop condition and clear the transmit interrupt flag. You need to wait a bit before reading the measurment (~80ms), and then transmit the sub address in recieve mode, after which the sensor will send 7 bytes of data containing temp data, humidity data, state of machine, and CRC data. Convert temperature data, and humidity data to celcius/relative humidity based on formulas given in datasheet. 

Display: Initialize the ports on the MSP430 in the same way as for the sensor. Then write various intializations to the LCD module itself. The initializations are based on writing to different registers. You write the first initalization command, then you write the commands for making the bus width (changing it to have two cycles of 4 bits each), then writing it to clear the screen. For each write, you essentially take the high 4 bits and then add commands at the end of the command to enable the LCD and add backlight. If you're in 4 bit mode you then send the lower four bits in the same way after transmission. Setting the position of the LCD is also writing to registers in the LCD. To write the numbers, we decompose strings into their characters and sent each one at a time for the LCD to display on screen.

### UART Setup
The UART Setup is much simpler than the setup for I2C. We make sure they are running at a common Baud rate (9600 sourced from the 1MHZ SMCLK) setting P1.2 on the transmitting msp to TXD and setting P1.1 on the recieving msp to RXD. Then we disable any interrrupts from the UART since they run on a common vector with the I2C interrupts. To transmit, we first make sure that the transmit flag is raised and then we load a byte onto the transmit buffer to send the temperature byte. Then we wait until the transmit flag has been set again, and load the humidity data on the the transmit buffer. 

For the recieve data we wait until the recieve flag has been set and then store the byte on the buffer into it's corresponding variable. Then we wait until the reieve flag has been set again, and store the byte into it's corresponding variable. Then we send these bytes via I2C to the display. 
