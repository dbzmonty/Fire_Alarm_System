/*
 * File:   Indoor.c
 * Author: Dbz
 *
 * Sources used: 
 * https://circuitdigest.com/microcontroller-projects/16x2-lcd-interfacing-with-pic-microcontroller
 * https://circuitdigest.com/microcontroller-projects/uart-communication-using-pic16f877a
 * https://stackoverflow.com/questions/46030610/pic16f1829-uart-rx-interrupt-not-working-using-mplabx-and-xc8-compiler
 *
 * Created on 2019. november 10., 11:05
 */

// Definitions
#define RS RA2
#define EN RA1
#define D4 RC0
#define D5 RC1
#define D6 RC2
#define D7 RB4

#define SW_Mute     PORTCbits.RC3
#define SW_Reset    PORTCbits.RC6
#define SW_Light    PORTCbits.RC7
#define LED         PORTBbits.RB6

#define _XTAL_FREQ 4000000
#pragma config FOSC=INTRCIO, WDTE=OFF, PWRTE=OFF, MCLRE=ON, CP=OFF, CPD=OFF, BOREN=OFF, IESO=OFF, FCMEN=OFF

// Includes
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//// Function declarations
void Mute(void);
void Reset(void);
void CheckButtons(void);
void Alarmed(void);
void CheckRecieved(void);

// Variables
int flagRXFramingError = 0;
int flagRXOverrunError = 0;
volatile unsigned char Received;
bool Alert;

// <editor-fold defaultstate="collapsed" desc="LCD functions">
void Lcd_SetBit(char data_bit)
{
    if(data_bit& 1) 
        D4 = 1;
    else
        D4 = 0;

    if(data_bit& 2)
        D5 = 1;
    else
        D5 = 0;

    if(data_bit& 4)
        D6 = 1;
    else
        D6 = 0;

    if(data_bit& 8) 
        D7 = 1;
    else
        D7 = 0;
}

void Lcd_Cmd(char a)
{
    RS = 0;           
    Lcd_SetBit(a);
    EN  = 1;         
        __delay_ms(4);
        EN  = 0;         
}

Lcd_Clear()
{
    Lcd_Cmd(0);
    Lcd_Cmd(1);
}

void Lcd_Set_Cursor(char a, char b)
{
    char temp,z,y;
    if(a== 1)
    {
      temp = 0x80 + b - 1;
        z = temp>>4;
        y = temp & 0x0F;
        Lcd_Cmd(z);
        Lcd_Cmd(y);
    }
    else if(a== 2)
    {
        temp = 0xC0 + b - 1;
        z = temp>>4;
        y = temp & 0x0F;
        Lcd_Cmd(z);
        Lcd_Cmd(y);
    }
}

void Lcd_Start()
{
  Lcd_SetBit(0x00);
  for(int i=1065244; i<=0; i--)  NOP();  
  Lcd_Cmd(0x03);
    __delay_ms(5);
  Lcd_Cmd(0x03);
    __delay_ms(11);
  Lcd_Cmd(0x03);
  Lcd_Cmd(0x02);
  Lcd_Cmd(0x02);
  Lcd_Cmd(0x08);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x0C);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x06);
}

void Lcd_Print_Char(char data)
{
   char Lower_Nibble,Upper_Nibble;
   Lower_Nibble = data&0x0F;
   Upper_Nibble = data&0xF0;
   RS = 1;
   Lcd_SetBit(Upper_Nibble>>4);
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP();
   EN = 0;
   Lcd_SetBit(Lower_Nibble);
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP();
   EN = 0;
}

void Lcd_Print_String(char *a)
{
    int i;
    for(i=0;a[i]!='\0';i++)
       Lcd_Print_Char(a[i]);
}
// </editor-fold>

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

// Logic functions
void Mute()
{
    TX('z');
}

void Reset()
{
	Alert = false;
	//Received = 'x';
	TX('y');
	Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String("ALL OK!");
    Lcd_Set_Cursor(2,1);
    Lcd_Print_String("");
}

