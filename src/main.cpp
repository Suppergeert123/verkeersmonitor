#include <Arduino.h>

const uint8_t telslang1 = PD2;
const uint8_t telslang2 = PD3;

const uint8_t led1 = PB4;
const uint8_t led2 = PB5;
const uint8_t led4 = PC4;
const uint8_t led8 = PC5;

// 7-segment
const uint8_t segmentA = PD4;
const uint8_t segmentB = PD5;
const uint8_t segmentC = PD6;
const uint8_t segmentD = PD7;

const uint8_t segmentE = PC0;
const uint8_t segmentF = PC1;
const uint8_t segmentG = PC2;
const uint8_t segmentDP = PC3;

// digits
const uint8_t digit1 = PB0;
const uint8_t digit2 = PB1;
const uint8_t digit3 = PB2;

const unsigned long debounceTime = 50;
const unsigned long displayRefreshTime = 2000;

uint8_t count = 0;

bool vorigeState1 = HIGH;
bool vorigeState2 = HIGH;

bool displayActief = false;

uint8_t huidigeDigit = 1;
unsigned long vorigeDisplayRefresh = 0;

void InitializeIO();

bool button_state(uint8_t pin);
bool axle_detected(uint8_t pin);
bool vehicle_passed();

void display_counter();

void show_digit(uint8_t digit, uint8_t getal);
void refresh_display();
void clear_display();

void setup()
{
    InitializeIO();
}

void loop()
{
    if (vehicle_passed())
    {
        count++;

        if (count > 15)
        {
            count = 0;
        }

        display_counter();

        displayActief = true;
    }

    refresh_display();
}

// ----------------------------
// function implementaties
// ----------------------------

void InitializeIO()
{
    // Knoppen als input
    DDRD &= ~((1 << telslang1) | (1 << telslang2));

    // Interne pull-up weerstanden aan
    PORTD |= (1 << telslang1) | (1 << telslang2);

    // LEDs als output
    DDRB |= (1 << led1) | (1 << led2);
    DDRC |= (1 << led4) | (1 << led8);

    // 7-segment segmenten als output
    DDRD |= (1 << segmentA) |
            (1 << segmentB) |
            (1 << segmentC) |
            (1 << segmentD);

    DDRC |= (1 << segmentE) |
            (1 << segmentF) |
            (1 << segmentG) |
            (1 << segmentDP);

    // Digits als output
    DDRB |= (1 << digit1) |
            (1 << digit2) |
            (1 << digit3);

    clear_display();
}

bool button_state(uint8_t pin)
{
    bool state;

    if (pin == telslang1)
    {
        state = (PIND & (1 << telslang1));

        if (state != vorigeState1)
        {
            delay(debounceTime);

            state = (PIND & (1 << telslang1));

            vorigeState1 = state;

            if (state == LOW)
            {
                return true;
            }
        }
    }
    else
    {
        state = (PIND & (1 << telslang2));

        if (state != vorigeState2)
        {
            delay(debounceTime);

            state = (PIND & (1 << telslang2));

            vorigeState2 = state;

            if (state == LOW)
            {
                return true;
            }
        }
    }

    return false;
}

bool axle_detected(uint8_t pin)
{
    if (button_state(pin))
    {
        return true;
    }

    return false;
}

bool vehicle_passed()
{
    if (axle_detected(telslang1))
    {
        if (axle_detected(telslang2))
        {
            return true;
        }
    }

    return false;
}

void display_counter()
{
    if (count & 1)
        PORTB |= (1 << led1);
    else
        PORTB &= ~(1 << led1);

    if (count & 2)
        PORTB |= (1 << led2);
    else
        PORTB &= ~(1 << led2);

    if (count & 4)
        PORTC |= (1 << led4);
    else
        PORTC &= ~(1 << led4);

    if (count & 8)
        PORTC |= (1 << led8);
    else
        PORTC &= ~(1 << led8);
}

void refresh_display()
{
    if (!displayActief)
    {
        return;
    }

    if (micros() - vorigeDisplayRefresh < displayRefreshTime)
    {
        return;
    }

    vorigeDisplayRefresh = micros();

    if (huidigeDigit == 1)
    {
        show_digit(1, count);
        huidigeDigit = 2;
    }
    else
    {
        show_digit(2, 0);
        huidigeDigit = 1;
    }
}

void show_digit(uint8_t digit, uint8_t getal)
{
    // Alle digits uit
    PORTB |= (1 << digit1) |
             (1 << digit2) |
             (1 << digit3);

    // Alle segmenten uit
    PORTD &= ~((1 << segmentA) |
               (1 << segmentB) |
               (1 << segmentC) |
               (1 << segmentD));

    PORTC &= ~((1 << segmentE) |
               (1 << segmentF) |
               (1 << segmentG) |
               (1 << segmentDP));

    switch (getal)
    {
        case 0:
            PORTD |= (1 << segmentA) |
                     (1 << segmentB) |
                     (1 << segmentC) |
                     (1 << segmentD);

            PORTC |= (1 << segmentE) |
                     (1 << segmentF);
            break;

        case 1:
            PORTD |= (1 << segmentB) |
                     (1 << segmentC);
            break;

        case 2:
            PORTD |= (1 << segmentA) |
                     (1 << segmentB) |
                     (1 << segmentD);

            PORTC |= (1 << segmentE) |
                     (1 << segmentG);
            break;

        case 3:
            PORTD |= (1 << segmentA) |
                     (1 << segmentB) |
                     (1 << segmentC) |
                     (1 << segmentD);

            PORTC |= (1 << segmentG);
            break;

        case 4:
            PORTD |= (1 << segmentB) |
                     (1 << segmentC);

            PORTC |= (1 << segmentF) |
                     (1 << segmentG);
            break;

        case 5:
            PORTD |= (1 << segmentA) |
                     (1 << segmentC) |
                     (1 << segmentD);

            PORTC |= (1 << segmentF) |
                     (1 << segmentG);
            break;

        case 6:
            PORTD |= (1 << segmentA) |
                     (1 << segmentC) |
                     (1 << segmentD);

            PORTC |= (1 << segmentE) |
                     (1 << segmentF) |
                     (1 << segmentG);
            break;

        case 7:
            PORTD |= (1 << segmentA) |
                     (1 << segmentB) |
                     (1 << segmentC);
            break;

        case 8:
            PORTD |= (1 << segmentA) |
                     (1 << segmentB) |
                     (1 << segmentC) |
                     (1 << segmentD);

            PORTC |= (1 << segmentE) |
                     (1 << segmentF) |
                     (1 << segmentG);
            break;

        case 9:
            PORTD |= (1 << segmentA) |
                     (1 << segmentB) |
                     (1 << segmentC) |
                     (1 << segmentD);

            PORTC |= (1 << segmentF) |
                     (1 << segmentG);
            break;
    }

    // Juiste digit aanzetten
    if (digit == 1)
    {
        PORTB &= ~(1 << digit1);
    }
    else if (digit == 2)
    {
        PORTB &= ~(1 << digit2);
    }
}

void clear_display()
{
    displayActief = false;

    // Alle digits uit
    PORTB |= (1 << digit1) |
             (1 << digit2) |
             (1 << digit3);

    // Alle segmenten uit
    PORTD &= ~((1 << segmentA) |
               (1 << segmentB) |
               (1 << segmentC) |
               (1 << segmentD));

    PORTC &= ~((1 << segmentE) |
               (1 << segmentF) |
               (1 << segmentG) |
               (1 << segmentDP));
}