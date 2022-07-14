#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define MAX_DUTY_HEATER 100
#define MIN_DUTY_HEATER 0

int dutyCycleCooler = 50;
int dutyCycleHeater = MIN_DUTY_HEATER;

int main() 
{
	// Pin 0 of Port A is used for the warning LED.
	DDRA = (1 << DDA0);

	// Pin 7, 5, 4 of Port B is used to connect the two chips.
	// Pin 3 of Port B is used for cooler motor.
	DDRB = (0 << DDB7) | (0 << DDB5) | (0 << DDB4) | (1 << DDB3);

	// Pin 7 of Port D is used for heater motor.
	DDRD = (1 << DDD7);

	// The settings for data communication between the transmitter and the receiver:
	// We use SPI, therefore SPE = 1;
    // We use slave type of SPI, therefore MSTR = 0;
    // We use clock rate: 8MHz / 128 = 62.5 kHz, therefore SPI2X = 0, SPR = 11;
    // We use clock phase: cycle half, therefore CPHA = 0;
    // We use clock polarity: low, therefore CPOL = 0;
    // We use data order: MSB-first, therefore DORD = 0.
	SPCR = (1 << SPE) | (0 << DORD) | (0 << MSTR) | (0 << CPOL) | (0 << CPHA) | (1 << SPR1) | (1 << SPR0);
	SPSR = (0 << SPI2X);

	// The settings for heater and cooler motors (Timer/counter 2 and 0):
	// We use Fast PWM, therefore WGM0 = 11 and WGM2 = 11;
    // We clear OCx on compare match, therefore COM0 = 10 and COM2 = 10;
    // We use 1/8 clock prescaling, therefore CS0 = 101 and CS2 = 101;
	TCCR0 = (1 << WGM01) | (1 << WGM00) | (1 << COM01) | (0 << COM00) | (1 << CS02) | (0 << CS01) | (1 << CS00);
	OCR0 = (dutyCycleCooler/100)*255;
	TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (0 << COM20) | (1 << CS22) | (0 << CS21) | (1 << CS20);
	OCR2 = (dutyCycleHeater/100)*255;
	TIMSK |= (1 << TOIE2) | (1 << TOIE0);

	// Enabling interrups.
	sei();

	while(1) {
		SPDR = '0';
		while (((SPSR >> SPIF) & 1) == 0);
		_delay_ms(5);

		if (SPDR < 20) dutyCycleHeater = MAX_DUTY_HEATER;
		else dutyCycleHeater = MIN_DUTY_HEATER;
		_delay_ms(5);

		if (SPDR > 55) PORTA = 0x01;
		else PORTA = 0x00;
		_delay_ms(5);

		if (SPDR < 25) {dutyCycleCooler = 0; _delay_ms(5);}
		else if (SPDR >= 25 && SPDR < 30) { dutyCycleCooler = 50; _delay_ms(5); }
		else if (SPDR >= 30 && SPDR < 35) { dutyCycleCooler = 60; _delay_ms(5); }
		else if (SPDR >= 35 && SPDR < 40) { dutyCycleCooler = 70; _delay_ms(5); }
		else if (SPDR >= 40 && SPDR < 45) { dutyCycleCooler = 80; _delay_ms(5); }
		else if (SPDR >= 45 && SPDR < 50) { dutyCycleCooler = 90; _delay_ms(5); }
		else if (SPDR >= 50 && SPDR <= 55) { dutyCycleCooler = 100; _delay_ms(5); }
		else { dutyCycleCooler = 0; _delay_ms(5); }

	}
}

ISR (TIMER0_OVF_vect){
	OCR0 = ((float) dutyCycleCooler / 100) * 255;
}

ISR (TIMER2_OVF_vect){
	OCR2 = ((float) dutyCycleHeater / 100) * 255;
}