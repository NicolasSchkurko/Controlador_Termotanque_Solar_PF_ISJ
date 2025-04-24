/*
 * usart_avr.c
 *
 * Created: 11/08/2018 17:24:40
 * Author : CarlosQL
 */ 
#include <Arduino.h>
#include <avr/io.h>
#include "UART.h"


int main(void)
{
	
	DDRD |= (1<<4)|(1<<3)|(1<<2); //BIT 2 3 4 OUTPUT
	
	UART_init();
	
	UART_write_txt("digite 1 3 5 para encender leds\n\r");
	UART_write_txt("digite 2 4 6 para apagar leds\n\r");
	uint8_t dato=0;
    /* Replace with your application code */
    while (1) 
    {
		dato = UART_read();
		
		if(dato != 0)
		{
			switch(dato)
			{
				case '1':
					UART_write_txt("LED1 encendido\n\r");
					PORTD|= (1<<2);
				break;
				
				case '2':
					UART_write_txt("LED1 apagado\n\r");
					PORTD &= ~(1<<2);
				break;
				
				case '3':
				UART_write_txt("LED2 encendido\n\r");
				PORTD|= (1<<3);
				break;
				
				case '4':
				UART_write_txt("LED2 apagado\n\r");
				PORTD &= ~(1<<3);
				break;
				
				case '5':
				UART_write_txt("LED3 encendido\n\r");
				PORTD|= (1<<4);
				break;
				
				case '6':
				UART_write_txt("LED3 apagado\n\r");
				PORTD &= ~(1<<4);
				break;
				
				
			}
		}
		
    }
}

