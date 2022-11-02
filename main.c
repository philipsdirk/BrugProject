#include <avr/io.h>
#include <avr/interrupt.h> /*https://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html*//*https://redirect.cs.umbc.edu/~ameyk1/notes/AVR_Interrupts_in_C.pdf*/
#include <util/delay.h>
#include "servo.h" /*Credit: XvR 2020*/
#include "h_bridge.h" /*Credit: XvR 2020*/
int resetvalue;
int counter = 10;
int openen;
int sluiten;
int doorvaart;

void debug() {
    DDRL = (1<<PL0);
    while (1) {
        _delay_ms(500);
        PORTL ^= (1 << PL0);
        _delay_ms(500);
    }
}

int Knop1() { /*Knop zodat de brugwachter een actie kan goedkeuren*/
    if (PINF & (1<<PF7)) {
        return 0;
    } else {
        return 1;
    }
}

int Knop2() { /*Knop zodat de brugwachter een actie kan afwijzen*/
    if (PINF & (1<<PF6)) {
        return 0;
    } else {
        return 1;
    }
}

int DekSensor() { /*Drukschakelaar in het wegdek*/
    if (PINF & (1<<PF0)) {
        return 0;
    } else {
        return 1;
    }
}

int LimitOpen() { /*Limitswitch die checkt of de brug al helemaal open is*/
    if (PINF & (1<<PF1)) {
        return 1;
    } else {
        return 0;
    }
}

int LimitDicht() { /*Limitswitch die checkt of de brug al helemaal dicht is*/
    if (PINF & (1<<PF2)) {
        return 1;
    } else {
        return 0;
    }
}

void SlagbomenOpenen(){
    for (int i = 0; i<10; i++){
        PORTA ^= (1 << PA4);
        _delay_ms(200);}
    for (int i = 100; i > -75; i -= 1) {
        if(i==5 || i==75 || i==-35 || i==40){
            PORTA ^= (1 << PA4);
        }
        servo1_set_percentage(i);
        _delay_ms(5);}
}

void SlagbomenSluiten(){
    int SensorUsed;
    for (int i = 0; i<10; i++){
        PORTA ^= (1 << PA4);
        _delay_ms(200);
    }
    while (DekSensor()) {
        SensorUsed = 1;
        PORTA ^= (1 << PA4);
        _delay_ms(200);
    }
    if (SensorUsed == 1) {
        SensorUsed = 0;
        for (int i = 0; i<11; i++) {
            PORTA ^= (1 << PA4);
            _delay_ms(200);
        }
    }
    for (int i = -75; i < 100; i += 1) {
        if(i==5 || i==75 || i==-35 || i==40){
            PORTA ^= (1 << PA4);
        }
        servo1_set_percentage(i);
        _delay_ms(5);
    }
    PORTA &= ~(1 << PA4);
}

void LEDs(int Rood1, int Rood2, int Groen1, int voetgangers, int Aanmelding, int Klaar, int Beweging, int Varen, int Noodstop){
    PORTB &= ~(1<<PB0 | 1<<PB1 | 1<<PB2 | 1<<PB3);
    PORTL &= ~(1<<PL0 | 1<<PL1 | 1<<PL2 | 1<<PL3 | 1<<PL4);
    if (Rood1 == 1){
        PORTB |= (1 << PB3);
    }
    if (Rood2 == 1){
        PORTL |= (1 << PL3);
    }
    if (Groen1 == 1){
        PORTL |= (1 << PL1);
    }
    if (voetgangers == 1){
        PORTB |= (1 << PB1);
    }
    if (Aanmelding == 1){
        PORTB |= (1 << PB0);
    }
    if (Klaar == 1){
        PORTB |= (1 << PB2);
    }
    if (Beweging == 1){
        PORTL |= (1 << PL0);
    }
    if (Varen == 1){
        PORTL |= (1 << PL2);
    }
    if (Noodstop == 1){
        PORTL |= (1 << PL4);
    }
}

int InfraroodCounter() {
    /*Tel of alle boten gepasseerd zijn*/
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
        if (doorvaart == 1) {
            LEDs(0, 0, 1, 1, 0, 0, 0, 1, 0);
        }
        return 1;
    } else {
        LEDs(0, 0, 1, 1, 0, 1, 0, 0, 0);
        return 0;
    }
}

