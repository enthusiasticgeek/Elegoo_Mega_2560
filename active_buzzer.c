#include <avr/io.h>
#include <util/delay.h>

//Connect pin 13 on Arduino to +ve side Active buzzer and GND to -ve pin on Active buzzer.

#define C0  16
#define Cc0 17
#define D0  18
#define Dc0 19
#define E0  21
#define F0  22
#define Fc0 23
#define G0  25
#define Gc0 26
#define A0  28
#define Ac0 29
#define B0  31

#define C1  33
#define Cc1 35
#define D1  37
#define Dc1 39
#define E1  41
#define F1  44
#define Fc1 46
#define G1  49
#define Gc1 52
#define A1  55
#define Ac1 58
#define B1  62

#define C2  65
#define Cc2 69
#define D2  73
#define Dc2 78
#define E2  82
#define F2  87
#define Fc2 93
#define G2  98
#define Gc2 104
#define A2  110
#define Ac2 117
#define B2  123

#define C3  131
#define Cc3 139
#define D3  147
#define Dc3 156
#define E3  165
#define F3  175
#define Fc3 185
#define G3  196
#define Gc3 208
#define A3  220
#define Ac3 233
#define B3  247

#define C4  262
#define Cc4 277
#define D4  294
#define Dc4 311
#define E4  330
#define F4  349
#define Fc4 370
#define G4  392
#define Gc4 415
#define A4  440
#define Ac4 466
#define B4  494

#define C5  523
#define Cc5 554
#define D5  587
#define Dc5 622
#define E5  659
#define F5  698
#define Fc5 740
#define G5  783
#define Gc5 831
#define A5  880
#define Ac5 932
#define B5  988

static unsigned long division(unsigned long dividend, unsigned long divisor)
{
    return (dividend + (divisor/2)) / divisor;
}

static void set_note(unsigned long note_frequency, unsigned long timer_frequency)
{
    OCR0A = division(timer_frequency, note_frequency*2) - 1;
    TCCR0A = _BV(COM0A0) | _BV(WGM01); // toggle OC0A on Compare Match and CTC
}
 
int main(void)
{
    unsigned long timer_frequency;
    const unsigned long prescalar = 256;

    const unsigned long DO = 523;
    const unsigned long RE = 587;
    const unsigned long MI = 659;
    const unsigned long FA = 698;
    const unsigned long SO = 784;
    const unsigned long LA = 880;
    const unsigned long TI = 988;
    const unsigned long DO1 = 1047;
 
    DDRB |= _BV(DDB7);
    OCR0A = division(timer_frequency, prescalar*2) - 1;
    TCCR0A = _BV(COM0A0) | _BV(WGM01); // toggle OC0A on Compare Match and CTC
    TCCR0B = _BV(CS02); // 256 prescalar
    timer_frequency = division(F_CPU, prescalar);
 
    while(1)
    {
        set_note(DO, timer_frequency);
        _delay_ms(200);

        set_note(RE, timer_frequency);
        _delay_ms(200);

        set_note(MI, timer_frequency);
        _delay_ms(200);

        set_note(FA, timer_frequency);
        _delay_ms(200);

        set_note(SO, timer_frequency);
        _delay_ms(200);

        set_note(LA, timer_frequency);
        _delay_ms(200);

        set_note(TI, timer_frequency);
        _delay_ms(200);

        set_note(DO1, timer_frequency);
        _delay_ms(200);

    }
    return 0;
}