void CheckButtons()
{	
	if (SW_Mute == 1)
	{
        Mute();
        __delay_ms(50);
	}
    if (SW_Reset == 1)
    {
        Reset();
        LED = 1;
        __delay_ms(2000);
	}	
    if (SW_Light == 1 || Alert == 1)
	{
		LED = 1;
	}		
	if (SW_Light == 0 && Alert == 0)
	{
		LED = 0;
	}
}

void Alarmed()
{	
	switch(Received)
	{
		case 'a':				
			Lcd_Set_Cursor(2,1);
			Lcd_Print_String("A");
			break;
			
		case 'b':
			Lcd_Set_Cursor(2,2);
			Lcd_Print_String("B");
			break;
		case 'c':				
			Lcd_Set_Cursor(2,3);
			Lcd_Print_String("C");
			break;
			
		case 'd':
			Lcd_Set_Cursor(2,4);
			Lcd_Print_String("D");
			break;
		case 'e':				
			Lcd_Set_Cursor(2,5);
			Lcd_Print_String("E");
			break;
			
		case 'f':
			Lcd_Set_Cursor(2,6);
			Lcd_Print_String("F");
			break;
		case 'g':				
			Lcd_Set_Cursor(2,7);
			Lcd_Print_String("G");
			break;
			
		case 'h':
			Lcd_Set_Cursor(2,8);
			Lcd_Print_String("H");
			break;
		case 'i':				
			Lcd_Set_Cursor(2,9);
			Lcd_Print_String("I");
			break;
			
		case 'j':
			Lcd_Set_Cursor(2,10);
			Lcd_Print_String("J");
			break;
		case 'k':				
			Lcd_Set_Cursor(2,11);
			Lcd_Print_String("K");
			break;
			
		case 'l':
			Lcd_Set_Cursor(2,12);
			Lcd_Print_String("L");
			break;
		case 'm':				
			Lcd_Set_Cursor(2,13);
			Lcd_Print_String("M");
			break;
			
		case 'n':
			Lcd_Set_Cursor(2,14);
			Lcd_Print_String("N");
			break;
		case 'o':				
			Lcd_Set_Cursor(2,15);
			Lcd_Print_String("O");
			break;
			
		case 'p':
			Lcd_Set_Cursor(2,16);
			Lcd_Print_String("P");
			break;		
	}
}

void CheckRecieved()
{		
	if (Received == 'a' || Received == 'b' || Received == 'c' || Received == 'd'
		|| Received == 'e' || Received == 'f' || Received == 'g' || Received == 'h'
		|| Received == 'i' || Received == 'j' || Received == 'k' || Received == 'l'
		|| Received == 'm' || Received == 'n' || Received == 'o' || Received == 'p')
	{	
		if (Alert == 0)
		{
			Lcd_Clear();
			Lcd_Set_Cursor(1,1);
			Lcd_Print_String("ALERT AT SECTOR:");			
			Alert = 1;
		}			
		Alarmed();
	}
    /*else if (Received == 'x')
    {
        LED = 1;
        __delay_ms(50);
        LED = 0;
    }*/
}

int main()
{
	// Disable analog pins
	ANSEL = 0;
	ANSELH = 0;
	
	// Disable ADC interrupts
	PIE1bits.ADIE = 0;
	
	// Set port directions
	TRISAbits.TRISA2 = 0;	// LCD
	TRISAbits.TRISA1 = 0;	// LCD
	TRISCbits.TRISC0 = 0;	// LCD
	TRISCbits.TRISC1 = 0;	// LCD
	TRISCbits.TRISC2 = 0;	// LCD
	TRISBbits.TRISB4 = 0;	// LCD
	TRISCbits.TRISC3 = 1;	// SW1
	TRISCbits.TRISC6 = 1;	// SW2
	TRISCbits.TRISC7 = 1;	// SW3
	TRISBbits.TRISB6 = 0;	// LED
	
	//int a = 56;
	//char s[16];
	//sprintf(s, "Integer = %d", a);
		
	UART_Init();
    Lcd_Start();
    Reset();
	
	while(1) // The main loop
	{	
		CheckButtons();
		__delay_ms(50);		
		CheckRecieved();
		__delay_ms(50);
	}	
}