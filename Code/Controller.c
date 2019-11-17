/*
 * File:   controller.c
 * Author: Dbz
 *
 * Sources used: 
 * https://circuitdigest.com/microcontroller-projects/uart-communication-using-pic16f877a
 *
 * Created on 2019. november 17., 17:27
 */

// Definitions
#define Baud_rate 9600
#define _XTAL_FREQ 4000000
#pragma config FOSC=INTRCIO, WDTE=OFF, PWRTE=OFF, MCLRE=ON, CP=OFF, CPD=OFF, BOREN=OFF, IESO=OFF, FCMEN=OFF

// Includes
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Serial communication functions
void Initialize_UART(void)
{
	TRISBbits.TRISB7 = 0;	// TX
	TRISBbits.TRISB5 = 1;	// RX
	
    SPBRG = ((_XTAL_FREQ/16)/Baud_rate) - 1;
    BRGH  = 1;	// For high baud_rate
    SYNC  = 0;	// Asynchronous
    SPEN  = 1;	// Enable serial port pins
    TXEN  = 1;	// Enable transmission
    CREN  = 1;	// Enable reception
    TX9   = 0;	// 8-bit transmission mode selected
    RX9   = 0;	// 8-bit reception mode selected
}

void UART_send_char(char bt)
{
    while(!TXIF);
    TXREG = bt;
}

char UART_get_char()   
{
    if(OERR)
    {
        CREN = 0;
        CREN = 1;
    }
    
    while(!RCIF);
    
    return RCREG;
}

void UART_send_string(char* st_pt)
{
    while(*st_pt)
        UART_send_char(*st_pt++);
}

int main()
{
	// Set analog pins
	ANSEL = 0;
	ANSELH = 0;
	
	// Set port directions
			
	Initialize_UART();
	
	while(1) // The main loop
	{	
		UART_send_char('a');
        __delay_ms(2000);
        UART_send_char('c');
        __delay_ms(2000);
        UART_send_char('f');
        __delay_ms(2000);
        UART_send_char('h');
        __delay_ms(2000);
        UART_send_char('k');
        __delay_ms(2000);        
	}
}