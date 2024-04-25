# HERBert
Automatically Water a Garden Plant Using an MSP430G2553 From TI

## Overview
This project is designed to run on TI's MSP430G2553 Launchpad. The goal is to automate everything about taking care of a potted plant. The functionality is as follows:
  1. Automatically water the plant whenever the soil is too dry.
  2. Sound a buzzer if the water level in the water reservoir is too low, and needs to be refilled manually.
  3. Turn on the grow lights when ambient lighting is too low for healthy plant growth.
  4. Monitor temperature and humidity of the environment and report it to the user on an LCD screen.

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
The first MSP (reciever) drives an LCD display through the 5V onboard power supply, and pins 1.6 and 1.7 as these are the pins associated with the I2C interface.
Pins 1.1 and 1.2 were reserved for UART communication with the other MSP.

The second MSP (transmitter) uses pins 1.3, 1.4, and 1.5 to read the water level, soil moisture, and light levels from their respective sensors as an analog voltage value. This was done through setting up the 10-bit onboard ADC. Pins 1.6 and 1.7 were used exclusively for the temperature and humidity sensor which operated using i2c. Pins 1.1 and 1.2 were used for serial communication with the reciever MSP to send the temperature and humidity values to the reciever. Pins 2.3 and 2.4 were used as outputs to relays to control the water pump and lights respectively. Pin 2.5 was used to connect to the positive terminal of the buzzer (the negative terminal is grounded) in order to play a sound upon low water levels. 

### Sensor Setup & Peripherals
1. Soil sensor: connect Vcc to 3.3V, Ground to GND, and the data wire as mentioned previously.
2. Water level sensor: same as soil sensor, 3.3V power, common ground, and data wire as per the MSP pinout.
3. Photoresistor: use a voltage divider with 3.3V power, followed by the photoresistor, followed by a 330 Ohm resistor to ground. Tap the divider inbetween the two resistors and wire this to the MPS as indiciated previously.
4. Temperature and humidity sensor:
5. LCD display:
6. Relays: connect the water pump relay to pin 2.3 for the COM port, 3.3V power, and ground GND. The motor connects to the other end in the normally open (N O position). Connect the grow light relay to pin 2.4 for the COM port, 3.3V power, and ground GND. The motor connects to the other end in the normally connected (NC position).

## Code Architecture

The code architecture can be split into four main parts: periodic state checks for power savings, ADC setup, I2C setup, and UART setup. Details are as follows:

### Periodic State Checks

### ADC Setup

### I2C Setup

### UART Setup

