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
#define _XTAL_FREQ 20000000
#pragma config FOSC=HS, WDTE=OFF, PWRTE=ON, MCLRE=OFF, CP=OFF, CPD=OFF, BOREN=ON, IESO=OFF, FCMEN=OFF

#define SensorPower	PORTDbits.RD1
#define BellButton	PORTAbits.RA0
#define BellPort	PORTDbits.RD0
#define PortA		PORTBbits.RB7
#define PortB		PORTBbits.RB6
#define PortC		PORTBbits.RB5
#define PortD		PORTBbits.RB4
#define PortE		PORTBbits.RB3
#define PortF		PORTBbits.RB2
#define PortG		PORTBbits.RB1
#define PortH		PORTBbits.RB0
#define PortI		PORTDbits.RD7
#define PortJ		PORTDbits.RD6
#define PortK		PORTDbits.RD5
#define PortL		PORTDbits.RD4
#define PortM		PORTCbits.RC5
#define PortN		PORTCbits.RC4
#define PortO		PORTDbits.RD3
#define PortP		PORTDbits.RD2

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
        Alarm();
		TX('a');
	}
	if (PortB == 1)
	{
        Alarm();
		TX('b');
	}
	if (PortC == 1)
	{
        Alarm();
		TX('c');
	}
	if (PortD == 1)
	{
        Alarm();
		TX('d');
	}
	if (PortE == 1)
	{
        Alarm();
		TX('e');
	}
	if (PortF == 1)
	{
        Alarm();
		TX('f');
	}
	if (PortG == 1)
	{
        Alarm();
		TX('g');
	}
	if (PortH == 1)
	{
        Alarm();
		TX('h');
	}
	if (PortI == 1)
	{
        Alarm();
		TX('i');
	}
	if (PortJ == 1)
	{
        Alarm();
		TX('j');
	}
	if (PortK == 1)
	{
        Alarm();
		TX('k');
	}
	if (PortL == 1)
	{
        Alarm();
		TX('l');
	}
	if (PortM == 1)
	{
        Alarm();
		TX('m');
	}
	if (PortN == 1)
	{
        Alarm();
		TX('n');
	}
	if (PortO == 1)
	{
        Alarm();
		TX('o');
	}
	if (PortP == 1)
	{
        Alarm();
		TX('p');
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
	if (Received == 'y') // Reset
	{	
        SensorPower = 1;
        BellMuted = 0;
        TX('x');
	}
    if (Received == 'z' && Alert == 0) // Mute
	{	
		SensorPower = 0;
        BellMuted = 1;
	}
}

void Alarm()
{
	Alert = 1;
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
	TRISDbits.TRISD1 = 0;	// SensorPower
	TRISAbits.TRISA0 = 1;	// BellButton
	TRISAbits.TRISD0 = 0;	// BellPort
	TRISBbits.TRISB7 = 1;	// PortA
	TRISBbits.TRISB6 = 1;	// PortB
	TRISBbits.TRISB5 = 1;	// PortC
	TRISBbits.TRISB4 = 1;	// PortD
	TRISBbits.TRISB3 = 1;	// PortE
	TRISBbits.TRISB2 = 1;	// PortF
	TRISBbits.TRISB1 = 1;	// PortG
	TRISDbits.TRISB0 = 1;	// PortH
	TRISDbits.TRISD7 = 1;	// PortI
	TRISDbits.TRISD6 = 1;	// PortJ
	TRISDbits.TRISD5 = 1;	// PortK
	TRISCbits.TRISD4 = 1;	// PortL
	TRISDbits.TRISC5 = 1;	// PortM
	TRISCbits.TRISC4 = 1;	// PortN
	TRISDbits.TRISD3 = 1;	// PortO
	TRISDbits.TRISD2 = 1;	// PortP
	
	UART_Init();
	SensorPower = 1;
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