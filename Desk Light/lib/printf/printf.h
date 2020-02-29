/*
 * printf.h
 *
 * Created: 29.02.2020 19:33:04
 *  Author: Rene
 */ 


#ifndef PRINTF_H_
#define PRINTF_H_

#include <stdio.h>  // For Printf
#include <avr/io.h>
#include <avr/interrupt.h>

#define MYUBRR(f, b)  (f/16/b-1)

void printf_init(uint64_t baud, uint64_t f_cpu);
static int uart_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

#endif /* PRINTF_H_ */