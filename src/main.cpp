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

const uint8_t ledFoutieveMeting = PB3;

const unsigned long debounceTime = 50;
const unsigned long maxTime = 1000;
const unsigned long displayRefreshTime = 2000;
const float minSpeed = 0.0556; // minimum snelheid in m/s

uint8_t count = 0;

bool metingGestart = false;
bool voorsteBandenGeweest = false;
unsigned long tijdStartMeting = 0;

// start op high door de pull up
bool vorigeState1 = HIGH;
bool vorigeState2 = HIGH;

unsigned long verstrekenTijd = 0;
float snelheid = 0.0;
float afstand = 0.6;

uint8_t displayGetal1 = 0;
uint8_t displayGetal2 = 0;

bool displayActief = false;

uint8_t huidigeDigit = 1;
unsigned long vorigeDisplayRefresh = 0;

void InitializeIO();

bool button_state(uint8_t pin);
bool axle_detected(uint8_t pin);
bool vehicle_passed();

void display_counter();

void determine_and_show_speed();
void show_speed(float snelheid);
void show_digit(uint8_t digit, uint8_t getal);
void refresh_display();

void clear_display();
bool valid_speed(float snelheid);

void check_serial();

void setup()
{
    InitializeIO();
    Serial.begin(9600);
}

void loop()
{
    check_serial();

    if (vehicle_passed())
    {
        if(voorsteBandenGeweest){
            count++;
        }

        if (count > 15)
        {
            count = 0;
        }

        display_counter();
    }

    determine_and_show_speed();

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

    // Display standaard uit
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
        // rode led aan
        PORTB |= (1 << ledFoutieveMeting);

        if (!metingGestart)
        {
            voorsteBandenGeweest = !voorsteBandenGeweest;

            tijdStartMeting = millis();

            metingGestart = true;

            clear_display();
        }
    }

    if (axle_detected(telslang2))
    {
        // rode led aan
        PORTB |= (1 << ledFoutieveMeting);

        if (metingGestart)
        {
            if (millis() - tijdStartMeting < maxTime)
            {
                verstrekenTijd = millis() - tijdStartMeting;

                metingGestart = false;

                PORTB &= ~(1 << ledFoutieveMeting);

                return true;
            }

            metingGestart = false;
        }
    }

    // controle of de snelheid niet onder de 0.2 km/u valt
    if (metingGestart && afstand / ((millis() - tijdStartMeting) / 1000) < minSpeed)
    {
        metingGestart = false;
    }

    if (millis() - tijdStartMeting > maxTime)
    {
        metingGestart = false;
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

void determine_and_show_speed()
{
    if (verstrekenTijd == 0)
    {
        return;
    }

    float tijdInSeconden = verstrekenTijd / 1000.0;

    snelheid = afstand / tijdInSeconden;

    verstrekenTijd = 0;

    if (valid_speed(snelheid))
    {
        show_speed(snelheid);
    }
    else
    {
        clear_display();
    }
}

bool valid_speed(float snelheid)
{
    float snelheidKmh = snelheid * 3.6;

    if (snelheidKmh >= 0.2)
    {
        return true;
    }

    return false;
}

void show_speed(float snelheid)
{
    // Snelheid afronden op één decimaal
    if (snelheid > 2.78)
    {
        snelheid = 2.78;
    }

    uint8_t snelheidTiende = (uint8_t)(snelheid * 10.0 + 0.5);

    displayGetal1 = snelheidTiende / 10;
    displayGetal2 = snelheidTiende % 10;

    displayActief = true;
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
        show_digit(1, displayGetal1);
        huidigeDigit = 2;
    }
    else
    {
        show_digit(2, displayGetal2);
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

    // Juiste segmenten voor het getal aanzetten
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

    // Decimal point alleen bij digit 1
    if (digit == 1)
    {
        PORTC |= (1 << segmentDP);
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

void check_serial()
{
    static float nieuweAfstand = 0;
    static float decimaal = 0.1;
    static bool achterKomma = false;

    while (Serial.available() > 0)
    {
        char teken = Serial.read();

        if (teken >= '0' && teken <= '9')
        {
            if (!achterKomma)
            {
                nieuweAfstand = nieuweAfstand * 10 + (teken - '0');
            }
            else
            {
                nieuweAfstand += (teken - '0') * decimaal;
                decimaal *= 0.1;
            }
        }
        else if (teken == '.')
        {
            achterKomma = true;
        }
        else if (teken == '\n')
        {
            if (nieuweAfstand > 0)
            {
                afstand = nieuweAfstand;
            }

            nieuweAfstand = 0;
            decimaal = 0.1;
            achterKomma = false;
        }
    }
}

