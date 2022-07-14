#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <LCD.h>
#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>

unsigned short x;
unsigned short x_old;
char ignore;

int main(void)
{
	// Turn on LCD.
	init_LCD();
	LCD_cmd(0x0F);

	// Pins 2 to 0 of port A are used for the control of the LCD.
	DDRA = (1 << DDA2) | (1 << DDA1) | (1 << DDA0);

	// Pins 7, 5, and 4 of port B are used to connect the transmitter and the reciever.
	DDRB = (1 << DDB7) | (1 << DDB5) | (1 << DDB4);

	// Pins 7 to 0 of port C are used for the data of the LCD.
	DDRC = (1 << DDC7) | (1 << DDC6) | (1 << DDC5) | (1 << DDC4) | (1 << DDC3) | (1 << DDC2)| (1 << DDC1) | (1 << DDC0 );
	
	// Slave select; since we only have one slave, therefore only one bit is needed.
	PORTB = (1 << PORTB4);

	// The settings of the analog-to-digital converter:
	// We read the analog value from pin 3 of port A. Therefore MUX = 00011;
	// The results are left-adjusted. Therefore ADLAR = 1;
	// The voltage reference is at pin AVCC. Therefore RFS = 01;
	// AD converter must be enabled, therefore ADEN = 1;
	// The convertion is performed automatically, therefore ADATE = 1;
	// We use the interrupt of ADC. Therefore, ADIE = 1;
	// We us prescale of size 8. Therefore, ADPS = 011.
	ADMUX = (0 << REFS1) | (1 << REFS0) | (1 << ADLAR) | (0 << MUX4)  | (0 << MUX3)  | (0 << MUX2)  | (1 << MUX1)  | (1 << MUX0);
	ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADIE) | (0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	// The settings of analog comprator:
	// The comprator must be enabled, thus ACD = 0;
	// We use two inputs, therefor ACBG = 0;
	// The output is initially zero, thus ACO = 0;
	// We do not use interrupt of AC, therefore all other bits are zero.
	ACSR = (0 << ACD) | (0 << ACBG) | (0 << ACO) | (0 << ACI) | (0 << ACIE) | (0 << ACIC) | (0 << ACIS1) | (0 << ACIS0);

	// We use the free-runnig mode for ADC, therefore ADTS = 000;
	// We use ADC as well as AC, therefore ACME = 0;
	SFIOR = (0 << ACME) | (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0);

	// The settings for data communication between the transmitter and the receiver:
	// We use SPI, therefore SPE = 1;
    // We use master type of SPI, therefore MSTR = 1;
    // We use clock rate: 8MHz / 128 = 62.5 kHz, therefore SPI2X = 0, SPR = 11;
    // We use clock phase: cycle half, therefore CPHA = 0;
    // We use clock polarity: low, therefore CPOL = 0;
    // We use data order: MSB-first, therefore DORD = 0.
	SPCR = (1 << SPE) | (0 << DORD) | (1 << MSTR) | (0 << CPOL) | (0 << CPHA) | (1 << SPR1) | (1 << SPR0);
    SPSR = (0 << SPI2X);

	// Enabling all interrupts.
	sei();

	while (1) {
		// Checks if the result of AC.
		// If the output of AC is 1, starts a AD convertion.
		if (((ACSR >> ACO) & 1) == 1) {
			ADCSRA |= ((1 << ADSC));
		}
	}
}

void writeString(char *str, int length) 
{
	LCD_cmd(0x01);
	unsigned char i;
		for (i = 0; i < length; i++) {
			LCD_write(str[i]);
		}
}

ISR(ADC_vect) {
	x_old = x;
	x = (unsigned short)(ADCW >> 7);
	if (x_old != x) {
		// Writes data on the LCD.
		int length = snprintf(NULL, 0, "%u", x);
		char *str = malloc(length+1);
		snprintf(str, length+1, "%u", x);
		writeString(str, length);

		// Sends the data to the slave.
		PORTB &= ~(1 << PORTB4);
		SPDR = x;
		while(((SPSR >> SPIF) & 1) == 0);
		ignore = SPDR;
		_delay_ms(100);
	}
}
