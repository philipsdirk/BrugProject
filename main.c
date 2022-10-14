#include <avr/io.h>
#include <avr/interrupt.h> /*https://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html*//*https://redirect.cs.umbc.edu/~ameyk1/notes/AVR_Interrupts_in_C.pdf*/
#include <util/delay.h>
#include "servo.h" /*Credit: XvR 2020*/
#include "h_bridge.h" /*Credit: XvR 2020*/

void debug() {
    DDRL = (1<<PL0);
    while (1) {
        _delay_ms(500);
        PORTL = (0 << PL0);
        _delay_ms(500);
    }
}

ISR(INT0_vect) { /*Interrupt voor de noodstop op digital pin 21*/
    /*De acties die bij een noodstop moeten gebeuren*/
    init_h_bridge();
    h_bridge_set_percentage(0);
}

int Knop1() { /*Knop zodat de brugwachter de opening kan starten*/
    DDRF &= ~(1<<PF7);
    if (PINF & (1<<PF7)) {
        return 0;
    } else {
        return 1;
    }
}

int Knop2() {
    DDRF &= ~(1<<PF6);
    if (PINF & (1<<PF6)) {
        return 0;
    } else {
        return 1;
    }
}

int LimitOpen() {
    DDRF &= ~(1<<PF1);
    if (PINF & (1<<PF1)) {
        return 0;
    } else {
        return 1;
    }
}

int LimitDicht() {
    DDRF &= ~(1<<PF2);
    if (PINF & (1<<PF2)) {
        return 0;
    } else {
        return 1;
    }
}

void SlagbomenOpenen(){
    init_servo();
    for (int i = 100; i > -100; i -= 1) {
        servo1_set_percentage(i);
        servo2_set_percentage(i);
        _delay_ms(5);}
}
void SlagbomenSluiten(){
    init_servo();
    for (int i = -100; i < 100; i += 1) {
        servo1_set_percentage(i);
        servo2_set_percentage(i);
        _delay_ms(5);}
}

int counter = 10;
int InfraroodCounter() {
    /*Tel of alle boten gepasseerd zijn*/
    DDRK &= ~(1<<PK0 | 1<<PK1);
    int infra1_doorbroken = 0;
    int infra2_doorbroken = 0;

    if (PINK & (1<<PK0)) {
        if (infra1_doorbroken == 0) {
            _delay_ms(40);
            infra1_doorbroken = 1;
        }
    }
    if ((PINK & (1<<PK0)) == 0) {
        if (infra1_doorbroken != 0) {
            _delay_ms(40);
            infra1_doorbroken = 0;
            counter++;
        }
    }
    if (PINK & (1<<PK1)) {
        if (infra2_doorbroken == 0) {
            _delay_ms(40);
            infra2_doorbroken = 1;
        }
    }
    if ((PINK & (1<<PK1)) == 0) {
        if (infra2_doorbroken != 0) {
            _delay_ms(40);
            infra2_doorbroken = 0;
            counter--;
        }
    }
    if (counter >= 11){
        DDRL = (1<<PL0);
        PORTL = (1 << PL0);
        return 1;
    } else {
        DDRL = (1<<PL0);
        PORTL = (0 << PL0);
        return 0;
    }
}

int Weerstop() { /*Check of het te hardt waait*/
    int WeerSlecht = 0;
    DDRK &= ~(1<<PK3);
    if (PINK & (1<<PK3)){
        WeerSlecht = 1;
        _delay_ms(5000);
    } else {
        WeerSlecht = 0;
    }
    return WeerSlecht;
}

void LEDsWachter(int Aanmelding, int Klaar, int Beweging, int Varen, int Noodstop) {
    DDRB = (1<<PB0 | PB2);
    DDRL = (1<<PL0 | PL2 | PL4);
}

void LEDsBrug(int Rood1, int Rood2, int Groen1, int voetgangers) {
    DDRB = (1<<PB3);
    DDRL = (1<<PL1 | PL3);
    /*Defineer welke LED's je gebruikt*/
    /*Zet de LED's aan/uit afhankelijk van de input*/
}

void Openen() {
    while (Knop1());
    LEDsBrug(1, 0, 0, 1); /*LED: rood-groen en voetgangers aan*/
    _delay_ms(500);/*Wachten*/
    SlagbomenSluiten();
    while(Knop1()); /*Brugwachter om openen vragen*/
    init_h_bridge();
    while (LimitOpen()) {
        h_bridge_set_percentage(-55);/*Brug openen*/
    }
    h_bridge_set_percentage(0);
    LEDsBrug(0, 0, 0, 1); /*LED: groen en voetgangers aan*/
    Sluiten();
}

void Sluiten() {
    LEDsBrug(1, 0, 0, 1); /*LED: rood en voetgangers aan*/
    while (InfraroodCounter());
    while(Knop1()); /*Brugwachter om sluiten vragen*/
    init_h_bridge();
    while (LimitDicht()) {
        h_bridge_set_percentage(40);
    }
    _delay_ms(200);
    h_bridge_set_percentage(0);/*Brug sluiten*/
    SlagbomenOpenen();
    LEDsBrug(1, 0, 0, 0); /*LED: voetgangers uit en rood*/
}

int main(void) {
    EICRA = 0<<ISC00; /*Stelt in dat een interrupt getriggerd wordt als de waarde van pin21 verandert, zie Datasheet 15.2.1*/
    EIMSK = 1<<INT0; /*Zet de interrupt voor pin21 aan, zie Datasheet 15.2.3*/
    sei(); /*Zet interrupts aan, zie Datasheet 7.4.1*/
    SlagbomenOpenen();
    init_h_bridge();
    h_bridge_set_percentage(40);
    _delay_ms(500);
    h_bridge_set_percentage(0);
    while(1) { /*Hier komt de main loop waar meerdere functies gecalled worden*/
        if (Weerstop() == 0) { /*Als weer slecht niet verder gaan*/
            if (Knop1() == 0) {
                Openen();
            }
            if (InfraroodCounter() == 1) {
                Openen();
            }
        }
    }
}
