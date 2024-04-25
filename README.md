# automatic-plant-watering-msp430g2553
Automatically Water a Garden Plant Using an MSP430G2553 From TI

This project is designed to run on TI's MSP430G2553 Launchpad. The goal is to automate everything about taking care of a potted plant. The functionality is as follows:
  1. Automatically water the plant whenever the soil is too dry.
  2. Sound a buzzer if the water level in the water reservoir is too low, and needs to be refilled manually.
  3. Turn on the grow lights when ambient lighting is too low for healthy plant growth.
  4. Monitor temperature and humidity of the environment and report it to the user on an LCD screen.

The following parts were used for the hardware setup:
1. Water level sensors
2. Capacitive Soil Moisture Sensors
3. Photo-resistor
4. Plant Grow Lights
5. i2c LCD display
6. Sumbersible water pump
7. Relays

There were two MSPs used in this design that communicated between each other using the UART serial interface to pass the temperature and humidity values. 
The first MSP (reciever) drives an LCD display through the 5V onboard power supply, and pins 1.6 and 1.7 as these are the pins associated with the I2C interface.
Pins 1.1 and 1.2 were reserved for UART communication with the other MSP.

The second MSP (transmitter) uses pins 1.3, 1.4, and 1.5 to read the water level, soil moisture, and light levels from their respective sensors as an analog voltage value. This was done through setting up the 10-bit onboard ADC. Pins 1.6 and 1.7 were used exclusively for the temperature and humidity sensor which operated using i2c. Pins 1.1 and 1.2 were used for serial communication with the reciever MSP to send the temperature and humidity values to the reciever. Pins 2.3 and 2.4 were used as outputs to relays to control the water pump and lights respectively. Pin 2.5 was used to connect to the positive terminal of the buzzer (the negative terminal is grounded) in order to play a sound upon low water levels. 

