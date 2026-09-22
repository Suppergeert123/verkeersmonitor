#include <Arduino.h>

const uint8_t telslang1 = PD2;
const uint8_t telslang2 = PD3;

const uint8_t led1 = PB4;
const uint8_t led2 = PB5;
const uint8_t led4 = PC4;
const uint8_t led8 = PC5;

const unsigned long debounceTime = 50;

uint8_t count = 0;

bool vorigeState1 = HIGH;
bool vorigeState2 = HIGH;

void InitializeIO();

bool button_state(uint8_t pin);
bool axle_detected(uint8_t pin);
bool vehicle_passed();

void display_counter();

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
    }
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

    // LEDs uit
    PORTB &= ~((1 << led1) | (1 << led2));
    PORTC &= ~((1 << led4) | (1 << led8));
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