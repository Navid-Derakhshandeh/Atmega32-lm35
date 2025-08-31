#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

// LCD control pins
#define RS PC0
#define EN PC1

void lcd_command(unsigned char cmd) {
	PORTD = cmd;
	PORTC &= ~(1 << RS); // RS = 0 for command
	PORTC |= (1 << EN);
	_delay_ms(2);
	PORTC &= ~(1 << EN);
}

void lcd_data(unsigned char data) {
	PORTD = data;
	PORTC |= (1 << RS); // RS = 1 for data
	PORTC |= (1 << EN); // EN = 1 latch data
	_delay_ms(2);
	PORTC &= ~(1 << EN); // EN = 0 complete pulse
}

void lcd_init() {
	DDRD = 0xFF; // LCD data lines
	DDRC |= (1 << RS) | (1 << EN); // LCD control lines

	_delay_ms(20);
	lcd_command(0x38); // 8-bit, 2 line, 5x7 dots
	lcd_command(0x0C); // Display ON, Cursor OFF
	lcd_command(0x06); // Entry mode
	lcd_command(0x01); // Clear display
	_delay_ms(2);
}

void lcd_print(char *str) {
	while (*str) {
		lcd_data(*str++);
	}
}

void lcd_gotoxy(uint8_t x, uint8_t y) {
	uint8_t pos = (y == 0) ? (0x80 + x) : (0xC0 + x);
	lcd_command(pos);
}

// ADC setup
void adc_init() {
	ADMUX = (1 << REFS0); // AVCC as reference
	ADCSRA = (1 << ADEN) | (1 << ADPS2); // Enable ADC, prescaler 16
}

uint16_t adc_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF8) | channel;
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

int main() {
	char tempStr[8];
	lcd_init();
	adc_init();

	DDRB |= (1 << PB0); // Set PB0 as output for LED
	DDRB |= (1 << PB1); // Set PB0 as output
	while (1) {
		uint16_t adc_value = adc_read(0); // LM35 on ADC0 (PA0)
		float voltage = adc_value * 5.0 / 1024.0;
		float temp = voltage * 100.0; // LM35: 10mV per °C

		dtostrf(temp, 5, 1, tempStr); // Convert float to string

		lcd_command(0x01); // Clear screen
		lcd_gotoxy(0, 0);
		lcd_print("Temp: ");
		lcd_print(tempStr);
		lcd_print(" C");
		
			
			
		// Overheat check
		if (temp > 70.0) {
			PORTB |= (1 << PB0); // Turn LED ON
			PORTB &= ~(1 << PB1); // Turn LED OFF
			} else {
			PORTB &= ~(1 << PB0); // Turn LED OFF
			 PORTB |= (1 << PB1);  // Turn LED ON
			 
		}

		_delay_ms(120);
	}
}
