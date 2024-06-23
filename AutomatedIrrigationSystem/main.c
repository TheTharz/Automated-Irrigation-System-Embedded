#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "i2c.h"
#include "LCD_I2C.h"

// Define pin numbers
#define moistureSensor1Pin PC0 // Analog input pin for first soil moisture sensor (A0 = ADC0)
#define moistureSensor2Pin PC1 // Analog input pin for second soil moisture sensor (A1 = ADC1)
#define relay1Pin PB0       // Digital output pin for first relay control (Pin 8 = PB0)
#define relay2Pin PB1       // Digital output pin for second relay control (Pin 9 = PB1)

// I2C Pins (SDA = PC4, SCL = PC5)
#define SDA_PIN PC4
#define SCL_PIN PC5

// Define moisture threshold values (adjust as needed)
#define dryThreshold1 300
#define dryThreshold2 300

int map(int x) {
	return (int)(((float)x / 1023.0) * 100.0);
}


// Function to read ADC value from specified pin
int readADC(uint8_t pin) {
	ADMUX = (1 << REFS0) | (pin & 0x0f); // Select ADC pin with reference voltage on AVCC
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128
	ADCSRA |= (1 << ADSC); // Start conversion
	while (ADCSRA & (1 << ADSC)); // Wait for conversion to complete
	return ADCW; // Return ADC value
}

// Function to control relay based on moisture level
void controlRelay(int moistureValue, int threshold, uint8_t relayPin) {
	if (moistureValue < threshold) {
		PORTB |= (1 << relayPin); // Turn on relay
		} else {
		PORTB &= ~(1 << relayPin); // Turn off relay
	}
}

void lcd_print(int value, int relayState) {
	// Implementation to print an integer value on the LCD
	// along with the relay state (ON or OFF)
	char buffer[20];
	sprintf(buffer, "%d", value);  // Convert integer to string
	lcd_msg(buffer);               // Display moisture value on LCD

	lcd_msg(" ");
	if (relayState == 1) {
		lcd_msg("ON ");
		} else {
		lcd_msg("OFF");
	}
}

int main(void)
{
	 i2c_init();
	 i2c_start();
	 i2c_write(0x70);
	 lcd_init();
	DDRB |= (1 << relay1Pin) | (1 << relay2Pin); // Set relay pins as output

	while (1)
	{
		// Read moisture levels from sensors
		int moisture1 = readADC(moistureSensor1Pin);
		int moisture2 = readADC(moistureSensor2Pin);

		// Map ADC values to percentage (assuming ADC range 0-1023)
		int moisturePercentage1 = map(moisture1);
		int moisturePercentage2 = map(moisture2);

		// Display moisture levels on LCD
		lcd_cmd(0x80); // Set cursor to beginning of first line
		lcd_msg("M1: ");
		//_delay_ms(1000);

		
		lcd_print(moisturePercentage1, (PORTB & (1 << relay1Pin)) ? 1 : 0);

		
		//_delay_ms(1000);
		
		


		lcd_cmd(0xC0); // Set cursor to beginning of second line
		lcd_msg("M2: ");
		//_delay_ms(1000);

		lcd_print(moisturePercentage2, (PORTB & (1 << relay2Pin)) ? 1 : 0);
		//_delay_ms(1000);


		// Control relays based on moisture thresholds
		controlRelay(moisture1, dryThreshold1, relay1Pin);
		controlRelay(moisture2, dryThreshold2, relay2Pin);

		_delay_ms(1000); // Delay for stability and to prevent rapid updates
	}
}
