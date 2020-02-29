/*
 * printf.c
 *
 * Created: 29.02.2020 19:32:53
 *  Author: Rene
 */ 

#include "printf.h"

void printf_init(uint64_t baud, uint64_t f_cpu)
{
    // Initialize I/O's
    DDRD &= ~(1<<PD0); //PORTD (RX on PD0)

    //Set USART Baud rate
    UCSRB =(1 << TXEN | 1 << RXEN | 1<< RXCIE);
    UBRRH = (MYUBRR(f_cpu, baud) >> 8);
    UBRRL = MYUBRR(f_cpu, baud);
    UCSRB = (1<<RXEN)|(1<<TXEN);
    UCSRC = (1<<URSEL)|(3<<UCSZ0);
        
    stdout = &mystdout; //Required for printf init
}

static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') uart_putchar('\r', stream);
    
    while( !(UCSRA & (1<<UDRE)) );
    UDR = c;
    
    return 0;
}

uint8_t uart_getchar(void)
{
    while( !(UCSRA & (1<<RXC)) );
    return(UDR);
}