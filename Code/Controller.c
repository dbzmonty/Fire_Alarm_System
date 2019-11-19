/*
 * File:   controller.c
 * Author: Dbz
 *
 * Sources used: 
 * https://circuitdigest.com/microcontroller-projects/uart-communication-using-pic16f877a
 * https://stackoverflow.com/questions/46030610/pic16f1829-uart-rx-interrupt-not-working-using-mplabx-and-xc8-compiler
 *
 * Created on 2019. november 17., 17:27
 */

// Definitions
#define _XTAL_FREQ 4000000
#pragma config FOSC=INTRCIO, WDTE=OFF, PWRTE=OFF, MCLRE=ON, CP=OFF, CPD=OFF, BOREN=OFF, IESO=OFF, FCMEN=OFF

#define BellButton	PORTAbits.RA5
#define BellPort	PORTAbits.RA4
#define PortA		PORTCbits.RC4
#define PortB		PORTCbits.RC5

// Includes
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Function declarations
void CheckPorts(void);
void CheckDoorBell(void);
void CheckRecieved(void);
void Alarm(void);

// Variables
int flagRXFramingError = 0;
int flagRXOverrunError = 0;
volatile unsigned char Received;
bool Alert;
bool BellMuted;

// <editor-fold defaultstate="collapsed" desc="Serial communication functions">
unsigned char RX()
{
    return RCREG;
}

void writeRXIN(volatile unsigned char a)
{
    Received = a;
}

void TX(unsigned char a)
{
    while(!PIR1bits.TXIF);
    TXREG = a;
}

void UART_Init()
{
    TRISBbits.TRISB7 = 0;	// TX PIN direction
	TRISBbits.TRISB5 = 1;	// RX PIN direction
    
    SPBRG = ((_XTAL_FREQ / 16) / 9600) - 1;
    BRGH  = 1;          // For high baud rate
        
    TXSTAbits.SYNC = 0; // Asynchronous mode
    RCSTAbits.SPEN = 1; // Serial Port Enabled
    RCSTAbits.RX9 = 0;  // 8 bit reception
    TXSTAbits.TX9 = 0;  // 8 bit transmission

    RCSTAbits.CREN = 1; // Receiver enabled
    TXSTAbits.TXEN = 1; // Transmitter enabled 

    PIE1bits.TXIE = 0;  // Enable USART Transmitter interrupt
    PIE1bits.RCIE = 1;  // Enable USART Receive interrupt
    
    while (PIR1bits.RCIF)
	{
        writeRXIN(RX());
    }

    INTCONbits.PEIE = 1;    // Enable peripheral interrupts
    INTCONbits.GIE = 1;     // Enable global interrupts
}

__interrupt() void ISR(void)
{
    if (PIE1bits.RCIE && PIR1bits.RCIF)
    {
        while (PIR1bits.RCIF) // Handle RX pin interrupts
		{
            writeRXIN(RX());
        }
        if (RCSTAbits.FERR)
		{
            flagRXFramingError = 1;
            SPEN = 0;
            SPEN = 1;

        }
        if (RCSTAbits.OERR)
		{
            flagRXOverrunError = 1;
            CREN = 0;
            CREN = 1;
        }
    }
}
// </editor-fold>

void CheckPorts()
{
	if (PortA == 1)
	{
		Alert = 1;
        Alarm();
		TX('a');
	}
	if (PortB == 1)
	{
		Alert = 1;
        Alarm();
		TX('b');
	}
}

void CheckDoorBell()
{
	if (BellButton == 1)
	{
        BellMuted = 0;
        BellPort = 1;
	}
	else
	{
		BellPort = 0;
	}
}

void CheckRecieved()
{		
	if (Received == 'y' && Alert == 1) // Reset
	{	
        // Áramszünetet a szenzoroknak
		Alert = 0;
        BellMuted = 0;
        TX('x');
	}
    if (Received == 'z' && Alert == 1) // Mute
	{	
        BellMuted = 1;
	}
}

void Alarm()
{
	if (BellMuted == 0)
	{
		BellPort = 1;
		__delay_ms(500);
		BellPort = 0;
		__delay_ms(500);
	}	
}

int main()
{
	// Disable analog pins
	ANSEL = 0;
	ANSELH = 0;
	
	// Disable ADC interrupts
	PIE1bits.ADIE = 0;
	
	// Set port directions
	TRISAbits.TRISA5 = 1;	// BellButton
	TRISAbits.TRISA4 = 0;	// BellPort
	TRISCbits.TRISC4 = 1;	// PortA
	TRISCbits.TRISC5 = 1;	// PortB
				
	UART_Init();
    BellMuted = 0;
		
	while(1) // The main loop
	{        
		CheckPorts();
		__delay_ms(50);
		CheckDoorBell();
		__delay_ms(50);
		CheckRecieved();
		__delay_ms(50);
	}	
}