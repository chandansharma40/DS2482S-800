/*
 * OneWire_I2C.c
 *
 * Created: 9/28/2016 6:45:19 PM
 * Author : Chandan
 */ 
#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include "i2c_master.h"
#include "UART.h"
#include "DB18S20.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <compat/deprecated.h>

#define WRITEADD	0x30
#define READADD		0x31

#define SHORTDETECT 0x04
#define PPD			0x02
#define STATUS_1WB	0x01

bool shortdet = false, ppdstatus = false;

#define SCL		100000L
#define TRUE	1
#define FALSE	0

// void i2c_init()
// {
// 	TWBR = (((F_CPU/SCL)-16)/2);
// 	TWSR = 0;
// 	TWCR |= (1<<TWEN);
// 	
// }
// 
// void i2c_start()
// {
// 	TWCR=(1<<TWINT)|(1<<TWSTA);
// 	while(!(TWCR & (1<<TWINT)));
// 	//cbi(TWCR,TWSTA);
// }
// 
// void i2c_stop()
// {
// 	TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
// 	while(TWCR & (1<<TWSTO));
// }
// 
// uint8_t i2c_write(uint8_t data)
// {
// 	TWDR=data;
// 	TWCR=(1<<TWINT);
// 	while(!(TWCR & (1<<TWINT)));
// 	if((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x28 )
// 	{
// 		return TRUE;
// 	}
// 	else
// 	{
// 		return FALSE;
// 	}
// 	
// }
// 
// uint8_t i2c_read(uint8_t *data,uint8_t ack)
// {
// 	if(ack)
// 	{
// 		TWCR|=(1<<TWEA);
// 	}
// 	else
// 	{
// 		TWCR&=(~(1<<TWEA));
// 	}
// 
// 	TWCR|=(1<<TWINT);
// 	while(!(TWCR & (1<<TWINT)));
// 	if((TWSR & 0xF8) == 0x58 || (TWSR & 0xF8) == 0x50)
// 	{
// 		*data=TWDR;
// 		return TRUE;
// 	}
// 	else
// 	return FALSE;
// }

uint8_t devicereset(){
	uint8_t rec;
	
	i2c_start(WRITEADD);
	i2c_write(0xF0);
	i2c_start(READADD);
	rec = i2c_read_nack();
	i2c_stop();
	
	if((rec & 0xF7) != 0x10){
		return 0;
	}
	else{
	return 1;}
}

uint8_t channelsel(int chan){
	uint8_t status, ch, ch_read;
	
	i2c_start(WRITEADD);
	i2c_write(0xC3);
	
	switch (chan)
	{
		default: case 0: ch = 0xF0; ch_read = 0xB8; break;
		case 1: ch = 0xE1; ch_read = 0xB1; break;
		case 2: ch = 0xD2; ch_read = 0xAA; break;
		case 3: ch = 0xC3; ch_read = 0xA3; break;
		case 4: ch = 0xB4; ch_read = 0x9C; break;
		case 5: ch = 0xA5; ch_read = 0x95; break;
		case 6: ch = 0x96; ch_read = 0x8E; break;
		case 7: ch = 0x87; ch_read = 0x87; break;
	};
	
	i2c_write(ch);
	i2c_start(READADD);
	status = i2c_read_nack();
	i2c_stop();
	
	if (status != ch_read)
	{
		return 0;
	}
	else{
		return 1;
	}	
}

uint8_t writecfg(uint8_t config){
	uint8_t rec;
	
	i2c_start(WRITEADD);
	i2c_write(0xD2);
	i2c_write(config|(~(config)<<4));
	i2c_start(READADD);
	rec = i2c_read_nack();
	i2c_stop();
	
	if(rec != config){
		return 0;
	}
	else{
		return 1;
	}
}

uint8_t owire_reset(){
	uint8_t status, status1;
	int poll_count = 0, poll_limit = 20;
	
	i2c_start(WRITEADD);
	i2c_write(0xB4);
	i2c_start(READADD);
	status = i2c_read_ack();
	do 
	{
		status = i2c_read_ack();
	} while ((status & 0x01) && (poll_count++ < poll_limit));
	
	status1 = i2c_read_nack();
	i2c_stop();
	
	if (poll_count >= poll_limit)
	{
		devicereset();
		return 0;
	}
	
	 if (status1 & PPD)
	 return TRUE;
	 else
	 return FALSE;
	
}

