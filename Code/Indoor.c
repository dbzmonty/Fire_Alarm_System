/*
 * File:   Indoor.c
 * Author: Dbz
 *
 * Sources used: 
 * https://circuitdigest.com/microcontroller-projects/16x2-lcd-interfacing-with-pic-microcontroller
 * https://circuitdigest.com/microcontroller-projects/uart-communication-using-pic16f877a
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

#define SW1 PORTCbits.RC3
#define SW2 PORTCbits.RC6
#define SW3 PORTCbits.RC7
#define LED	PORTBbits.RB6

#define Baud_rate 9600
#define _XTAL_FREQ 4000000
#pragma config FOSC=INTRCIO, WDTE=OFF, PWRTE=OFF, MCLRE=ON, CP=OFF, CPD=OFF, BOREN=OFF, IESO=OFF, FCMEN=OFF

// Includes
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Variables
bool Alert;
char Received;

// LCD functions
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

// Logic functions
void Reset()
{
	Alert = false;	
	UART_send_char('y');	
	Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String("ALL OK!");
    Lcd_Set_Cursor(2,1);
    Lcd_Print_String("");
}

void CheckButtons()
{
	// SW1 pushed (RESET)
	if (SW1 == 1)
	{
        Reset();
        __delay_ms(2000);	
	}
        
	// SW3 pushed (LIGHT)
    else if (SW3 == 1 || Alert == 1)
	{
		LED = 1;
	}		
	else if (SW3 == 0 && Alert == 0)
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
	Received = UART_get_char();
		
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
}

int main()
{
	// Set analog pins
	ANSEL = 0;
	ANSELH = 0;
	
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
		
	Initialize_UART();	
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