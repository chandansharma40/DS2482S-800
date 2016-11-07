# DS2482S-800
ATmega 1284p and DS2482S-800 interfacing code. DS2482S-800 is a 1-wire I2C bridge IC, which has 8 channels for 1-wire protocol sensors and devices. Helped me save, GPIOs on my micro controller as it just uses SCL and SDA pins

# Libraries included
  1. DS18B20.h - 1-wire temperature sensor, temperature read by bit banging
  2. UART.h - USART1 functions for Atmega1284p.
  3. i2c_master.h - A TWI library for Atmel Microcontrollers