uint8_t owire_write(uint8_t sendbyte){
	uint8_t status, status1;
	int poll_count=0, poll_limit=16;
	
	i2c_start(WRITEADD);
	i2c_write(0xA5);
	i2c_write(sendbyte);
	i2c_start(READADD);
	status = i2c_read_ack();
	do 
	{
		status = i2c_read_ack();
	} while ((status & 0x01) && (poll_count++ < poll_limit));
	
	status1 = i2c_read_nack();
	i2c_stop();
	
	if (poll_count >= poll_limit)
	{
		devicereset();
		return 0;
	}
	else{
		return 1;
	}
}

uint8_t owire_read(void){
	uint8_t rec, status, status1;
	int poll_count=0, poll_limit=16;
	
	i2c_start(WRITEADD);
	i2c_write(0x96);
	i2c_start(READADD);
	status = i2c_read_ack();
	do 
	{
		status = i2c_read_ack();
	} while ((status & STATUS_1WB) && (poll_count++ < poll_limit));
	
	rec = i2c_read_nack();
	
	if (poll_count >= poll_limit)
	{
		devicereset();
		return 0;
	}
	
	i2c_start(WRITEADD);
	i2c_write(0xE1);
	i2c_write(0xE1);
	i2c_start(READADD);
	status1 = i2c_read_nack();
	i2c_stop();
	
	return status1;
}

int main(void)
{
	_delay_ms(100);
	
	uint8_t temperature_l;
	uint8_t temperature_h;
	uint16_t retd = 0;
	char temp[4];
	DDRD=0xF2;
	uint8_t rec=true;
	char ch1[6];
	
	uint8_t tes_count=0;
	int TES1=0;
	double tf1;
	
	UART_init();
	i2c_init();
	
	UART_puts("Start");
	UART_puts("\r\n");
	
	rec = devicereset();
	if (!rec)
	{
		UART_puts("Error");
	}
	else{
	UART_puts("Reset Done");
	UART_puts("\r\n");
	}
	
	rec = writecfg(0x01);
	if(!rec){
		UART_puts("Error");
	}
	else{
		UART_puts("Config Done");
		UART_puts("\r\n");
	}
	
	rec = channelsel(3);
	if (!rec)
	{
		UART_puts("Error");
	}
	else{
		UART_puts("Channel Selected");
		UART_puts("\r\n");
	}
	
	
	
    while (1) 
    {
		
		for (tes_count=0;tes_count<20;tes_count++)
		{
			if(therm_read_temperature(PB0, &TES1) != -1)
			{
				break;
				}else{
			}
		}
		
		tf1 = (double)TES1/10.0;
		dtostrf(tf1,3,1,ch1);
		
		UART_puts("Temperature (Bit Banged): ");
		UART_puts(ch1);
		UART_puts("\r\n");
		
		rec = owire_reset();
		if (!rec)
		{
			UART_puts("Error");
		}
		else{
// 			UART_puts("1wire Reset");
// 			UART_puts("\r\n");
		}
		
		rec = owire_write(0xCC);
		if(!rec)	UART_puts("Error 1");
		
		rec = owire_write(0x44);
		if(!rec)	UART_puts("Error 2");
		
		_delay_ms(1000);
		
		rec = owire_reset();
		if (!rec)
		{
			UART_puts("Error");
		}
		else{
// 			UART_puts("1wire Reset");
// 			UART_puts("\r\n");
		}
		
		rec = owire_write(0xCC);
		if(!rec)	UART_puts("Error 3");
		
		rec = owire_write(0xBE);
		if(!rec)	UART_puts("Error 4");
		
		temperature_l = owire_read();
		UART_putc(temperature_l);
		temperature_h = owire_read();
		UART_putc(temperature_h);
// 		dtostrf(temperature_l,3,2,temp);
// 		UART_puts(temp);
		
 		
 		retd = ceil((( temperature_h << 8 ) + temperature_l ) * 0.625);
		 
 		itoa(retd,temp,10);
		UART_puts("Temperature is : ");
		UART_puts(temp);
		UART_puts("\r\n");
		
		_delay_ms(3000);
    }
}

