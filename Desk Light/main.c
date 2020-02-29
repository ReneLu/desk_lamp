/*
 * Desk Light.c
 *
 * Created: 08.02.2020 18:16:48
 * Author : Rene
 */ 

#define F_CPU   8000000UL
#define BAUD    9600

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "lib/ws2812/light_ws2812.h"
#ifdef DEBUG
#include "lib/printf/printf.h"
#endif

#define STRIP_LEN       45

#define TOUCH_DDR       DDRD
#define TOUCH_PIN_NR    PD2
#define TOUCH_PIN       PIND

#define BRIGHTNESS      1.0
#define HOLD_TIME_MS    2000

#define LED_COLOR_R     255
#define LED_COLOR_G     100
#define LED_COLOR_B     0

#ifdef DEBUG
#define PRINTF(str, ...)    printf((str), ##__VA_ARGS__)
#endif
#ifdef NDEBUG
#define PRINTF(str, ...)
#endif

enum _lampState
{
    LIGHT_OFF,
    LIGHT_TUNR_OFF,
    LIGHT_TUNR_ON,
    LIGHT_ON,
    LIGHT_BRIGHTER,
    NEXT_LIGHT,
    LAMP_MAX_STATE
} lampState;

enum _button_state
{
    NOT_PRESSED,
    SINGLE_PRESSED,
    LONG_PRESSED,
    HOLD,
    BUTTON_MAX_STATE
} button_state;

struct cRGB led[STRIP_LEN];
volatile uint64_t g_millisecs = 0;
uint8_t g_last_button_state = NOT_PRESSED;

volatile bool light = false;
float brightness = BRIGHTNESS;
uint8_t lamp_state = LIGHT_OFF;

void ioinit(void);
void timer_init(void);
void int0_init(void);
void light_on();
void light_off();
void set_light(float brightness);



int main(void)
{
    // Disable Interrupts during Initialization
    cli();
   
   for(uint8_t i = 0; i < STRIP_LEN; i++) {
       led[i].r=0;led[i].g=0;led[i].b=0;
   } 
    
    ioinit();
    timer_init();
    int0_init();
#ifdef DEBUG
    printf_init(BAUD, F_CPU);
#endif
    
    for(uint8_t i = 0; i < STRIP_LEN; i++) {
        led[i].r=LED_COLOR_R;led[i].g=LED_COLOR_G;led[i].b=LED_COLOR_B;
    }
    
    set_light(0);
    
    // Enable Interrupts after Initialization
    sei();

    PRINTF("Hello World!\n");

    while(1)
    {     
        switch (lamp_state)
        {
            case LIGHT_OFF:
                set_light(0);
                break;
            case LIGHT_TUNR_OFF:
                light_off();
                lamp_state = LIGHT_OFF;
                break;
            case LIGHT_TUNR_ON:
                light_on();
                lamp_state = LIGHT_ON;
                break;
            case LIGHT_ON:
                set_light(brightness);
                break;
            case LIGHT_BRIGHTER:
                brightness += 0.05;
                if (brightness > 1)
                    brightness = 0;
                set_light(brightness);
                PRINTF("New Brightness %i\n", (100*brightness));
                _delay_ms(100);
                break;
            default:
                lamp_state = LIGHT_OFF;
            break;
        }
    }
}


void ioinit(void)
{
    TOUCH_DDR &= ~(1 << TOUCH_PIN_NR);
    DDRB|=_BV(ws2812_pin);
}

void timer_init(void)
{
    // Timer 1
    TCCR1B |= (1<<CS11) | (1<<CS10) | (1<<WGM12); // Prescaler 64 | CTC Mode

    // Compare A Interrupt
    TIMSK |= (1<<OCIE1A);

    OCR1A = 125 - 1;  // 1 ms Timer    
}

void int0_init(void)
{
    MCUCR |= (1<<ISC00);
    GICR |= (1<<INT0);
}

void light_on()
{
    for (float i = 0; i <= brightness; i+=0.01)
    {
        set_light(i);
        _delay_ms(2);
    }
    
    set_light(brightness);
}

void light_off()
{
    for (float i = brightness; i > 0; i-=0.01)
    {
        set_light(i);
        _delay_ms(2);
    }
    
    set_light(0);
}

void set_light(float brightness)
{
    struct cRGB led_cor[STRIP_LEN];
    
    if (brightness > 1)
        brightness = 1;
    if (brightness < 0)
        brightness = 0;
    
    for (uint8_t i = 0; i < STRIP_LEN; i++)
    {
        led_cor[i].r = led[i].r * brightness;
        led_cor[i].g = led[i].g * brightness;
        led_cor[i].b = led[i].b * brightness;
    }
    
    ws2812_setleds(led_cor, STRIP_LEN);
}

uint64_t start_pressed = 0;
ISR (TIMER1_COMPA_vect)
{
    g_millisecs++;    
    
    // Enter Brighter state
    if ((g_millisecs - start_pressed) >= HOLD_TIME_MS && start_pressed > 0 && lamp_state == LIGHT_ON) {
        lamp_state = LIGHT_BRIGHTER;
    }
}

ISR(INT0_vect)
{
    if (TOUCH_PIN & (1 << TOUCH_PIN_NR)) { // Rising Edge Interrupt
        PRINTF("Rising at %d\n", g_millisecs);
        start_pressed = g_millisecs;
    } else {        // Falling Edge Interrupt
        PRINTF("Falling at %d\n", g_millisecs);
        PRINTF("Started pressing at %d\n", start_pressed);
        PRINTF("diff %d\n", (g_millisecs - start_pressed));
        if (g_millisecs - start_pressed < HOLD_TIME_MS) {
            if (lamp_state == LIGHT_OFF)
                lamp_state = LIGHT_TUNR_ON;
            else if (lamp_state == LIGHT_ON)
                lamp_state = LIGHT_TUNR_OFF;
        } else {
            lamp_state = LIGHT_ON; // Exit Brighter state
        }
        
        start_pressed = 0; 
    }
}