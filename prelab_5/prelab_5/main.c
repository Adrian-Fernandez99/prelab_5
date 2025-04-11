/*
Universidad del Valle de Guatemala
IE2023 : Programaci�n de Microcontroladores

prelab_5.c

Author : Adri�n Fern�ndez
Created: 4/11/2025 11:59:40 AM

Descripci�n: contador de 8 bits
 */ 

// Librerias
#include <avr/io.h>
#include <avr/interrupt.h>

// Se definen variables
const uint8_t TABLA7SEG[16] = 
{ 
	0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B, 0x77, 0x7F, 0x4E, 0x7E, 0x4F, 0x47 
};
//	0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    A,    B,    C,    D,    E,    F

uint8_t ADC_reader = 0;		// Variable del ADC
uint8_t dutyC = 191;

// Prototipo de funciones
void setup();
void ADC_conf();
void display();
void PWM_conf();
void up_DutyCycle(uint8_t dutyC);

// Main loop
int main(void)
{
	// Se llama funci�n para setear
	setup();
	
	// Se inicia el loop
	while (1)
	{
		up_DutyCycle(uint8_t dutyC);
	}
}

// NON-Interrupt subroutines

// Funci�n para setear
void setup()
{
	cli();
	
	// Se configuran puertos
	DDRD |= 0xFF;		// Puerto D es salida
	PORTD |= 0x00;
	
	DDRC |= 0xFF;		// Puerto C es salida
	PORTC |= 0x00;
	
	DDRB |= 0x0F;		// La mitad del puerto B es salida y la otra entrada
	PORTB |= ((1 << PORTB4) | (1 << PORTB5));
	
	// Se configuran las interrupciones
	PCMSK0 |= (1 << PORTB4) | (1 << PORTB5);
	PCICR |= (1 << PCIE0);
	
	CLKPR	= (1 << CLKPCE); // Habilitar cambio en el prescaler
	CLKPR	= (1 << CLKPS2); // Setea presc a 16 para 1Mhz
	
	PWM_conf();
	
	ADC_conf();
	ADCSRA	|= (1 << ADSC);


	sei();
}

void PWM_conf()
{
	TCCR0A = 0;
	TCCR0A |= (1 << COM0A1) | (1 << COM0A0); // Seteamos modo invertido
	TCCR0A |= (1 << WGM01) | (1 << WGM00);	// Modo 3 = FAST PWM y TOP
	
	TCCR0B = 0;
	TCCR0B |= (1 << CS02); // Prescaler de 256
}

up_DutyCycle(uint8_t dutyC)
{
	OCR0A = dutyC;
}

void ADC_conf()
{
	ADMUX = 0;								// Se edita el MUX
	ADMUX	|= (1 << REFS0);
	ADMUX	|= (1 << ADLAR);				// Se ajusta a la izquierda
	ADMUX	|= (1 << MUX2) | (1 << MUX1);	// Selecciona ADC6 como entrada
	
	ADCSRA	= 0;
	ADCSRA	|= (1 << ADPS1) | (1 << ADPS0);	// Se edita prescaler del ADC
	ADCSRA	|= (1 << ADIE);					// Se habilitan interrupciones
	ADCSRA	|= (1 << ADEN);					// Se habilita el ADC
	ADCSRA	|= (1 << ADSC);					// Se lee el ADC
}

// Interrupt routines
ISR(ADC_vect)
{
	uint16_t suma = 0;
	for (uint8_t i = 0; i < 8; i++) {	// Mientras i no sea mayor que 8
		ADCSRA |= (1 << ADSC);			// Leer ADC
		while (ADCSRA & (1 << ADSC));	// Mientras se pueda leer el ADC
		suma += ADCH;					// Sumar el ADC a una variable
	}
	ADC_reader = suma / 8;				// Sacar promedio de esta variable
	
	display1 = ADC_reader & 0x0F;				// Subir bits menos significativos del ADC al display1
	display2 = (ADC_reader >> 4) & 0x0F;		// Subir bits m�s significativos del ADC al display2
	
	ADCSRA	|= (1 << ADSC);				// Leer el ADC
}