int Weerstop() { /*Check of het te hardt waait*/
    int WeerSlecht = 0;
    DDRK &= ~(1<<PK3);
    if (PINK & (1<<PK3)){
        WeerSlecht = 1;
        LEDs(1, 1, 0, 0, 0, 0, 0, 0, 1);
        _delay_ms(5000);
    } else {
        WeerSlecht = 0;
        LEDs(1, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    return WeerSlecht;
}

void Sluiten() {
    doorvaart = 1;
    while (InfraroodCounter() || Knop1()) {
        if (Knop2() == 0) {
            counter = 10;
            break;
        }
    }
    doorvaart = 0;
    LEDs(1, 0, 0, 1, 0, 0, 1, 0, 0);
    h_bridge_set_percentage(40);
    sluiten = 1;
    while (LimitDicht());
    sluiten = 0;
    h_bridge_set_percentage(50);
    _delay_ms(100);
    h_bridge_set_percentage(0);/*Brug sluiten*/
    LEDs(1, 0, 0, 1, 0, 0, 0, 0, 0);
    _delay_ms(500);
    SlagbomenOpenen();
    LEDs(1, 0, 0, 0, 0, 0, 0, 0, 0); /*LED: voetgangers uit en rood*/
}

void Openen() {
    _delay_ms(500);
    while(1) {
        if (Knop1() == 0) {
            break;
        }
        if (Knop2() == 0) {
            resetvalue = 1;
            goto reset;
        }
    }
    LEDs(1, 0, 1, 1, 0, 0, 0, 0, 0); /*LED: rood-groen en voetgangers aan*/
    SlagbomenSluiten();
    LEDs(1, 0, 1, 1, 0, 1, 0, 0, 0);
    while(1) {
        if (Knop1() == 0){
            break;
        }
        if (Knop2() == 0) {
            resetvalue = 1;
            goto reset;
        }
    } /*Brugwachter om openen vragen*/
    LEDs(1, 0, 1, 1, 0, 0, 1, 0, 0);
    h_bridge_set_percentage(-40);/*Brug openen*/
    openen = 1;
    while (LimitOpen());
    openen = 0;
    h_bridge_set_percentage(0);
    LEDs(0, 0, 1, 1, 0, 0, 0, 1, 0); /*LED: groen en voetgangers aan*/
    Sluiten();
    reset:
        if (resetvalue == 1) {
            resetvalue = 0;
            SlagbomenOpenen();
            LEDs(1, 0, 0, 0, 0, 0, 0, 0, 0);
        }
}

ISR(INT0_vect) { /*Interrupt voor de noodstop op digital pin 21*/
    /*De acties die bij een noodstop moeten gebeuren*/
    init_h_bridge();
    h_bridge_set_percentage(0);
    LEDs(1, 1, 0, 1, 0, 0, 0, 0, 1);
    while(1) {
        LEDs(1, 1, 0, 1, 0, 0, 0, 0, 1);
        _delay_ms(500);
        LEDs(1, 1, 0, 1, 0, 0, 0, 0, 0);
        _delay_ms(500);
        if (Knop1() == 0) {
            if (sluiten == 1) {
                init_h_bridge();
                h_bridge_set_percentage(40);
                while (LimitDicht());
                h_bridge_set_percentage(0);
                sluiten = 0;
            }
            if (openen == 1) {
                init_h_bridge();
                h_bridge_set_percentage(-40);
                while (LimitOpen());
                h_bridge_set_percentage(0);
                openen = 0;
            }
            break;
        }
    }
}

void init() {
    DDRB = (1<< PB0 | 1<<PB1 | 1<<PB2 | 1<<PB3);
    DDRL = (1<<PL0 | 1<<PL1 | 1<<PL2 | 1<<PL3 | 1<<PL4);
    DDRA = (1<<PA4);
    PORTA &= ~(1<<PA4);
    DDRK &= ~(1<<PK0 | 1<<PK1);
    DDRF &= ~(1<<PF0 | 1<<PF1 | 1<<PF2);
    DDRF &= ~(1<<PF6 | 1<<PF7);
    EICRA = (1<<ISC00 | 1<<ISC01); /*Stelt in dat een interrupt getriggerd wordt als de waarde van pin21 verandert, zie Datasheet 15.2.1*/
    EIMSK = (1<<INT0); /*Zet de interrupt voor pin21 aan, zie Datasheet 15.2.3*/
    sei(); /*Zet interrupts aan, zie Datasheet 7.4.1*/
    init_servo();
    init_h_bridge();
    servo1_set_percentage(-75);
    h_bridge_set_percentage(40);
    _delay_ms(500);
    h_bridge_set_percentage(0);
    LEDs(1, 0, 0, 0, 0, 0, 0, 0, 0);
}

int main(void) {
    init();
    while(1) { /*Hier komt de main loop waar meerdere functies gecalled worden*/
        if (Weerstop() == 0) { /*Als weer slecht niet verder gaan*/
            if (Knop1() == 0) {
                LEDs(1, 0, 0, 0, 1, 0, 0, 0, 0);
                Openen();
            }
            if (InfraroodCounter() == 1) {
                LEDs(1, 0, 0, 0, 1, 0, 0, 0, 0);
                Openen();
            }
        }
    }
}
